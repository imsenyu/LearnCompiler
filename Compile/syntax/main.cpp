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

/*
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
    /*<程序>→{<声明语句><语句>}
    {Program, {lBRACE,Declaration,Statement,rBRACE,ENDofRULE} },
    /*<声明语句>→<声明语句>int ID; | int ID;
    {Declaration, {Declaration,INT,ID,SEMICOLON,ENDofRULE} },
    {Declaration, {INT,ID,SEMICOLON,ENDofRULE} },
    /*<语句>→if (<布尔表达式>) then <语句>else <语句> | if (<布尔表达式>) then <语句> | while (<布尔表达式>) do <语句> | ID=<算术表达式> | {<复合语句> }
    {Statement, {IF,lBRACKET,BoolExp,rBRACKET,THEN,Statement,ELSE,Statement,ENDofRULE} },
    {Statement, {IF,lBRACKET,BoolExp,rBRACKET,THEN,Statement,ENDofRULE} },
    {Statement, {WHILE,lBRACKET,BoolExp,rBRACKET,DO,Statement,ENDofRULE} },
    {Statement, {ID,ASSIGN,CalcExp,ENDofRULE} },
    {Statement, {lBRACE,ComplexExp,rBRACE,ENDofRULE} },
    /*<复合语句>→<语句>;<复合语句> | <语句>
    {ComplexExp, {Statement,SEMICOLON,ComplexExp,ENDofRULE} },
    {ComplexExp, {Statement,ENDofRULE} },
    /*<布尔表达式>→<布尔表达式>and <布尔表达式> | <布尔表达式> or <布尔表达式> | ID relop ID | ID
    {BoolExp, {BoolExp,opAnd,BoolExp,ENDofRULE} },
    {BoolExp, {BoolExp,opOr,BoolExp,ENDofRULE} },
    {BoolExp, {ID,EQUALs,ID,ENDofRULE} },
    {BoolExp, {ID,ENDofRULE} },
    /*<算术表达式>→<算术表达式>+ <算术表达式> | <算术表达式> - <算术表达式> | <算术表达式> * <算术表达式> | <算术表达式> / <算术表达式> | (<算术表达式>) | ID | NUM
    {CalcExp, {CalcExp,opPlus,CalcExp,ENDofRULE} },
    {CalcExp, {CalcExp,opMinus,CalcExp,ENDofRULE} },
    {CalcExp, {CalcExp,opProduct,CalcExp,ENDofRULE} },
    {CalcExp, {CalcExp,opDivide,CalcExp,ENDofRULE} },
    {CalcExp, {lBRACKET,CalcExp,rBRACKET,ENDofRULE} },
    {CalcExp, {ID,ENDofRULE} },
    {CalcExp, {NUM,ENDofRULE} }
};

class Rule {
public:
    Rule(int t) {
        cout<<"1"<<endl;
    }
    Rule (const char *t) {
    }
};*/
///初始化delim[|]，generator[ ]，仅限于单字符，目前仅支持 单个非终结符左部,加入终止符[#],终结符初始化列表，非终结符初始化列表，不在列表报错



class LR_Syntax {
public:
    struct Rule {
        string source;
        string value;
    };
    struct Term {
        string name;
        bool terminal;
    };
private:
    struct Production {
        int from;
    };
    class m_Rule {
    public:
        vector<Term*> term;
        int step;
        m_Rule(string& s, map<string,Term*> m, string delim):step(0) {
            vector<string> vt;
            cout<<"split ["<<s<<"]: "<<split(s, vt, delim)<<endl;
            for(vector<string>::iterator itr=vt.begin();itr!=vt.end();itr++) {
                cout<<" vt/n"<<*itr<<endl;
                if ( m.find(*itr + "_t") != m.end() ) {
                    term.push_back( m.find(*itr + "_t")->second );
                }
                else if ( m.find(*itr + "_n") != m.end() ) {
                    term.push_back( m.find(*itr + "_n")->second );
                }
                else
                    cout<<"ERROR....1"<<endl;
            }
            cout<<"m_Rule:"<<s<<endl;
            for(vector<Term*>::iterator itr=term.begin();itr!=term.end();itr++) {
                Term &p = **itr;
                cout<<" "<<p.name<<" "<<p.terminal<<endl;
            }
            /* TODO
             从Term映射中 把每个拆开来，构成term数组
             */
        }
    private:
        int split(const string& str, vector<string>& ret_, string sep = "|")
        {
            if (str.empty())
            {
                return 0;
            }

            string tmp;
            string::size_type pos_begin = str.find_first_not_of(sep);
            string::size_type comma_pos = 0;

            while (pos_begin != string::npos)
            {
                comma_pos = str.find(sep, pos_begin);
                if (comma_pos != string::npos)
                {
                    tmp = str.substr(pos_begin, comma_pos - pos_begin);
                    pos_begin = comma_pos + sep.length();
                }
                else
                {
                    tmp = str.substr(pos_begin);
                    pos_begin = comma_pos;
                }

                if (!tmp.empty())
                {
                    ret_.push_back(tmp);
                    tmp.clear();
                }
            }
            return ret_.size();
        }
    };
public:
    LR_Syntax():ruleDelim("|") {}
    LR_Syntax& initSymbol( vector<LR_Syntax::Term>& );
    LR_Syntax& initProduction( vector<LR_Syntax::Rule>& ); //vector 导入 map
private:
    string ruleDelim;
    map<string,Term*> symbolTable;
    map<Term*,m_Rule*> productionTable;
};
LR_Syntax& LR_Syntax::initSymbol( vector<LR_Syntax::Term>& t) {
    for(vector<LR_Syntax::Term>::iterator itr = t.begin(); itr!=t.end(); itr++) {
        Term &p = *itr;
        symbolTable.insert( pair<string,Term*>( p.name + "_" + (p.terminal?"t":"n"), &p ) );
    }
    return *this;
}
LR_Syntax& LR_Syntax::initProduction( vector<LR_Syntax::Rule>& r) {
    cout<<"Production"<<endl;
    for(vector<LR_Syntax::Rule>::iterator itr=r.begin();itr!=r.end();itr++) {
        Rule &p = *itr;
        m_Rule *address = new m_Rule(p.value, symbolTable, ruleDelim);
        if ( symbolTable.find( p.source + "_n" ) != symbolTable.end() ) {
            Term* sym = symbolTable.find( p.source + "_n" )->second;
            cout<<" from "<<sym->name<<" "<<sym->terminal<<" -> "<<endl;
            productionTable.insert( pair<Term*,m_Rule*>( sym,address ) );
        }
        else
            cout<<"ERROR....2"<<endl;

    }

    return *this;
}

LR_Syntax::Rule m_Syntax_Rule[] = {
    {"P","{|D|S|}"},
    {"D","D|int|ID|;"},{"D","int|ID|;"},
    {"S","if|(|BoolExp|)|then|S|else|S"},{"S","if|(|BoolExp|)|then|S"},{"S","while|(|BoolExp|)|do|S"},{"S","ID|=|CalcExp"},{"S","{|CpxS|}"},
    {"CpxS","S|;|CpxS"},{"CpxS","S"},
    {"BoolExp","BoolExp|and|BoolExp"},{"BoolExp","BoolExp|or|BoolExp"},{"BoolExp","ID|relop|ID"},{"BoolExp","ID"},
    {"CalcExp","CalcExp|+|CalcExp"},{"CalcExp","CalcExp|-|CalcExp"},{"CalcExp","CalcExp|*|CalcExp"},{"CalcExp","CalcExp|/|CalcExp"},{"CalcExp","(|CalcExp|)"},{"CalcExp","ID"},{"CalcExp","NUM"},
    {"relop","<"},{"relop",">"},{"relop","<="},{"relop",">="},{"relop","!="},{"relop","=="}
};

LR_Syntax::Term m_Syntax_VN[] = {
    {"int",true},{"if",true},{"then",true},{"else",true},{"while",true},{"do",true},
    {"ID",true},{"NUM",true},
    {"+",true},{"-",true},{"*",true},{"/",true},{"and",true},{"or",true},
    {"<",true},{">",true},{"<=",true},{">=",true},{"!=",true},{"==",true},
    {"{",true},{"}",true},{";",true},{"(",true},{")",true},{"=",true}
};

LR_Syntax::Term m_Syntax_VT[] = {
    {"P",false},{"D",false},{"S",false},{"BoolExp",false},{"CalcExp",false},{"CpxS",false},{"relop",false}
};
vector<LR_Syntax::Rule> v_Syntax_Rule( m_Syntax_Rule, m_Syntax_Rule+sizeof(m_Syntax_Rule)/sizeof(m_Syntax_Rule[0]) );
vector<LR_Syntax::Term> v_Syntax_VN( m_Syntax_VN, m_Syntax_VN+sizeof(m_Syntax_VN)/sizeof(m_Syntax_VN[0]) );
vector<LR_Syntax::Term> v_Syntax_VT( m_Syntax_VT, m_Syntax_VT+sizeof(m_Syntax_VT)/sizeof(m_Syntax_VT[0]) );

int main()
{
    LR_Syntax lr;
    lr.
      initSymbol(v_Syntax_VN).
      initSymbol(v_Syntax_VT).
      initProduction(v_Syntax_Rule);
    return 0;
}
