#ifndef STATEEXTITEM_H_INCLUDED
#define STATEEXTITEM_H_INCLUDED

#include "stdafx.h"
#include "stateItem.h"
#include "term.h"

class Term;
class StateItem;

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
    bool operator()(const StateExtItem* a, const StateExtItem* b) const;
    bool operator==(const StateExtItem& b) const;
    bool operator!=(const StateExtItem& b) const;
    bool operator<(const StateExtItem& b) const;
    bool hasNextSItem() const;
    Term* getFromTerm() const;
    StateItem* getNextSItem() const;
    void print(bool breakLine = true) const;
};

#endif // STATEEXTITEM_H_INCLUDED
