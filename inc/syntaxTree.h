#ifndef SYNTAXTREE_H_INCLUDED
#define SYNTAXTREE_H_INCLUDED

#include "stdafx.h"
#include "term.h"
#include "production.h"
#include "clUtils.h"
#include "json/json.h"

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

/*
 * class SyntaxNode
 * description: 语法树节点
 * data:    ptrToken 该语法节点对应的输入词
 *          ptrPdt 非叶子节点对用的规约产生式,叶子节点为NULL
 *          ptrParent 父语法节点
 *          child 孩子语法节点, 双向队列
 *          hashData 存储所有属性数据( 通过重载的运算符和clUtils中的PASS和GET 可以简便地操作数据对象 )
 *
 * method:  ~syntaxNode delete时删除属性数据
 *          getLex 返回获取的输入词的字符串
 *          print 输出,可选换行,dep = 0则表示根节点, hasNext用于分支线的绘制
 *          operator[ int ] 获取孩子节点的引用, 不存在直接throw
 *          operator[ string ] 获取对应string的属性数据, 不存在会创建一个对NULL的引用，用于修改
 */
class syntaxNode {
public:
    int tId;
    Token* ptrToken;
    Production* ptrPdt;
    syntaxNode* ptrParent;
    deque<syntaxNode*> child;
    Json::Value data;
    syntaxNode(Token* _ptr, Production* _pdt = NULL) : ptrToken(_ptr), ptrPdt(_pdt), ptrParent(NULL) {}
    ~syntaxNode();
    string getLex() const;
    void print(bool breakLine = true, int dep = 0, bool hasNext = false) const;
    syntaxNode& operator[](const int pos);
    Json::Value& operator[](const string& key) {
        return data[key];
    }
    const Json::Value& operator[](const string& key) const {
        return data[key];
    }

};

#endif // SYNTAXTREE_H_INCLUDED
