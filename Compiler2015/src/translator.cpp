#include "translator.h"

Translator& Translator::init( syntaxParser* _newParser ) {
    if ( _newParser != ptrParser && NULL != ptrParser) {
        delete ptrParser;
        ptrParser = _newParser;
    }
    mpSymbol.clear();
    vecCodes.clear();
    numLine = 0;
    numTmp = 0;
    return *this;
}

void Translator::translateRecur(syntaxNode* root, vector<TransFuncType>& func) {
    if ( NULL == root ) return;
    deque<syntaxNode*>& child = root->child;

    if ( NULL == root->ptrPdt ) return;
    int pId = root->ptrPdt->pId;
    TransFuncType& f = func[pId];

    f(*root, -1);
    for(int c=0;c<child.size();c++) {
        translateRecur(child[c], func);
        //printf("\n\nCall func[t%d p%d, %d]\n", root->tId, pId, c);
        f(*root, c);
        //printf("Call func[t%d p%d, %d] End\n\n\n", root->tId, pId, c);
    }
}

int Translator::getLineNum(bool _plus, bool start) {
    if ( true == start ) numLine = 1;
    if ( _plus ) return numLine++;
    else return numLine;
}

int Translator::getNewTmp(bool start) {
    if ( true == start ) numTmp = 1;
    return numTmp++;
}

///虚函数扩展，可自行定义，然后根据自行定义执行完整的翻译
Translator& Translator::translate() {
    if ( NULL == ptrParser || false == ptrParser->getIsTree() ) return *this;
    vecTransFunc = getTransFunc();
    auto vecPdt = ptrParser->getProduction();
    if ( vecPdt.size() != vecTransFunc.size() ) {
        printf("Number of Translate Functions (%d) is not equal to %d\n", vecTransFunc.size(), vecPdt.size());
        throw "Fewer Functions";
    }
    printf("Do Translate.\n");
    try{
        translateRecur(ptrParser->getSyntaxTree(), vecTransFunc);
    }
    catch(...){
        vecError.push_back( "Translate Error" );
        printf("Translate Error\n");
    }
    printf("\nCode:\n");
    for(auto code : vecCodes) {
        printf("Line %3d: ", code.lineNum);
        cout<<code.data;

    }
    printf("Err:%d\n", vecError.size());
    for(auto err : vecError) {
        cout<<err<<endl;
    }
    return *this;
}

void Translator::Backpatch( Json::Value& p, Json::Value t ) {

    assert( p.isArray() || p.isNull() );
    Json::Value right = t;
    while( right.isArray() ) right = right[0u];
    for(auto ele : p) {
        assert( vecCodes.size() > ele.asInt() );
        vecCodes[ ele.asInt() ].data[3u] = right.asInt();
    }
}
Json::Value Translator::Merge( Json::Value& p1, Json::Value p2 ) {

    Json::Value ret;
    assert( p1.isArray() || p1.isNull() );
    for(auto ele : p1)
        ret.append( ele );
    if ( p2.isArray() || p2.isNull() ) {
        for(auto ele : p2)
            ret.append( ele );
    }
    else {
        ret.append( p2 );
    }

    if( p1.size() && p2.size() ) {
        assert( vecCodes.size() > p2[0u].asInt() );
        vecCodes[ p2[0u].asInt() ][3u] = p1[ p1.size()-1 ].asInt();
    }

    return ret;
}

///定义成虚 =0函数，必须派生。
vector<Translator::TransFuncType> ArithmeticTranslator::getTransFunc() {
    ///根据数组中每个函数的定义
    /*
_S 1 S
S 1 E
E 1 T
E 3 E + T
T 1 P
T 3 T * P
P 1 NUM
P 3 ( E )
     */
    TransFuncType transFuncArr[] = {
        /* 0 _S->S */
        [&](syntaxNode& root, int pos){ /*do nothing*/ return true; },
        /* 1 S-> E */
        [&](syntaxNode& root, int pos){
            switch(pos) {
                case -1: {
                    root["place"] =  getNewTmp();
                } break;
                case 0: {
                    root["value"] = root[0]["value"];
                    printf("L-%d\t", getLineNum());
                    printf("t%d = t%d\n", root["place"].asInt(), root[0]["place"].asInt()  );
                    printf("Need t%d = %d\n", root["place"].asInt() , root["value"].asInt()  );
                } break;
            }
            return true;
        },
        /* 2 E-> T */
        [&](syntaxNode& root, int pos){
            switch(pos) {
                case -1:  break;
                case 0: {
                    root["value"] = root[0]["value"];
                    root["place"] = root[0]["place"];
                } break;
            }
            return true;
        },
        /* 3 E-> E + T */
        [&](syntaxNode& root, int pos){
            switch(pos) {
                case -1: {
                    root["place"] =  getNewTmp();
                } break;
                case 0: case 1: break;
                case 2: {
                    root["value"] = root[0]["value"].asInt() + root[2]["value"].asInt();
                    printf("L-%d\t",getLineNum());
                    printf("t%d = t%d + t%d\n", root["place"].asInt(), root[0]["place"].asInt(), root[2]["place"].asInt());
                } break;
            }
            return true;
        },
        /* 4 T-> P */
        [&](syntaxNode& root, int pos){
            switch(pos) {
                case -1: break;
                case 0: {
                    root["value"] = root[0]["value"];
                    root["place"] = root[0]["value"];
                } break;
            }
            return true;
        },
        /* 5 T-> T * P */
        [&](syntaxNode& root, int pos){
            switch(pos) {
                case -1: {
                    root["place"] =  getNewTmp();
                } break;
                case 0: case 1: break;
                case 2: {
                    root["value"] = root[0]["value"].asInt() * root[2]["value"].asInt();
                    printf("L-%d\t",getLineNum());
                    printf("t%d = t%d * t%d\n", root["place"].asInt(), root[0]["place"].asInt(), root[2]["place"].asInt());
                } break;
            }
            return true;
        },
        /* 6 P-> NUM */
        [&](syntaxNode& root, int pos){
            switch(pos) {
                case -1: {
                    root["place"] = getNewTmp();
                } break;
                case 0: {
                    root["value"] = clUtils::atoi( root[0].getLex() ) ;
                    printf("L-%d\t", getLineNum());
                    printf("t%d = [%d]\n", root["place"].asInt(), root["value"].asInt() );
                } break;
            }
            return true;
        },
        /* 7 P-> ( E ) */
        [&](syntaxNode& root, int pos){
            switch(pos) {
                case -1: case 0: case 1: break;
                case 2: {
                    root["value"] = root[1]["value"];
                    root["place"] = root[1]["place"];
                } break;
            }
            return true;
        },
    };

    return vector<TransFuncType>( transFuncArr, transFuncArr+sizeof(transFuncArr)/sizeof(TransFuncType) );
}


vector<Translator::TransFuncType> CLikeTranslator::getTransFunc() {
/*
 * 文法定义
0    _P 1 P
1    P 4 { D CS }
2    D 4 D int ID ;
3    D 3 int ID ;
4    S 9 if ( BEp ) then S else S endif
5    S 7 if ( BEp ) then S endif
6    S 6 while ( BEp ) do S
7    S 4 ID = CEp ;
8    S 3 { CS }
9    CS 2 S CS
10    CS 1 S
11    BEp 1 BET
12    BEp 3 BEp or BET
13    BET 1 BEF
14    BET 3 BET and BEF
15    BEF 3 CEp relop CEp
16    BEF 1 CEp
17    CEp 1 CET
18    CEp 3 CEp + CET
19    CEp 3 CEp - CET
20    CET 1 CEF
21    CET 3 CET * CEF
22    CET 3 CET / CEF
23    CEF 1 ID
24    CEF 1 NUM
25    CEF 3 ( CEp )
*/
    ///先制作if + bool 的 回填拉链 测试
    TransFuncType transFuncArr[] = {
        /* 0. _P 1 P */
        [&](syntaxNode& root, int pos){ return true;/* do nothing */ },
        /* 1. P 4 { D CS } */
        [&](syntaxNode& root, int pos){
            switch(pos) {
                case 2: {
                    //root["chain"] = root[2]["chain"];
                    Backpatch( root[2]["chain"], getLineNum(false) );
                    Emit( getLineNum(), "halt", -1, -1, -1 );
                } break;
            }
            return true;
        },
        /* 2. D 4 D int ID ; */
        [&](syntaxNode& root, int pos){
            ///把ID加入符号表中,并分配地址
            switch(pos) {
                case 1: {
                    root[2]["type"] = "int";
                } break;
                case 2: {
                    root[2]["name"] = root[2].ptrToken->lexData;

                    if ( mpSymbol.find(root[2]["name"].asString()) == mpSymbol.end() ) {
                        Emit(getLineNum(), "SYMBOL", root[2]["name"], root[2]["type"], -1);
                        mpSymbol.insert( root[2]["name"].asString() );
                    }
                    else {
                        string err = "Symbol[" + root[2]["name"].asString() + "] Redefined.";
                        vecError.push_back( err );
                    }
                }
            }
            return true;
        },
        /* 3. D 3 int ID ; */
        [&](syntaxNode& root, int pos){
            ///把ID加入符号表中,并分配地址
            switch(pos) {
                case 0: {
                    root[1]["type"] = "int";
                } break;
                case 1: {
                    root[1]["name"] = root[1].ptrToken->lexData;

                    if ( mpSymbol.find(root[1]["name"].asString()) == mpSymbol.end() ) {
                        Emit(getLineNum(), "SYMBOL", root[1]["name"], root[1]["type"], -1);
                        mpSymbol.insert( root[1]["name"].asString() );
                    }
                    else {
                        string err = "Symbol[" + root[1]["name"].asString() + "] Redefined.";
                        vecError.push_back( err );
                    }
                }
            }
            return true;
        },
        /* 4. S 9 if ( BEp ) then S else S endif */
        [&](syntaxNode& root, int pos){
            switch(pos) {
                case -1: break;
                case 0: case 1: break;
                case 2: {
                    Backpatch( root[2]["true"], Json::Value( getLineNum(false) ) );
                    root["chain_C"] = root[2]["false"];
                } break;
                case 5: {
                    int q = getLineNum(false);
                    Emit(getLineNum(), "goto", -1, -1, -1);
                    Backpatch(root["chain_C"], getLineNum(false));
                    root["chain_T"] = Merge( root[5]["chain"], Json::Value(q) );
                } break;
                case 7: {
                    root["chain"] = Merge( root["chain_T"], root[7]["chain"] );
                } break;
            }
            return true;
        },
        /* 5. S 7 if ( BEp ) then S endif */
        [&](syntaxNode& root, int pos){
            switch(pos) {
                case -1: break;
                case 0: case 1: break;
                case 2: {
                    Backpatch( root[2]["true"], Json::Value( getLineNum(false) ) );
                    root["chain_C"] = root[2]["false"];
                } break;
                case 5: {
                    root["chain"] = Merge( root["chain_C"], root[5]["chain"] );
                } break;
            }
            return true;
        },
        /* 6. S 6 while ( BEp ) do S */
        [&](syntaxNode& root, int pos){
            switch(pos) {
                case -1: break;
                case 0: {
                    root["codebegin_W"] = getLineNum(false);
                } break;
                case 2: {
                    Backpatch( root[2]["true"], getLineNum(false) );
                    root["chain_W"] = root[2]["false"];
                    root["codebegin_W2"] = root["codebegin_W"];
                } break;
                case 5: {
                    Backpatch( root[5]["chain"], root["codebegin_W2"] );
                    Emit(getLineNum(), "goto", -1, -1, root["codebegin_W2"].asInt() );
                    root["chain"] = root["chain_W"];
                } break;
            }
            return true;
        },
        /* 7. S 4 ID = CEp ; */
        [&](syntaxNode& root, int pos){
            switch(pos) {
                case -1: case 1: break;
                case 0: {
                    root[0]["name"] = root[0].ptrToken->lexData;
                } break;
                case 2: {
                    //root["chain"].append(-2);
                    if ( mpSymbol.find(root[0]["name"].asString()) == mpSymbol.end() ) {
                        string err = "Symbol[" + root[0]["name"].asString() + "] Not Found.";
                        vecError.push_back( err );
                    }
                    else {
                        Emit(getLineNum(), "Assign", root[2]["place"].asString(), -1, root[0]["name"] );
                    }

                } break;
            }
            return true;
        },
        /* 8. S 3 { CS } */
        [&](syntaxNode& root, int pos){
            switch(pos) {
                case -1: case 0: break;
                case 1: {
                    root["chain"] = root[1]["chain"];
                } break;
            }
            return true;
        },
        /* 9. CS 2 S CS */
        [&](syntaxNode& root, int pos){
            switch(pos) {
                case 0: {
                    Backpatch( root[0]["chain"], getLineNum(false) );
                } break;
                case 1: {
                    root["chain"] = root[1]["chain"];
                } break;
            }
            return true;
        },
        /* 10. CS 1 S */
        [&](syntaxNode& root, int pos){
            switch(pos) {
                case 0: {
                    root["chain"] = root[0]["chain"];
                } break;
            }
            return true;
        },
        /* 11. BEp 1 BET */
        [&](syntaxNode& root, int pos){
            switch(pos) {
                case -1: break;
                case 0: {
                    root["true"] = root[0]["true"];
                    root["false"] = root[0]["false"];
                    root["codebegin"] = root[0]["codebegin"];
                } break;
            }
            return true;
        },
        /* 12. BEp 3 BEp or BET */
        [&](syntaxNode& root, int pos){
            switch(pos) {
                case -1: break;
                case 0: case 1: break;
                case 2: {
                    Backpatch( root[0]["false"], root[2]["codebegin"] );

                    root["codebegin"] = root[0]["codebegin"];
                    root["false"] = root[2]["false"];
                    root["true"] = Merge( root[0]["true"], root[2]["true"] );

                } break;
            }
            return true;
        },
        /* 13. BET 1 BEF */
        [&](syntaxNode& root, int pos){
            switch(pos) {
                case -1: break;
                case 0: {
                    root["true"] = root[0]["true"];
                    root["false"] = root[0]["false"];
                    root["codebegin"] = root[0]["codebegin"];
                } break;
            }
            return true;
        },
        /* 14. BET 3 BET and BEF */
        [&](syntaxNode& root, int pos){
            switch(pos) {
                case -1: case 0: case 1: break;
                case 2: {
                    Backpatch( root[0]["true"], root[2]["codebegin"] );

                    root["codebegin"] = root[0]["codebegin"];
                    root["true"] = root[2]["true"];
                    root["false"] = Merge( root[0]["false"], root[2]["false"] );

                } break;
            }
            return true;
        },
        /* 15. BEF 3 CEp relop CEp */
        [&](syntaxNode& root, int pos){
            switch(pos) {
                case -1: case 0: case 1: break;
                case 2: {
                    root["true"].append( getLineNum(false) );
                    root["codebegin"].append( getLineNum(false) );
                    root["false"].append( getLineNum(false) + 1 );
                    Emit( getLineNum(),
                         string("if ")+root[1].ptrToken->lexData ,
                         root[0]["place"].asString(),
                         root[2]["place"].asString(),
                         -1
                          );
                    Emit( getLineNum(), "goto", -1, -1 , -1 );
                } break;
            }
            return true;
        },
        /* 16. BEF 1 CEp */
        [&](syntaxNode& root, int pos){
            switch(pos) {
                case -1: break;
                case 0: {
                    root["place"] = root[0]["place"];
                } break;
            }
            return true;
        },
        /* 17. CEp 1 CET */
        [&](syntaxNode& root, int pos){
            switch(pos) {
                case -1: break;
                case 0: {
                    root["place"] = root[0]["place"];
                } break;
            }
            return true;
        },
        /* 18. CEp 3 CEp + CET */
        [&](syntaxNode& root, int pos){
            switch(pos) {
                case -1: {
                    root["place"] = string("t") + clUtils::itoa( getNewTmp() );
                } break;
                case 2: {
                    Emit( getLineNum(), "+", root[0]["place"], root[2]["place"], root["place"] );
                } break;
            }
            return true;
        },
        /* 19. CEp 3 CEp - CET */
        [&](syntaxNode& root, int pos){
            switch(pos) {
                case -1: {
                    root["place"] = string("t") + clUtils::itoa( getNewTmp() );
                } break;
                case 2: {
                    Emit( getLineNum(), "-", root[0]["place"], root[2]["place"], root["place"] );
                } break;
            }
            return true;
        },
        /* 20. CET 1 CEF */
        [&](syntaxNode& root, int pos){
            switch(pos) {
                case -1: break;
                case 0: {
                    root["place"] = root[0]["place"];
                } break;
            }
            return true;
        },
        /* 21. CET 3 CET * CEF */
        [&](syntaxNode& root, int pos){
            switch(pos) {
                case -1: {
                    root["place"] = string("t") + clUtils::itoa( getNewTmp() );
                } break;
                case 2: {
                    Emit( getLineNum(), "*", root[0]["place"], root[2]["place"], root["place"] );
                } break;
            }
            return true;
        },
        /* 22. CET 3 CET / CEF */
        [&](syntaxNode& root, int pos){
            switch(pos) {
                case -1: {
                    root["place"] = string("t") + clUtils::itoa( getNewTmp() );
                } break;
                case 2: {
                    Emit( getLineNum(), "/", root[0]["place"], root[2]["place"], root["place"] );
                } break;
            }
            return true;
        },
        /* 23. CEF 1 ID */
        [&](syntaxNode& root, int pos){
            switch(pos) {
                case -1: break;
                case 0: {
                    root[0]["name"] = root[0].ptrToken->lexData;
                    if ( mpSymbol.find( root[0]["name"].asString() ) == mpSymbol.end() ) {
                        string err = "Symbol[" + root[0]["name"].asString() + "] Not Found.";
                        vecError.push_back( err );
                    }
                    root["place"] = root[0]["name"];
                } break;
            }
            return true;
        },
        /* 24. CEF 1 NUM */
        [&](syntaxNode& root, int pos){
            switch(pos) {
                case -1: {
                    root["place"] = string("t") + clUtils::itoa( getNewTmp() );
                } break;
                case 0: {
                    root["value"] = root[0].ptrToken->lexData;
                    Emit( getLineNum(), "Assign", root["value"],-1,root["place"]  );
                } break;
            }
            return true;
        },
        /* 25. CEF 3 ( CEp ) */
        [&](syntaxNode& root, int pos){
            switch(pos) {
                case -1: break;
                case 0: {
                    root["place"] = root[0]["place"];
                } break;
            }
            return true;
        },

    };

    return vector<TransFuncType>( transFuncArr, transFuncArr+sizeof(transFuncArr)/sizeof(TransFuncType) );
}
