#ifndef TERM_H_INCLUDED
#define TERM_H_INCLUDED

#include "stdafx.h"
#include "production.h"

class Term;
class Production;

/*
 * class Term
 * description: 语法定义的词
 * data:    词的字符串表示tName，是否终结符isTerminal，由其导出的产生式vector<Production*>
 * method:  print(输出语法词的字符串;可选换行)
 *          constructor(使用istream构造数据)//这个应该被剥离出类实现！
 */
class Term {
public:
    string tName;
    bool isTerminal;
    vector<Production*> vecPdtPtrs;
    Term(string _name = "", bool _terminal = true): tName(_name), isTerminal(_terminal) { }
    Term(istream& in);
    void print(bool breakLine = true) const;
    string getString() const;
};
/*
 * const class Term
 * description: 常量，空字符，和 结束#字符；发现空字符没有使用过，可以考虑删除
 */
extern Term* const nullTermPtr;// = new Term("",true);
extern Term* const endTermPtr;// = new Term("#",true);


#endif // TERM_H_INCLUDED
