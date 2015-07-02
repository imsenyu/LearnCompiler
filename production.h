#ifndef PRODUCTION_H_INCLUDED
#define PRODUCTION_H_INCLUDED

#include "stdafx.h"
#include "term.h"
#include "stateItem.h"

class Term;
class Production;
class StateItem;
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
    void print(int pos = -1, bool breakLine = true) const;
};

#endif // PRODUCTION_H_INCLUDED
