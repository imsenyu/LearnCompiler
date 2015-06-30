#ifndef SYNTAXTREE_H_INCLUDED
#define SYNTAXTREE_H_INCLUDED

///语法树定义，节点构造等
#include "token.h"

class syntaxNode;

class syntaxNode {
public:
    Production* production; ///可以是产生式也可以是终结，如果终结的话就是leaf叶子节点了
    int order; ///是父节点的 第几个孩子
    Token* ptrToken; ///叶子节点的对应词法数据
    vector<syntaxNode*> child;
    syntaxNode(Production* _ptr, int _order):production(_ptr), order(_order) {}
};


#endif // SYNTAXTREE_H_INCLUDED
