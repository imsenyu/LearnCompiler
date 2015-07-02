#ifndef SYNTAXTREE_H_INCLUDED
#define SYNTAXTREE_H_INCLUDED

#include "stdafx.h"
#include "term.h"
#include "production.h"
#include "clUtils.h"

class Term;
class Token;
class syntaxNode;

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
    void print(bool breakLine = true) const;
};

class syntaxNode {
public:
    Token* ptrToken; ///该语法节点对应的输入词
    Production* ptrPdt; ///非叶子节点对用的规约产生式
    syntaxNode* ptrParent; ///父节点
    deque<syntaxNode*> child; ///孩子节点
    D1Map<string,void*> hashData; ///存储所有属性数据
    syntaxNode(Token* _ptr, Production* _pdt = NULL) : ptrToken(_ptr), ptrPdt(_pdt), ptrParent(NULL) {}
    ~syntaxNode();
    ///获得对应输入词的具体文本
    string getLex() const;
    ///递归输出语法树，dep = 0则表示根节点, hasNext用于分支线的绘制
    void print(bool breakLine = true, int dep = 0, bool hasNext = false) const;
};


#endif // SYNTAXTREE_H_INCLUDED
