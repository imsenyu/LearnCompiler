/**
 * @fileoverview syntax
 * @author 郁森<senyu@mail.dlut.edu.cn>/一杉<yusen.ys@alibaba-inc.com>
 * @language C++
 * @description Compiler-基于LL(0)的简易语法分析器实现
 */
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <map>

using namespace std;

enum P_Token {
    ENDofRULE=0,
    INT=1,IF,THEN,ELSE,WHILE,DO,ID,NUM,opPlus,opMinus,opProduct,opDivide,opAnd,opOr,EQUALs,lBRACE,rBRACE,SEMICOLON,lBRACKET,rBRACKET,ASSIGN,
    SEPERATOR,
    Program=1001, Declaration, Statement, BoolExp, CalcExp, ComplexExp,
};

struct P_Rule {
    P_Token from;
    P_Token to[100];
};

P_Rule m_P_Rule[] = {
    /*<程序>→{<声明语句><语句>}*/
    {Program, {lBRACE,Declaration,Statement,rBRACE,ENDofRULE} },
    /*<声明语句>→<声明语句>int ID; | int ID;*/
    {Declaration, {Declaration,INT,ID,SEMICOLON,ENDofRULE} },
    {Declaration, {INT,ID,SEMICOLON,ENDofRULE} },
    /*<语句>→if (<布尔表达式>) then <语句>else <语句> | if (<布尔表达式>) then <语句> | while (<布尔表达式>) do <语句> | ID=<算术表达式> | {<复合语句> }*/
    {Statement, {IF,lBRACKET,BoolExp,rBRACKET,THEN,Statement,ELSE,Statement,ENDofRULE} },
    {Statement, {IF,lBRACKET,BoolExp,rBRACKET,THEN,Statement,ENDofRULE} },
    {Statement, {WHILE,lBRACKET,BoolExp,rBRACKET,DO,Statement,ENDofRULE} },
    {Statement, {ID,ASSIGN,CalcExp,ENDofRULE} },
    {Statement, {lBRACE,ComplexExp,rBRACE,ENDofRULE} },
    /*<复合语句>→<语句>;<复合语句> | <语句>*/
    {ComplexExp, {Statement,SEMICOLON,ComplexExp,ENDofRULE} },
    {ComplexExp, {Statement,ENDofRULE} },
    /*<布尔表达式>→<布尔表达式>and <布尔表达式> | <布尔表达式> or <布尔表达式> | ID relop ID | ID*/
    {BoolExp, {BoolExp,opAnd,BoolExp,ENDofRULE} },
    {BoolExp, {BoolExp,opOr,BoolExp,ENDofRULE} },
    {BoolExp, {ID,EQUALs,ID,ENDofRULE} },
    {BoolExp, {ID,ENDofRULE} },
    /*<算术表达式>→<算术表达式>+ <算术表达式> | <算术表达式> - <算术表达式> | <算术表达式> * <算术表达式> | <算术表达式> / <算术表达式> | (<算术表达式>) | ID | NUM*/
    {CalcExp, {CalcExp,opPlus,CalcExp,ENDofRULE} },
    {CalcExp, {CalcExp,opMinus,CalcExp,ENDofRULE} },
    {CalcExp, {CalcExp,opProduct,CalcExp,ENDofRULE} },
    {CalcExp, {CalcExp,opDivide,CalcExp,ENDofRULE} },
    {CalcExp, {lBRACKET,CalcExp,rBRACKET,ENDofRULE} },
    {CalcExp, {ID,ENDofRULE} },
    {CalcExp, {NUM,ENDofRULE} }
};


int main()
{

    return 0;
}
