#ifndef TOKEN_H_INCLUDED
#define TOKEN_H_INCLUDED


///词法数据类型，再加一个 tokenConfig.h 自定义语言词法
#include <iostream>
#include <cstdio>
#include <vector>
#include <string>
#include <set>
#include <functional>

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
    Term(string _name, bool _terminal = true): tName(_name), isTerminal(_terminal) {}
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
    Term* ptrTerm;
    bool isTerminal;
    vector<Term*> toTerms;
    vector<StateItem*> vecSItems;
    Production( Term* _ptr = NULL , bool _terminal = false): ptrTerm(_ptr), isTerminal(_terminal) {}
    void print(int pos = -1, bool breakLine = true) {
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
    StateItem* getNext() {
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
        if ( a->ptrItem == b->ptrItem ) return a->next < b->next;
        else return a->ptrItem < b->ptrItem;
    }
    bool hasSItemNext() {
        if ( NULL == ptrItem ) return false;
        return ptrItem->hasNext();
    }
    StateItem* getSItemNext() {
        if ( NULL == ptrItem ) return NULL;
        return ptrItem->getNext();
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
        return true;
    }
    bool operator()(const StateSet* a, const StateSet* b) const {
        ///像比较字符串一样，按顺序比较两个set的每一个 StateExtItem 中的 StateItem* ptrSItem;
        ///缺就当 NULL
        return compareCollection(a->collection,b->collection);
    }


};



#endif // TOKEN_H_INCLUDED
