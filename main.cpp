#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <functional>
#include <cassert>

#include "clUtils.h"
#include "token.h"
#include "closure.h"
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
            }
        }
        else return NULL;
    }

    syntaxParser(bool _debug = true):isDebug(_debug) {  }
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
        for(auto iter = mpTerm.begin();iter!=mpTerm.end();iter++) {
            Term& iTerm = *(iter->second);
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

            Production* addPtr = new Production(curTerm);
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

        for(auto iter=mpATerm.begin(); iter!=mpATerm.end(); iter++) {
            string startNode = (iter->first);
            cout<<"["<<iter->first<<"]"<<endl;
            vector<Production*>& pdts = (iter->second);
            for(auto jter=pdts.begin(); jter!=pdts.end(); jter++ ) {
                Production& pdt = **jter;
                printf("   ");pdt.print(-1,true);
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
            map<Term*, vector<StateExtItem*>> term2SEItem;
            for( auto ptrSEItem : vecStates[curStateId]->collection ) {

                StateExtItem& SEItem = *ptrSEItem;
                Term& term = *SEItem.ptrItem->ptrPdt->ptrTerm;
                if ( term2SEItem.find( &term ) == term2SEItem.end() ) {
                    term2SEItem.insert( make_pair( &term, vector<StateExtItem*>() ) );
                }
                term2SEItem.find(&term)->second.push_back( &SEItem );
            }
            for( auto& link : term2SEItem ) {
                printf("B begin\n");

                Term* ptrTerm = link.first;
                ptrTerm->print(true);
                vector<StateExtItem*>& vecMapSEItem = link.second;

                StateSet* newStateSet = new StateSet( vecStates.size() );
                for( auto ptrSEItem : vecMapSEItem) {

                    if ( ptrSEItem->hasSItemNext() ) {
                        newStateSet->collection.insert( new StateExtItem( ptrSEItem->getSItemNext(), ptrSEItem->next ) );
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
//            for( auto ptrSEItem : vecStates[curStateId]->collection ) {
//                //StateExtItem* ptrSEItem;
//                ///这么写不对，需要按照 sameTerm 放在一起做, 否则算出来的闭包不对，是个NFA了
//                ///需要有一个 Term* <-> vector<StateExtItem*> 的映射,根据那个来算 闭包
//                StateSet* newStateSet = new StateSet( vecStates.size() );
//                if ( ptrSEItem->ptrItem->hasNext() ) {
//
//                }
//                else {
//                    ///走不了的
//                    delete newStateSet;
//                    continue;
//                }
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
        for(auto stateRow : mpStateTable ) {
            int stateId = stateRow.first;
            vecStates[stateId]->print(true);
        }
    }

    void nop() {}
    //syntaxParse* buildStateTable
};

int main()
{
    freopen("cfg.txt", "r",stdin);
    syntaxParser parser;
    parser.
        inputTerm(cin).
        showTerm().
        inputProduction(cin).
        showProduction().
        buildStateItems().
        showStateItems().
        buildStateSet().
        showStateSet().
        nop();

    cout << "Hello world!" << endl;
    return 0;
}
