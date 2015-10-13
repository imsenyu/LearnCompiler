#ifndef ACTION_H_INCLUDED
#define ACTION_H_INCLUDED

#include "stdafx.h"
#include "term.h"
#include "stateSet.h"
#include "clUtils.h"
#include "production.h"

class Term;
class StateSet;
class Production;

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
    void print(bool breakLine = true) const;
    string getString() const;
};

class ActionGotoTable {
public:
    typedef D2Map<int,Term*,int> TypeStateTable;
    typedef vector<StateSet*> TypeVectorStates;
    TypeStateTable* ptrData;
    TypeVectorStates* ptrVec;
    Production* origin;
    D2Map<int,Term*,Action> table;

    ActionGotoTable(
        TypeStateTable* _ptr = NULL,
        TypeVectorStates* _ptrVec = NULL,
        Production* _ptrPdt = NULL
    ) : ptrData(_ptr), ptrVec(_ptrVec), origin(_ptrPdt) {}
    ~ActionGotoTable() {}
    bool build();
    void displayConflict(const int fromId, Term* ptrNextTerm, const Action& act );
};

#endif // ACTION_H_INCLUDED
