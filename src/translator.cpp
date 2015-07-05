#include "translator.h"

Translator& Translator::init( syntaxParser* _newParser ) {
    if ( _newParser != ptrParser && NULL != ptrParser) {
        delete ptrParser;
        ptrParser = _newParser;
    }
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
        f(*root, c);
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

    translateRecur(ptrParser->getSyntaxTree(), vecTransFunc);
    printf("\n");
    return *this;
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
                    root["place"] = new int( getNewTmp() );
                } break;
                case 0: {
                    PASS<int>( root["value"], root[0]["value"] );
                    printf("L-%d\t", getLineNum());
                    printf("t%d = t%d\n", GET<int>( root["place"] ), GET<int>( root[0]["place"] )  );
                    printf("Need t%d = %d\n", GET<int>( root["place"] ), GET<int>( root["value"] ) );
                } break;
            }
            return true;
        },
        /* 2 E-> T */
        [&](syntaxNode& root, int pos){
            switch(pos) {
                case -1:  break;
                case 0: {
                    PASS<int>( root["value"], root[0]["value"] );
                    PASS<int>( root["place"], root[0]["place"] );
                } break;
            }
            return true;
        },
        /* 3 E-> E + T */
        [&](syntaxNode& root, int pos){
            switch(pos) {
                case -1: {
                    PASS<int>( root["place"], new int(getNewTmp()) );
                } break;
                case 0: case 1: break;
                case 2: {
                    PASS<int>( root["value"], new int(  GET<int>( root[0]["value"] ) + GET<int>( root[2]["value"] ) ) );
                    printf("L-%d\t",getLineNum());
                    printf("t%d = t%d + t%d\n", GET<int>(root["place"]), GET<int>(root[0]["place"]), GET<int>(root[2]["place"]));
                } break;
            }
            return true;
        },
        /* 4 T-> P */
        [&](syntaxNode& root, int pos){
            switch(pos) {
                case -1: break;
                case 0: {
                    PASS<int>( root["value"], root[0]["value"] );
                    PASS<int>( root["place"], root[0]["value"] );
                } break;
            }
            return true;
        },
        /* 5 T-> T * P */
        [&](syntaxNode& root, int pos){
            switch(pos) {
                case -1: {
                    PASS<int>( root["place"], new int( getNewTmp() ) );
                } break;
                case 0: case 1: break;
                case 2: {
                    PASS<int>( root["value"], new int(  GET<int>( root[0]["value"] ) * GET<int>( root[2]["value"] ) ) );
                    printf("L-%d\t",getLineNum());
                    printf("t%d = t%d * t%d\n", GET<int>(root["place"]), GET<int>(root[0]["place"]), GET<int>(root[2]["place"]));
                } break;
            }
            return true;
        },
        /* 6 P-> NUM */
        [&](syntaxNode& root, int pos){
            switch(pos) {
                case -1: {
                    PASS<int>( root["place"], new int( getNewTmp() ) );
                } break;
                case 0: {
                    PASS<int>( root["value"], new int( clUtils::atoi( root[0].getLex() ) ) );
                    printf("L-%d\t", getLineNum());
                    printf("t%d = [%d]\n", GET<int>(root["place"]), GET<int>(root["value"]));
                } break;
            }
            return true;
        },
        /* 7 P-> ( E ) */
        [&](syntaxNode& root, int pos){
            switch(pos) {
                case -1: case 0: case 1: break;
                case 2: {
                    PASS<int>( root["value"], root[1]["value"] );
                    PASS<int>( root["place"], root[1]["place"] );
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
    TransFuncType transFuncArr[] = {
        /* 0. _P 1 P */
        [&](syntaxNode& root, int pos){ return true;/* do nothing */ },
        /* 1. P 4 { D CS } */
        [&](syntaxNode& root, int pos){
            return true;
        },
        /* 2. D 4 D int ID ; */
        [&](syntaxNode& root, int pos){
            ///如果搞成  int ID, ID, ID 需要 往上传递"type"属性
            ///把ID加入符号表中,并分配地址
            switch(pos) {
                case -1: case 0: case 2: break;
                case 1: {
                    PASS<int>( root["place"], new int(-1));
                } break;
            }
            return true;
        },
        /* 3. D 3 int ID ; */
        [&](syntaxNode& root, int pos){
            ///把ID加入符号表中,并分配地址
            switch(pos) {
                case -1: case 0: case 2: break;
                case 1: {
                    PASS<int>( root["place"], new int(-1));
                } break;
            }
            return true;
        },
        /* 4. S 9 if ( BEp ) then S else S endif */
        [&](syntaxNode& root, int pos){
            ///获取BEp的结果place，
            ///需要做label标记
            switch(pos) {
                case -1: case 0: case 2: break;
                case 1: {
                    PASS<int>( root["place"], new int(-1));
                } break;
            }
            return true;
        },
        /* 5. S 7 if ( BEp ) then S endif */
        [&](syntaxNode& root, int pos){
            return true;
        },
        /* 6. S 6 while ( BEp ) do S */
        [&](syntaxNode& root, int pos){
            return true;
        },
        /* 7. S 4 ID = CEp ; */
        [&](syntaxNode& root, int pos){
            return true;
        },
        /* 8. S 3 { CS } */
        [&](syntaxNode& root, int pos){
            return true;
        },
        /* 9. CS 2 S CS */
        [&](syntaxNode& root, int pos){
            return true;
        },
        /* 10. CS 1 S */
        [&](syntaxNode& root, int pos){
            return true;
        },
        /* 11. BEp 1 BET */
        [&](syntaxNode& root, int pos){
            return true;
        },
        /* 12. BEp 3 BEp or BET */
        [&](syntaxNode& root, int pos){
            return true;
        },
        /* 13. BET 1 BEF */
        [&](syntaxNode& root, int pos){
            return true;
        },
        /* 14. BET 3 BET and BEF */
        [&](syntaxNode& root, int pos){
            return true;
        },
        /* 15. BEF 3 CEp relop CEp */
        [&](syntaxNode& root, int pos){
            return true;
        },
        /* 16. BEF 1 CEp */
        [&](syntaxNode& root, int pos){
            return true;
        },
        /* 17. CEp 1 CET */
        [&](syntaxNode& root, int pos){
            return true;
        },
        /* 18. CEp 3 CEp + CET */
        [&](syntaxNode& root, int pos){
            return true;
        },
        /* 19. CEp 3 CEp - CET */
        [&](syntaxNode& root, int pos){
            return true;
        },
        /* 20. CET 1 CEF */
        [&](syntaxNode& root, int pos){
            return true;
        },
        /* 21. CET 3 CET * CEF */
        [&](syntaxNode& root, int pos){
            return true;
        },
        /* 22. CET 3 CET / CEF */
        [&](syntaxNode& root, int pos){
            return true;
        },
        /* 23. CEF 1 ID */
        [&](syntaxNode& root, int pos){
            return true;
        },
        /* 24. CEF 1 NUM */
        [&](syntaxNode& root, int pos){
            return true;
        },
        /* 25. CEF 3 ( CEp ) */
        [&](syntaxNode& root, int pos){
            return true;
        },

    };

    return vector<TransFuncType>( transFuncArr, transFuncArr+sizeof(transFuncArr)/sizeof(TransFuncType) );
}
