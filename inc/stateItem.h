#ifndef STATEITEM_H_INCLUDED
#define STATEITEM_H_INCLUDED

#include "stdafx.h"
#include "term.h"
#include "production.h"

class Term;
class Production;
class StateItem;
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
    bool hasNextSItem() const;
    Term* getNextTerm() const;
    Term* getFromTerm() const;
    StateItem* getNextSItem() const;
    void print(bool breakLine = true) const;
};

#endif // STATEITEM_H_INCLUDED
