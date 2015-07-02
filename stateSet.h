#ifndef STATESET_H_INCLUDED
#define STATESET_H_INCLUDED

#include "stdafx.h"
#include "stateExtItem.h"
#include "term.h"

class Term;
class StateExtItem;
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
    ~StateSet();
    bool calcClosure();
    void print(bool breakLine = true) const;
    bool operator()(const StateSet* a, const StateSet* b) const;
private:
    set<Term*> calcFirstSet(vector<Term*>& after);
    bool splitStateExtItem( StateExtItem& SEItem, Term* &NTerm, vector<Term*>& after );
    void calcClosureOne();
    bool compareCollection(const StateCollection& a, const StateCollection& b) const;
};

#endif // STATESET_H_INCLUDED
