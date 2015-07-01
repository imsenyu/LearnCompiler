#ifndef TOKEN_H_INCLUDED
#define TOKEN_H_INCLUDED


///词法数据类型，再加一个 tokenConfig.h 自定义语言词法
#include <iostream>
#include <cstdio>
#include <vector>
#include <string>
#include <set>
#include <functional>

#include "clUtils.h"

using namespace std;

class Term;
class Production;
class Token;
class StateItem;
class StateExtItem;
class StateSet;

///单词
///需要 map: tName <-> Term*
class Term {
public:
    string tName;
    bool isTerminal;
    vector<Production*> vecPdtPtrs;
    Term(string _name = "", bool _terminal = true): tName(_name), isTerminal(_terminal) {}
    Term(istream& in) {
        in>>tName>>isTerminal;
    }
    void print(bool breakLine) {
        cout<<"["<<tName<<"]";
        breakLine && printf("\n");
    }
};
 Term* const nullTermPtr = new Term("",true);
 Term* const endTermPtr = new Term("#",true);

///读取词法分析结果够构造Token vector序列用，跑AG表的时候按顺序读取
class Token {
public:
    Term* ptrTerm;
    string lexData;
    Token(Term* _ptr, const string& _str): ptrTerm(_ptr), lexData(_str) {}
    void print(bool breakLine = true) {
        ptrTerm->print(false);
        cout<<" "<<lexData;
        breakLine && printf("\n");
    }
};

/*
///产生式
///需要 map: Term* <-> vector<Production*>
///允许 使用 一个 Term->isTerminal == true 的元素 当 Production的 终结节点
class Production {
public:
    Term* tFrom;
    vector<Term*> toTerms;
    Production(Term* _from = NULL): tFrom(_from) {}
};
*/
///Prodution，
class Production {
public:
    int pId;
    Term* ptrTerm;
    bool isTerminal;
    vector<Term*> toTerms;
    vector<StateItem*> vecSItems;
    Production( int _id, Term* _ptr = NULL , bool _terminal = false): pId(_id), ptrTerm(_ptr), isTerminal(_terminal) {}
    void print(int pos = -1, bool breakLine = true) {
        printf("[%d] ",pId);
        ptrTerm->print(false);
        if ( false == ptrTerm->isTerminal ) {
            printf(" => ");
            int cnt = 0;
            for(auto iPtrTerm : toTerms) {
                if ( cnt == pos ) {
                    printf("* ");
                }
                iPtrTerm->print(false);
                printf(" ");
                cnt++;
            }
            if ( cnt == pos ) {
                printf("* ");
            }
        }
        breakLine && printf("\n");
    }
};

///LR项目，带·位置pos和LR1需要考虑的下一个字符
class StateItem {
public:
    Production* ptrPdt;
    int pos;
    StateItem(Production* _ptr = NULL, int _pos = 0): ptrPdt(_ptr), pos(_pos) {}
    bool hasNext() {
        if ( NULL == ptrPdt ) return false;
        return pos < ptrPdt->toTerms.size();// && true == ptrPdt->toTerms[pos]->isTerminal;
    }
    Term* getNextTerm() {
        if ( NULL == ptrPdt || pos >= ptrPdt->toTerms.size()) return NULL;
        return ptrPdt->toTerms[ pos ];
    }
    Term* getFromTerm() {
        if ( NULL == ptrPdt ) return NULL;
        return ptrPdt->ptrTerm;
    }
    StateItem* getNextSItem() {
        if ( NULL == ptrPdt ) return NULL;
        return ptrPdt->vecSItems[ pos+1 ];
    }
    void print(bool breakLine) {
        if ( NULL != ptrPdt )
            ptrPdt->print(pos, breakLine);
    }
};

class StateExtItem {
public:
    StateItem* ptrItem;
    Term *next;
    StateExtItem(StateItem* _ptr = NULL, Term *_next = endTermPtr): ptrItem(_ptr), next(_next) {}
    bool operator()(const StateExtItem* a, const StateExtItem* b) const {
        return (*a) < (*b);
    }
    bool operator==(const StateExtItem& b) const {
        return ptrItem == b.ptrItem && next == b.next;
    }
    bool operator!=(const StateExtItem& b) const {
        return !(*this == b);
    }
    bool operator<(const StateExtItem& b) const {
        if ( ptrItem != b.ptrItem ) return ptrItem < b.ptrItem;
        return next < b.next;
    }
    bool hasNextSItem() {
        if ( NULL == ptrItem ) return false;
        return ptrItem->hasNext();
    }
    Term* getFromTerm() {
        if ( NULL == ptrItem ) return NULL;
        return ptrItem->getFromTerm();
    }
    StateItem* getNextSItem() {
        if ( NULL == ptrItem ) return NULL;
        return ptrItem->getNextSItem();
    }
    void print(bool breakLine) {
        if ( ptrItem ) {
            ptrItem->print(false);
            printf("  char"); next->print(false);
            breakLine && printf("\n");
        }
    }
};

///LR项目集状态，
class StateSet {
private:
    set<Term*> calcFirstSet(vector<Term*>& after) {
        set<Term*> firstSet;
        ///准备一个带路径回溯的dfs
        ///根据after中的一个个开始扫描

        ///如果你能找到 true就反
        ///还需要记录一个dfs的visit
        function<bool(vector<Term*>&)> dfs = [&dfs,&firstSet](vector<Term*>& terms) -> bool{
            bool ret = false;
            for(auto ptrTerm : terms ) {
                Term& curTerm = *ptrTerm;
                if ( false == curTerm.isTerminal ) {
                    ///遍历所有production,进去dfs
                    vector<Production*>& vecPdtPtr = curTerm.vecPdtPtrs;
                    for(auto ptrPdt : vecPdtPtr) {
                        Production& curPdt = *ptrPdt;
                        ret |= dfs( curPdt.toTerms );
                    }
                    if ( ret ) break;
                }
                else {
                    firstSet.insert( &curTerm );
                    ret = true;
                    break;
                }
            }
            return ret;
        };

        dfs(after);

        return firstSet;
    }
    bool splitStateExtItem( StateExtItem& SEItem, Term* &NTerm, vector<Term*>& after ) {
        vector<Term*> &toTerms = SEItem.ptrItem->ptrPdt->toTerms;
        int step = SEItem.ptrItem->pos;
        if ( step < toTerms.size() ) {
            NTerm = toTerms[ step ];
            if ( true == NTerm->isTerminal ) return false;
            for(int i=step+1;i<toTerms.size();i++)
                after.push_back( toTerms[i] );
            return true;
        }

        return false;
    }
    void calcClosureOne() {
        printf(" Once CaclClosure Begin\n");
        for(auto ptrSEItem : collection) {
            StateExtItem& SEItem = *ptrSEItem;
            Term* NTerminal = NULL;
            vector<Term*> vecNextTerms;

            printf("  ");ptrSEItem->print(true);
            if ( splitStateExtItem( SEItem, NTerminal, vecNextTerms ) ) {
                printf("    ");printf(" ok\n");
                printf("    ");SEItem.print(false); NTerminal->print(true);
                vecNextTerms.push_back( SEItem.next );

                printf("     ");for(auto ptrTerm : vecNextTerms) ptrTerm->print(false);
                printf("\n");

                set<Term*> firstSet = calcFirstSet(vecNextTerms);
                printf("     firstSet(%d)\n", firstSet.size());
                for( auto pdtPtr : NTerminal->vecPdtPtrs  ) {
                    for( auto fPtrTerm : firstSet ) {
                        StateExtItem* p = new StateExtItem( pdtPtr->vecSItems[0] , fPtrTerm );
                        collection.insert( p );
                    }
                }
            }
        }
        printf(" Once CalcClosure End\n");
    }
public:
    int sId;
    typedef set<StateExtItem*,StateExtItem> StateCollection;
    StateCollection collection;

    bool calcClosure() {
        ///每次循环增一个
        int curCnt = 0;
        do {
            curCnt = collection.size();
            calcClosureOne();
        }
        while( curCnt < collection.size() );
        return true;
    }
    void print(bool breakLine) {
        printf("StateId(%d) :\n",sId);
        for(auto ptrSEItem : collection ) {
            printf("  ");ptrSEItem->print(true);
        }
    }
    StateSet(int _id = 0):sId(_id) {}
    ~StateSet() {
        ///把 collection 中的 SEItem 全 delete 掉
    }
    bool compareCollection(const StateCollection& a, const StateCollection& b) const {
        auto iter = a.begin(), jter = b.begin();
        while( iter!=a.end() && jter != b.end() ) {
            if ( NULL == *iter ) return true;
            if ( NULL == *jter ) return false;
            StateExtItem &A = **iter, &B = **jter;
            if ( A != B ) return A < B;
            iter++, jter++;
        }
        if ( jter != b.end() ) return true;
        if ( iter != a.end() ) return false;
        return false;
    }
    bool operator()(const StateSet* a, const StateSet* b) const {
        ///像比较字符串一样，按顺序比较两个set的每一个 StateExtItem 中的 StateItem* ptrSItem;
        ///缺就当 NULL
        return compareCollection(a->collection,b->collection);
    }


};

class Action {
public:
    enum Type {Err = 0,Acc, Step, Goto, Recur};
    Type type;
    int toId;
    Action(Type _type, int _id): type(_type), toId(_id) {}
    void print(bool breakLine = true) {
        switch(type) {
            case Err: printf("Err"); break;
            case Acc: printf("Acc"); break;
            case Step: printf("S%d",toId); break;
            case Goto: printf("G%d",toId); break;
            case Recur: printf("R%d",toId); break;
        }
        breakLine && printf("\n");
    }
};

class ActionGotoTable {
public:


    typedef D2Map<int,Term*,int> TypeStateTable;
    typedef vector<StateSet*> TypeVectorStates;
    TypeStateTable* ptrData;
    TypeVectorStates* ptrVec;
    Production* origin;
    D2Map<int,Term*,Action> table;

    ActionGotoTable(TypeStateTable* _ptr = NULL, TypeVectorStates* _ptrVec = NULL, Production* _ptrPdt = NULL): origin(_ptrPdt), ptrData(_ptr), ptrVec(_ptrVec) {
        printf("build\n");
    }
    ~ActionGotoTable() {}
    bool build() {
        if ( NULL == ptrData || NULL == ptrVec || NULL == origin ) return false;
        printf("Action Goto Table\n");
        printf("  Step, Goto\n");
        TypeStateTable& mpStateTable = *ptrData;
        for(auto row : mpStateTable) {
            int stateFromId = row.first;
            for(auto col : row.second) {
                ///C0000005 异常，待查
                Term* ptrStepTerm = col.first;
                int stateToId = col.second;
                //ptrStepTerm->print(true);
                if ( true == ptrStepTerm->isTerminal ) {
                    ///Step 移进
                    table.add( stateFromId, ptrStepTerm, Action(Action::Type::Step, stateToId) );
                }
                else {
                    table.add( stateFromId, ptrStepTerm, Action(Action::Type::Goto, stateToId) );
                }
            }
        }
        ///还有一个Recur
        printf("  Recur\n");
        TypeVectorStates& vecStates = *ptrVec;
        for(int fromId = 0;fromId < vecStates.size();fromId++) {
            for( auto ptrSEItem : vecStates[fromId]->collection ) {
                if ( false == ptrSEItem->hasNextSItem() )
                    table.add( fromId, ptrSEItem->next, Action(Action::Type::Recur, ptrSEItem->ptrItem->ptrPdt->pId) );
            }
        }
        ///还有最后一个ACC
        printf("  Acc\n");
        vector<int> vecAcc;
        StateExtItem* target = new StateExtItem( origin->vecSItems[ origin->toTerms.size() ], endTermPtr );
        for(int vId = 0; vId < vecStates.size(); vId++) {
            StateSet::StateCollection& collection = vecStates[vId]->collection;
            if ( collection.find( target ) != collection.end() ) {
                vecAcc.push_back( vId );
            }
        }
        for(auto accId : vecAcc) {
            table.add( accId, endTermPtr, Action(Action::Type::Acc, -1) );
        }
        delete target;
        return true;
    }


};

#endif // TOKEN_H_INCLUDED
