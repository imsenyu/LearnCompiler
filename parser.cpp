#include "parser.h"

syntaxParser& syntaxParser::inputTerm(istream& in) {
/*
 * 先读取个数，然后读取id str isTerminal, 加入到 mpTerm中
 * 输入格式：
 * N
 * strname bool
 * strname bool
 */
    int N = 0;
    in>>N;
    for(int i=0;i<N;i++) {
        Term* addPtr = new Term(in);
        vecTerm.push_back(addPtr);
        mpTerm.insert( make_pair(addPtr->tName, addPtr) );
    }
    return *this;
}

syntaxParser& syntaxParser::showTerm() {
/*
 * Term构造函数的检验函数
 * 输出格式：
 * Terms(N)
 * strname bool
 * strname bool
 */
    printf("Terms(%d) Output at line %d:\n",mpTerm.size(), __LINE__);
    for(auto iter : mpTerm) {
        Term& iTerm = *iter.second;
        cout<<iTerm.tName<<" "<<iTerm.isTerminal<<endl;
    }
    return *this;
}

syntaxParser& syntaxParser::inputProduction(istream& in) {
/*
 * 先读取个数，然后读取from 和 to,加入到 mpATerm中
 * 输入格式：
 * N
 * strname M strname strname strname
 * strname M strname strname
 */
    int N = 0, M;
    Term* curTerm;
    string startNode, nextNode;
    in>>N;
    for(int i=0;i<N;i++) {
        M = 0;
        in>>startNode;
        in>>M;
        if ( NULL == (curTerm = HashFind( mpTerm, startNode )) ) {
            cout<<"["<<startNode<<"]";
            throw "Term Not Found";
        }

        Production* addPtr = new Production( vecATerm.size(), curTerm);
        for(int j=0;j<M;j++) {
            in>>nextNode;
            if ( NULL == (curTerm = HashFind( mpTerm, nextNode )) ) {
                cout<<"["<<nextNode<<"]";
                throw " Term Not Found";
            }
            addPtr->toTerms.push_back(curTerm);
        }

        vecATerm.push_back(addPtr);
        mpTerm.find(startNode)->second->vecPdtPtrs.push_back( addPtr );
    }
    return *this;
}

syntaxParser& syntaxParser::showProduction() {
/*
 * Production函数的检验函数
 * 输出格式：
 * Production(N)
 * strname
 *     strname => ...
 */
    if (!_isDebug) return *this;
    printf("Production(%d) Output at line %d:\n",vecATerm.size(), __LINE__);

    for(auto iter : mpTerm) {
        const string& startNode = iter.first;
        cout<<"["<<startNode<<"]"<<endl;
        vector<Production*>& vecPdts = iter.second->vecPdtPtrs;
        for( auto ptrPdt : vecPdts ) {
            printf("   ");ptrPdt->print(-1,true);
        }
    }
    return *this;
}

syntaxParser& syntaxParser::buildStateItems() {
    for(auto iter=vecATerm.begin();iter!=vecATerm.end();iter++) {
        Production& pdt = **iter;
        vector<StateItem*>& vecSItem = pdt.vecSItems;
        for(int j=0;j<=pdt.toTerms.size();j++) {
            StateItem* addPtr = new StateItem(&pdt, j);
            vecSItem.push_back(addPtr);
        }
    }
    return *this;
}

syntaxParser& syntaxParser::showStateItems() {
    if (!_isDebug) return *this;
    printf("StateItem Output at line %d:\n",__LINE__);
    for(auto iter=vecATerm.begin();iter!=vecATerm.end();iter++) {
        Production& pdt = **iter;
        vector<StateItem*>& vecSItem = pdt.vecSItems;
        pdt.print();
        for(auto jter=vecSItem.begin();jter!=vecSItem.end();jter++) {
            StateItem& sitem = **jter;
            printf("    "); sitem.print(true);
        }
    }
    return *this;
}

syntaxParser& syntaxParser::buildStateSet() {
    int curStateId = 0;
    vecStates.push_back( new StateSet(0) );

    ///需要知道起始Production,默认以 第一个production来算
    StateExtItem* beginSEItem = new StateExtItem( vecATerm[0]->vecSItems[0], endTermPtr );
    ///加入初始_S->*S, #状态，开始计算闭包
    vecStates[0]->collection.insert( beginSEItem );
    vecStates[0]->calcClosure();

    //vecStates[0]->print(true);
    ///记录访问过的状态集
    set< StateSet*, StateSet > visited;
    visited.insert( vecStates[0] );

    ///curStateId 小于等于该编号小的都已经完成 运算
    while( curStateId < vecStates.size() ) {
        //printf("curStateId = %d\n", curStateId);
        vector<Term*> vecOrderedTerm;
        map<Term*, vector<StateExtItem*>> term2SEItem;
        ///当前curStateId中有哪些可以走的Term*,分组放好到term2SEItem
        for( auto ptrSEItem : vecStates[curStateId]->collection ) {
            StateExtItem& SEItem = *ptrSEItem;
            Term* ptrTerm = SEItem.ptrItem->getNextTerm();
            if ( NULL == ptrTerm ) continue;
            if ( term2SEItem.find( ptrTerm ) == term2SEItem.end() ) {
                term2SEItem.insert( make_pair( ptrTerm, vector<StateExtItem*>() ) );
                vecOrderedTerm.push_back(ptrTerm);
            }
            term2SEItem.find(ptrTerm)->second.push_back( &SEItem );
        }
        ///按照OrderedTerm的顺序遍历Term*
        for( auto ptrTerm : vecOrderedTerm ) {
            //ptrTerm->print(true);
            vector<StateExtItem*>& vecMapSEItem = term2SEItem.find(ptrTerm)->second;
            StateSet* newStateSet = new StateSet( vecStates.size() );
            ///对给定Term*,遍历所有可以通过Term*往下走的SEItem,可以走则加入collection
            for( auto ptrSEItem : vecMapSEItem) {
                if ( ptrSEItem->hasNextSItem() )
                    newStateSet->collection.insert( new StateExtItem( ptrSEItem->getNextSItem(), ptrSEItem->next ) );
            }
            ///该状态走不了，删除
            if ( 0 == newStateSet->collection.size() ) {
                delete newStateSet;
                continue;
            }
            newStateSet->calcClosure();

            ///得到的新状态判重,如果重复，标记回去的路径，并删除该状态
            if ( visited.find( newStateSet ) != visited.end() ) {
                mpStateTable.add( curStateId, ptrTerm, (*visited.find( newStateSet ))->sId );
                delete newStateSet;
                continue;
            }
            ///不重复新状态，加入，标记为访问过吗加入转移项
            vecStates.push_back( newStateSet );
            visited.insert( newStateSet );
            mpStateTable.add( curStateId, ptrTerm, newStateSet->sId );
        }
        curStateId++;
    }

    return *this;
}

syntaxParser& syntaxParser::showStateSet() {
    if (!_isDebug) return *this;
    printf("StateSet Output at line %d:\n",__LINE__);
    for(auto ptrStateSet : vecStates ) {
        ptrStateSet->print(true);
    }
    printf("StateSet - Graph Output at line %d:\n",__LINE__);
    for(auto row : mpStateTable) {
        int stateFromId = row.first;
        for(auto col : row.second) {
            int stateToId = col.second;
            printf("  I(%d) ", stateFromId);
            col.first->print(false);
            printf(" I(%d)\n",stateToId);
        }
    }

    return *this;
}

syntaxParser& syntaxParser::buildActionGotoTable() {
    ///需要得到 map<int, map<Term*, AGState>>

    ptrAGTable = new ActionGotoTable( &mpStateTable, &vecStates, vecATerm[0] );
    ptrAGTable->build();

    return *this;
}

syntaxParser& syntaxParser::showActionGotoTable() {
    if (!_isDebug) return *this;
    printf("ActionGotoTable Output at line %d:\n",__LINE__);

    vector<Term*> termsWithEnd = vecTerm;
    termsWithEnd.push_back(endTermPtr);
    printf("\t");
    for(auto ptrTerm : termsWithEnd) {
        ptrTerm->print(false);
        printf("\t");
    }
    printf("\n");
    auto& table = ptrAGTable->table;
    for(auto row : table ) {
        int vId = row.first;
        printf("I%d\t",vId);
        for(auto ptrTerm : termsWithEnd) {
            Action* action = table.get( vId, ptrTerm );
            if ( NULL != action ) {
                action->print(false);
            }
            printf("\t");
        }
        printf("\n");
    }
    return *this;
}

syntaxParser& syntaxParser::inputLex(istream& in) {
    string bufTerm, bufLex;
    int bufId;
    while( in>>bufTerm>>bufId>>bufLex ) {
        Term* ptrFoundTerm = NULL;
        if ( NULL == (ptrFoundTerm = HashFind(mpTerm, bufTerm)) ) {
            cout<<"["<<bufTerm<<"]";
            printf(" Term Not Found, SKIPED\n");
            continue;
        }
        ///每次读入 构造一个 Token, 加入队列中
        vecToken.push_back( new Token( ptrFoundTerm, bufLex ) );
    }
    vecToken.push_back(new Token( endTermPtr, string("#") ));
    if (!_isDebug) return *this;
    for(auto ptrToken : vecToken) {
        ptrToken->print(true);
    }

    return *this;
}

syntaxParser& syntaxParser::runSyntaxAnalyse() {
    stack<int> stkState; ///状态号-栈
    stack<syntaxNode*> stkSyntaxNode; ///语法树(已规约词)-栈
    D2Map<int,Term*,Action>& table = ptrAGTable->table; ///ActionGoto表

    stkState.push(0); ///压入0
    stkSyntaxNode.push( new syntaxNode(*vecToken.rbegin()) ); ///压入 (endTermPtr, "#")这个终结符，方便最后比较

    int vTokenCnt = 0;  ///当前已读入词
    bool isRunnable = true;
    while( vTokenCnt < vecToken.size() && isRunnable) {
        int topState = stkState.top();
        Token* nextToken = vecToken[ vTokenCnt ];

        printf("topState = %d, vTokenCnt = %d\n",topState, vTokenCnt);
        printf("  ");nextToken->ptrTerm->print(true);
        Action* curAction = table.get(topState, nextToken->ptrTerm);
        if ( NULL == curAction ) {
            ///true 代表解决成功, false 代表解决失败 throw
            if ( false == solveSyntaxException(stkState, stkSyntaxNode, vTokenCnt) ) {
                printf("\nSyntax Error at topState=%d vTokenCnt = %d\n", topState,vTokenCnt);
                throw "Error Can't be solved";
            }
            vTokenCnt++;
            continue;
        }

        switch(curAction->type) {
            case Action::Type::Step: {
                    stkState.push( curAction->toId );
                    stkSyntaxNode.push( new syntaxNode( nextToken ) );
                    vTokenCnt++;
                } break;
            case Action::Type::Recur: {
                    ///使用 Production[j] 规约
                    Production* ptrPdt = vecATerm[curAction->toId];
                    printf("  ");ptrPdt->print(-1,true);
                    for(int i=0;i<ptrPdt->toTerms.size();i++)
                        stkState.pop();
                    topState = stkState.top();

                    printf("  StatePoped: %d\n",topState);

                    curAction = table.get( topState, ptrPdt->ptrTerm );
                    if ( NULL == curAction || curAction->type != Action::Type::Goto ) {
                        ///stkState 弹出k次， 加入goto[ 新栈顶, 规则左部Term* ],，不应该没有，没有说明AGT打错了
                        throw "  Need Goto\n";
                    }
                    stkState.push( curAction->toId );
                    ///stkSyntaxNode 弹出的k次加入 new syntaxNode( nextToken ),把弹出的加入到 这个new的child里面, 设置这个new的Production*
                    syntaxNode* parent = new syntaxNode( nextToken, ptrPdt );
                    for(int i=0;i<ptrPdt->toTerms.size();i++) {
                        syntaxNode* child = stkSyntaxNode.top();
                        child->ptrParent = parent;
                        parent->child.push_front( child );
                        stkSyntaxNode.pop();
                    }
                    stkSyntaxNode.push( parent );
                }
                break;
            case Action::Type::Acc:
                printf("Acc\n");
                isRunnable = false;
                break;
        }

    }

    if ( 1 > stkSyntaxNode.size() ) {
        throw "Can't buildTree\n";
        return *this;
    }
    ptrSyntaxTreeRoot = stkSyntaxNode.top();
    stkSyntaxNode.pop();
    while( !stkSyntaxNode.empty() ) {
        delete stkSyntaxNode.top();
        stkSyntaxNode.pop();
    }

    return *this;
}

syntaxParser& syntaxParser::showSyntaxTree() {
    if ( !_isDebug ) return *this;
    ptrSyntaxTreeRoot->print(true);
    return *this;
}

syntaxParser& syntaxParser::ConstructLR1(istream& grammarIn) {

    destoryAll().
    inputTerm(grammarIn).
    showTerm().
    inputProduction(grammarIn).
    showProduction().
    buildStateItems().
    showStateItems().
    buildStateSet().
    showStateSet().
    buildActionGotoTable().
    showActionGotoTable().
    nop();

    _isLR = true;

    return *this;
}

syntaxParser& syntaxParser::ConstructTree(istream& lexIn) {
    if ( false == _isLR ) return *this;

    inputLex(lexIn).
    runSyntaxAnalyse().
    showSyntaxTree().
    nop();

    _isTree = true;

    return *this;
}
