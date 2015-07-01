#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <set>
#include <stack>
#include <map>
#include <algorithm>
#include <functional>
#include <fstream>

#include "clUtils.h"
#include "token.h"
//#include "closure.h"
#include "syntaxTree.h"

using namespace std;

class syntaxParser;

class syntaxParser {
public:
    bool isDebug;
public:
    ///所有数据存储使用vector
    vector<Term*> vecTerm;
    vector<Production*> vecATerm;
    vector<StateSet*> vecStates;

    ///供数据查找
    map<string, Term*> mpTerm; ///TermMap
    map<string, vector<Production*>> mpATerm; ///ProductionMap
   // map<Production*, vector<StateItem*>> mpSItems; ///存储兼查找
    D2Map<int,Term*,int> mpStateTable;/// stateset - ATerm* 的二维查找表
    ActionGotoTable* ptrAGTable;
    //map<int, map<Term*, int>> mpStateTable;/// stateset - ATerm* 的二维查找表
    ///供标号，delete

    //map<int, map<Production*
    ///供顺序读取
    vector<Token*> vecToken;

#define hashFind( _T, _key, _success, _fail) {  auto iter = _T.find(_key); if ( iter != _T.end() ) _success else _fail   }
    enum HashFindType {mapSecond};
    template<class _T, class _KEY>
    void* HashFind(_T container, _KEY key, HashFindType _type) {
        typename _T::iterator iter = container.find( key );
        if ( iter != container.end() ) {
            switch(_type) {
                case mapSecond:     return (void*)iter->second;
                default: return NULL;
            }
        }
        else return NULL;
    }

    syntaxParser(bool _debug = true):isDebug(_debug), ptrAGTable(NULL) {  }
    syntaxParser& inputTerm(istream& in) {
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

    syntaxParser& showTerm() {
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

    syntaxParser& inputProduction(istream& in) {
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
            curTerm = (Term*)HashFind( mpTerm, startNode, mapSecond );
            if ( curTerm == NULL ) {
                cout<<"["<<startNode<<"]";
                throw "Term Not Found";
            }

            Production* addPtr = new Production( vecATerm.size(),  curTerm);
            for(int j=0;j<M;j++) {
                in>>nextNode;
                curTerm = (Term*)HashFind( mpTerm, nextNode, mapSecond );
                if ( curTerm == NULL ) {
                    cout<<"["<<nextNode<<"]";
                    throw " Term Not Found";
                }
                addPtr->toTerms.push_back(curTerm);
            }

            vecATerm.push_back(addPtr);
            if ( mpATerm.find( startNode ) == mpATerm.end() ) {
                mpATerm.insert( make_pair(startNode, vector<Production*>() ) );
            }
            mpTerm.find(startNode)->second->vecPdtPtrs.push_back( addPtr );
            mpATerm.find(startNode)->second.push_back( addPtr );
        }
        return *this;
    }

    syntaxParser& showProduction() {
    /*
     * Production函数的检验函数
     * 输出格式：
     * Production(N)
     * strname
     *     strname => ...
     */
        if (!isDebug) return *this;
        printf("Production(%d), TermStart(%d) Output at line %d:\n",vecATerm.size(), mpATerm.size(), __LINE__);

        for(auto iter : mpATerm) {
            const string& startNode = iter.first;
            cout<<"["<<startNode<<"]"<<endl;
            vector<Production*>& vecPdts = iter.second;
            for( auto ptrPdt : vecPdts ) {
                printf("   ");ptrPdt->print(-1,true);
            }
        }
        return *this;
    }

    syntaxParser& buildStateItems() {
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

    syntaxParser& showStateItems() {
        if (!isDebug) return *this;
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

    syntaxParser& buildStateSet() {
        int curStateId = 0;

        vecStates.push_back( new StateSet(0) );

        ///需要知道begin production,默认以 第一个production来算
        StateExtItem* beginSEItem = new StateExtItem( vecATerm[0]->vecSItems[0], endTermPtr );
        vecStates[0]->collection.insert( beginSEItem );
        vecStates[0]->calcClosure();

        vecStates[0]->print(true);

        set< StateSet*, StateSet > visited;
        visited.insert( vecStates[0] );

        while( curStateId < vecStates.size() ) {
        ///curStateId 小于等于该编号小的都已经完成 运算
        ///并且拿 curStateId 进行 step下一步, 对于构造出的每一个
        ///构造出一个算一个闭包，加入判重
            printf("curStateId = %d\n", curStateId);
            vector<Term*> vecOrderedTerm;
            map<Term*, vector<StateExtItem*>> term2SEItem;
            for( auto ptrSEItem : vecStates[curStateId]->collection ) {
                ///需要经过的是 B 而不是 startNode，改成B
                StateExtItem& SEItem = *ptrSEItem;
                Term* ptrTerm = SEItem.ptrItem->getNextTerm();
                if ( NULL == ptrTerm ) continue;
                if ( term2SEItem.find( ptrTerm ) == term2SEItem.end() ) {
                    term2SEItem.insert( make_pair( ptrTerm, vector<StateExtItem*>() ) );
                    vecOrderedTerm.push_back(ptrTerm);
                }
                term2SEItem.find(ptrTerm)->second.push_back( &SEItem );
            }
            for( auto ptrTerm : vecOrderedTerm ) {
                printf("B begin\n");

                //Term* ptrTerm;
                ptrTerm->print(true);
                vector<StateExtItem*>& vecMapSEItem = term2SEItem.find(ptrTerm)->second;

                StateSet* newStateSet = new StateSet( vecStates.size() );
                for( auto ptrSEItem : vecMapSEItem) {

                    if ( ptrSEItem->hasNextSItem() ) {
                        newStateSet->collection.insert( new StateExtItem( ptrSEItem->getNextSItem(), ptrSEItem->next ) );
                        printf("ok ");
                    }
                    printf("  ");ptrSEItem->print(true);
                }
                printf("B closure Start\n");
                if ( 0 == newStateSet->collection.size() ) {
                    ///走不了的
                    printf("  Deleted!!!\n");
                    delete newStateSet;
                    continue;
                }
                newStateSet->calcClosure();
                printf("B closure\n");
                newStateSet->print(true);
                printf("B end\n");

                if ( visited.find( newStateSet ) != visited.end() ) {
                    ///判断是否重复. 自行维护一个 set<collection>
                    ///不过需要标记一下 走这个Term 可以回去
                    ///curID & curTerm -> 旧id

                    mpStateTable.add( curStateId, ptrTerm, (*visited.find( newStateSet ))->sId );
                    delete newStateSet;
                    continue;
                }

                ///准备加入新状态
                ///还要构造 StateTable

                vecStates.push_back( newStateSet );
                visited.insert( newStateSet );
                mpStateTable.add( curStateId, ptrTerm, newStateSet->sId );
                ///在table 中 标记一下 curId & curTerm -> 这个新id
            }

            curStateId++;
        }

        return *this;
    }

    syntaxParser& showStateSet() {
        if (!isDebug) return *this;
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

    syntaxParser& buildActionGotoTable() {
        ///需要得到 map<int, map<Term*, AGState>>

        ptrAGTable = new ActionGotoTable( &mpStateTable, &vecStates, vecATerm[0] );
        ptrAGTable->build();

        return *this;
    }

    syntaxParser& showActionGotoTable() {
        if (!isDebug) return *this;
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

    syntaxParser& inputLex(istream& in) {
        string bufTerm, bufLex;
        int bufId;
        while( in>>bufTerm>>bufId>>bufLex ) {
            Term* ptrFoundTerm = NULL;
            if ( mpTerm.find( bufTerm ) == mpTerm.end() ) {
                cout<<bufTerm;
                printf(" Term Not Found, SKIPED\n");
                continue;
            }
            ptrFoundTerm = (Term*)HashFind(mpTerm, bufTerm, mapSecond);
            ///每次读入 构造一个 Token, 加入队列中
            vecToken.push_back( new Token( ptrFoundTerm, bufLex ) );
        }
        vecToken.push_back(new Token( endTermPtr, string("#") ));
        if (!isDebug) return *this;
        for(auto ptrToken : vecToken) {
            ptrToken->print(true);
        }

        return *this;
    }

    syntaxParser& runSyntaxAnalyse() {
        stack<int> stkState;
        stack<syntaxNode*> stkSyntaxNode;
        D2Map<int,Term*,Action>& table = ptrAGTable->table;

        stkState.push(0);
        stkSyntaxNode.push( new syntaxNode(*vecToken.rbegin()) );

        int vTokenCnt = 0;
        bool isRunnable = true;
        while( vTokenCnt < vecToken.size() && isRunnable) {
            int topState = stkState.top();
            Token* nextToken = vecToken[ vTokenCnt ];

            printf("topState=%d  %d, vTokenCnt = %d\n",topState, stkState.size(), vTokenCnt);
            printf("  ");nextToken->ptrTerm->print(false);
            Action* curAction = table.get(topState, nextToken->ptrTerm);
            if ( NULL == curAction ) {
                ///这个错就是没有合适匹配，具体怎么处理思考一下,跳过当前Token读下一个。  或者，目前先按照吃掉一个再说
                ///并说明当前接受哪些Term读入
                ///出错，未找到Action
                printf("Error at topState=%d vTokenCnt = %d\n",topState,vTokenCnt);
                vTokenCnt++;
                continue;
            }
            nextToken->print(false);curAction->print(true);

            switch(curAction->type) {
                case Action::Type::Step:
                    {
                        stkState.push( curAction->toId );
                        stkSyntaxNode.push( new syntaxNode( nextToken ) );
                        vTokenCnt++;
                    }
                    break;
                case Action::Type::Recur:
                    {
                        ///使用 Production[j] 规约
                        Production* ptrPdt = vecATerm[curAction->toId];
                        ptrPdt->print(-1,true);
                        for(int i=0;i<ptrPdt->toTerms.size();i++)
                            stkState.pop();
                        topState = stkState.top();

                        printf("  StatePop: %d\n",topState);

                        curAction = table.get( topState, ptrPdt->ptrTerm );
                        if ( NULL == curAction || curAction->type != Action::Type::Goto ) {
                            ///stkState 弹出k次， 加入goto[ 新栈顶, 规则左部Term* ],，不应该没有，没有说明AGT打错了
                            printf("  Need Goto\n");
                        }
                        stkState.push( curAction->toId );
                        ///stkSyntaxNode 弹出的k次加入 new syntaxNode( nextToken ),把弹出的加入到 这个new的child里面, 设置这个new的Production*
                        syntaxNode* parent = new syntaxNode( nextToken, ptrPdt );
                        for(int i=0;i<ptrPdt->toTerms.size();i++) {
                            parent->child.push_front( stkSyntaxNode.top() );
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

            ///实时构造语法树


        }

        printf("Tree:%d\n",stkSyntaxNode.size());
        while( !stkSyntaxNode.empty() ) {
            stkSyntaxNode.top()->print();
            stkSyntaxNode.pop();

        }

        return *this;
    }

    void nop() {}
    //syntaxParse* buildStateTable
};

int main()
{

    fstream fileSyntax, fileLex;
    fileSyntax.open("cfg.146.txt",ios_base::in );
    fileLex.open("lex.146.txt", ios_base::in);

    syntaxParser parser;
    parser.
        inputTerm(fileSyntax).
        showTerm().
        inputProduction(fileSyntax).
        showProduction().
        buildStateItems().
        showStateItems().
        buildStateSet().
        showStateSet().
        buildActionGotoTable().
        showActionGotoTable().
        inputLex(fileLex).
        runSyntaxAnalyse().
        nop();

    cout << "Hello world!" << endl;
    return 0;
}
