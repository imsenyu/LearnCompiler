#ifndef SYNTAXTREE_H_INCLUDED
#define SYNTAXTREE_H_INCLUDED

///语法树定义，节点构造等
#include <deque>
#include "token.h"
#include "clUtils.h"
using namespace std;

class syntaxNode;

class syntaxNode {
public:
    Token* ptrToken; ///叶子节点的对应词法数据
    Production* ptrPdt;
    syntaxNode* ptrParent;
    deque<syntaxNode*> child;
    D1Map<string,void*> hashData;
    syntaxNode(Token* _ptr, Production* _pdt = NULL) : ptrToken(_ptr), ptrPdt(_pdt), ptrParent(NULL) {}
    ~syntaxNode() {
        for(auto ptrSNode : child) {
            for(auto item: hashData  ) {
                delete item.second;
            }
            delete ptrSNode;
        }
    }
    string getLex() {
        if ( NULL == ptrToken ) {
            return "";
        }
        else return ptrToken->lexData;
    }
    void print(bool breakLine = true, int dep = 0, bool hasNext = false) {
        static bool vis[1000];
        if ( 0 == dep ) memset(vis,false,sizeof(vis));
        vis[dep] = true;
        for(int i=0;i<dep-1;i++) {
            if ( vis[i] == true ) printf(" │");
            else printf("  ");
        }
        if ( dep > 0 )
            if ( true == hasNext )
                printf(" ├");
            else
                printf(" └");
        //printf("Addr:%x", this);
        if( 0 == child.size() )
            ptrToken->print(false);
        else
            ptrPdt->ptrTerm->print(false);
        printf("\n");
        int cnt = 0;
        for(auto ptrSNode : child) {
            if ( cnt == child.size()-1 ) {
                vis[dep] = false;
            }
            ptrSNode->print(false, dep+1, cnt != child.size()-1);
            cnt++;
        }
       // if( 0 == child.size())printf("\n");
    }
};


#endif // SYNTAXTREE_H_INCLUDED
