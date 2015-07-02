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

/*
 * class Term
 * description: 语法定义的词
 * data:    词的字符串表示tName，是否终结符isTerminal，由其导出的产生式vector<Production*>
 * method:  print(输出语法词的字符串;可选换行)
 *          constructor(使用istream构造数据)//这个应该被剥离出类实现！
 */
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
/*
 * const class Term
 * description: 常量，空字符，和 结束#字符；发现空字符没有使用过，可以考虑删除
 */
 Term* const nullTermPtr = new Term("",true);
 Term* const endTermPtr = new Term("#",true);

/*
 * class Token
 * description: 词法分析器输出的词
 * data:    对应的语法的词Term，拥有的综合属性lexData
 * method:  print(输出语法的词和综合属性;可选换行)
 */
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
 * class Production
 * description: 语法定义的产生式
 * data:    产生式编号pId, 产生式左部词ptrTerm, 产生式右部词数组toTerms, 由该产生式导出的 项目集数组vecSItems(按照*的位置存放)
 * method:  print(输出该产生式;可选*位置，可选换行)
 */
class Production {
public:
    int pId;
    Term* ptrTerm;
    vector<Term*> toTerms;
    vector<StateItem*> vecSItems;
    Production( int _id, Term* _ptr = NULL ): pId(_id), ptrTerm(_ptr) {}
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

/*
 * class StateItem
 * description: 语法定义的(产生式 + 项目集移进位置)
 * data: 对应产生式ptrPdt, 移进位置pos
 * method:  hasNextSItem(对于该项目,是否存在下一个移进项目StateItem)
 *          getNextTerm(获得下一个移进的词Term)
 *          getFromTerm(获得对应产生式的左部词Term)
 *          getNextSItem(获得下一个移进项目StateItem)
 *          print(输出对应移进位置pos的项目;可选换行)
 */
class StateItem {
public:
    Production* ptrPdt;
    int pos;
    StateItem(Production* _ptr = NULL, int _pos = 0): ptrPdt(_ptr), pos(_pos) {}
    bool hasNextSItem() {
        if ( NULL == ptrPdt ) return false;
        return pos < ptrPdt->toTerms.size();
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

/*
 * class StateExtItem
 * description: 语法定义的(产生式 + 项目集移进位置 + LR1向后看的词Term)
 * data: 对应被扩展的ptrItem 和 向后看的词next
 * method:  operator(*,*)(用于set<*>的排序/判重比较)
 *          operator==(用于判别内容是否相等)
 *          operator!=(用于判别内容是否不等)
 *          operator<(用于比较大小,用于set判重)
 *          hasNextSItem(同上)
 *          getFromTerm(同上)
 *          getNextSItem(同上)
 *          print(输出对应移进位置pos的项目 和 向后看的词Term;可选换行)
 */
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
        return ptrItem->hasNextSItem();
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

/*
 * class StateSet
 * description: LR1项目集的一整个状态
 * data: 状态编号sId, 拥有的扩展项目set集合collection
 * method:  calcClosure(用于计算当前collection的完整闭包)
 *          print(输出当前集合中的扩展项目)
 *          operator(*,*)(用于比较各个状态集合是否完全相同)
 *      PRIVATE
 *          calcFirstSet(对当前Term集合计算First集)
 *          splitStateExtItem(对当前扩展项目进行分割，判定是否可以进行闭包扩展)
 *          calcClosureOne(进行一次闭包扩展)
 *          compareCollection(用于public的operator，比较collection内容是否相同)
 */
class StateSet {

public:
    int sId;
    typedef set<StateExtItem*,StateExtItem> StateCollection;
    StateCollection collection;
    StateSet(int _id = 0):sId(_id) {}
    ~StateSet() {
        ///把 collection 中的 SEItem 全 delete 掉
    }

    bool calcClosure() {
    /*
     * 计算闭包,每次循环都算一次,如果不再增大就跳出
     */
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

    bool operator()(const StateSet* a, const StateSet* b) const {
        return compareCollection(a->collection,b->collection);
    }
private:
    set<Term*> calcFirstSet(vector<Term*>& after) {
    /*
     * 计算First集
     * 定义了一个函数闭包dfs,用来进行深度优先搜索
     *   对于搜索的每个vector<Term*>数组，按次序遍历Term
     *     若是终结符就加入First集，跳出
     *     若是非终结符就往其所有产生式进一步 递归
     */
        set<Term*> firstSet;

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
    /*
     * 判断当前 扩展项目能否根据闭包规则扩展
     * 如果可移进，并且移进项是非终结符，则可以，并且引用修改NTerm和after数据
     */
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
    /*
     * 单次闭包扩展
     * 对于collection中的每一个扩展项目
     *   判定其是否满足闭包移进规则
     *     若是，求First集，并生成新的扩展项目
     */
        //printf(" Once CaclClosure Begin\n");
        for(auto ptrSEItem : collection) {
            StateExtItem& SEItem = *ptrSEItem;
            Term* NTerminal = NULL;
            vector<Term*> vecNextTerms;

            //printf("  ");ptrSEItem->print(true);
            if ( splitStateExtItem( SEItem, NTerminal, vecNextTerms ) ) {
                //printf("    ");printf(" ok\n");
                //printf("    ");SEItem.print(false); NTerminal->print(true);
                vecNextTerms.push_back( SEItem.next );

                //printf("     ");for(auto ptrTerm : vecNextTerms) ptrTerm->print(false);
                //printf("\n");

                set<Term*> firstSet = calcFirstSet(vecNextTerms);
                //printf("     firstSet(%d)\n", firstSet.size());
                for( auto pdtPtr : NTerminal->vecPdtPtrs  ) {
                    for( auto fPtrTerm : firstSet ) {
                        StateExtItem* p = new StateExtItem( pdtPtr->vecSItems[0] , fPtrTerm );
                        collection.insert( p );
                    }
                }
            }
        }
        //printf(" Once CalcClosure End\n");
    }
    bool compareCollection(const StateCollection& a, const StateCollection& b) const {
    /*
     * 比较两个collection, 返回 operator< 的运算结果
     * 分别比较两个collection的同一个顺序下的元素
     *   只要有一个不同就返回 不同元素的operator<运算结果
     */
        if ( a.size() != b.size() ) return a.size() < b.size();
        auto iter = a.begin(), jter = b.begin();
        while( iter!=a.end() && jter != b.end() ) {
            if ( NULL == *iter ) return true;
            if ( NULL == *jter ) return false;
            StateExtItem &A = **iter, &B = **jter;
            if ( A != B ) return A < B;
            iter++, jter++;
        }
        return false;
    }
};

/*
 * class Action
 * description: ActionGoto表的子类型
 * data: 执行动作类型type, 动作参数toId
 * method:  print(输出具体动作状态)
 */
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
                Term* ptrStepTerm = col.first;
                int stateToId = col.second;
                if ( true == ptrStepTerm->isTerminal ) {
                    ///Step 移进
                    bool ret = table.add( stateFromId, ptrStepTerm, Action(Action::Type::Step, stateToId) );
                    if ( false == ret ) {
                        printf("Conflict\n");
                        table.get(stateFromId,ptrStepTerm)->print(true);
                        printf("Step, %d\n",stateToId);

                    }
                }
                else {
                    bool ret = table.add( stateFromId, ptrStepTerm, Action(Action::Type::Goto, stateToId) );
                    if ( false == ret ) {
                        printf("Conflict\n");
                        table.get(stateFromId,ptrStepTerm)->print(true);
                        printf("Goto, %d\n",stateToId);

                    }
                }
            }
        }
        ///还有一个Recur
        printf("  Recur\n");
        TypeVectorStates& vecStates = *ptrVec;
        for(int fromId = 0;fromId < vecStates.size();fromId++) {
            for( auto ptrSEItem : vecStates[fromId]->collection ) {
                if ( false == ptrSEItem->hasNextSItem() ) {
                    bool ret = table.add( fromId, ptrSEItem->next, Action(Action::Type::Recur, ptrSEItem->ptrItem->ptrPdt->pId) );
                    if ( false == ret ) {
                        printf("Conflict\n");
                        table.get(fromId,ptrSEItem->next)->print(true);
                        printf("Recur, %d\n",ptrSEItem->ptrItem->ptrPdt->pId);

                    }
                }
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
            bool ret = table.add( accId, endTermPtr, Action(Action::Type::Acc, -1), true );
            ///可覆盖
            if ( false == ret ) {
                printf("Conflict\n");
                table.get(accId, endTermPtr)->print(true);
                printf("Acc, %d\n",-1);

            }
        }
        delete target;
        return true;
    }


};

#endif // TOKEN_H_INCLUDED
