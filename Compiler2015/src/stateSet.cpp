#include "stateSet.h"

StateSet::~StateSet() {
    ///把 collection 中的 SEItem 全 delete 掉
    for(auto ptrSEItem : collection) {
        delete ptrSEItem;
    }
}

bool StateSet::calcClosure() {
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

void StateSet::print(bool breakLine) const {
    cout<<getString();
    breakLine && printf("\n");
}

string StateSet::getString() const {
    string ret;
    ret += "StateId("+ clUtils::itoa(sId) +") :\n";
    for(auto ptrSEItem : collection ) {
        ret += "  "+ ptrSEItem->getString() + "\n";
    }
    return ret;
}

bool StateSet::operator()(const StateSet* a, const StateSet* b) const {
    return compareCollection(a->collection,b->collection);
}

set<Term*> StateSet::calcFirstSet(vector<Term*>& after) {
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
                for(auto ptrPdt : curTerm.vecPdtPtrs) {
                    ret |= dfs( ptrPdt->toTerms );
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
bool StateSet::splitStateExtItem( StateExtItem& SEItem, Term* &NTerm, vector<Term*>& after ) {
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
void StateSet::calcClosureOne() {
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
bool StateSet::compareCollection(const StateCollection& a, const StateCollection& b) const {
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
