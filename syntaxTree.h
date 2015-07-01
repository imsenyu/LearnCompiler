#ifndef SYNTAXTREE_H_INCLUDED
#define SYNTAXTREE_H_INCLUDED

///语法树定义，节点构造等
#include <deque>
#include "token.h"

using namespace std;

class syntaxNode;

class syntaxNode {
public:
    Token* ptrToken; ///叶子节点的对应词法数据
    Production* ptrPdt;
    deque<syntaxNode*> child;
    syntaxNode(Token* _ptr, Production* _pdt = NULL) : ptrToken(_ptr), ptrPdt(_pdt) {}
    void print(bool breakLine = true, int dep = 0) {

        for(int i=0;i<dep;i++)
            printf(" ");
        printf("Addr:%x", this);
        if( 0 == child.size() )
            ptrToken->print(false);
        else
            ptrPdt->ptrTerm->print(false);
        printf("\n");
        for(auto ptrSNode : child) {
            for(int i=0;i<dep+1;i++)
                printf(" ");
            ptrSNode->print(false, dep+1);
        }
        if( 0 == child.size())printf("\n");
    }
};


#endif // SYNTAXTREE_H_INCLUDED
