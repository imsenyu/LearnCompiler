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
#include "strtool.h"

using namespace std;


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
                m_Rule(string & , map<string,Term*> , string);
                void showTerm();
        };
        struct m_Rule_Ptr {
            m_Rule *rule; //具体规则指针
            int itemID; //LR项目 编号
            int point;  //小圆点位置，从0开始
        };
    public:
        LR_Syntax():ruleDelim("|") {}
        LR_Syntax& initSymbol( vector<LR_Syntax::Term>& );
        LR_Syntax& initProduction( vector<LR_Syntax::Rule>& ); //vector 导入 map
        LR_Syntax& showProduction();
    private:
        string ruleDelim;
        map<string,Term*> symbolTable;
        map<Term*, vector<m_Rule*>* > productionTable;
        map<int, m_Rule_Ptr*> itemTable;   //LR项目表
};

LR_Syntax::m_Rule::m_Rule(string& s, map<string,Term*> m, string delim) {
    vector<string> vt;
    strtool::split(s, vt, delim);
    for(vector<string>::iterator itr=vt.begin(); itr!=vt.end(); itr++) {
        if ( m.find(*itr + "_t") != m.end() ) {
            term.push_back( m.find(*itr + "_t")->second );
        } else if ( m.find(*itr + "_n") != m.end() ) {
            term.push_back( m.find(*itr + "_n")->second );
        } else
            cout<<"ERROR...Can't Find RightTerm["<<*itr<<"] in SymbolTable"<<endl;
    }
    cout<<"m_Rule:"<<s<<ends;
    for(vector<Term*>::iterator itr=term.begin(); itr!=term.end(); itr++) {
        Term &p = **itr;
        cout<<" "<<p.name<<" "<<p.terminal<<endl;
    }
}

void LR_Syntax::m_Rule::showTerm() {
    vector<Term*>::iterator itr;
    for(itr=term.begin();itr!=term.end();itr++) {
        Term &p = **itr;
        cout<<p.name<<p.terminal<<"\t";
    }
}

LR_Syntax& LR_Syntax::initSymbol( vector<LR_Syntax::Term>& t) {
    for(vector<LR_Syntax::Term>::iterator itr = t.begin(); itr!=t.end(); itr++) {
        Term &p = *itr;
        symbolTable.insert( pair<string,Term*>( p.name + "_" + (p.terminal?"t":"n"), &p ) );
    }
    return *this;
}
LR_Syntax& LR_Syntax::initProduction( vector<LR_Syntax::Rule>& r) {
    cout<<"Production"<<endl;
    for(vector<LR_Syntax::Rule>::iterator itr=r.begin(); itr!=r.end(); itr++) {
        Rule &p = *itr;
        m_Rule *address = new m_Rule(p.value, symbolTable, ruleDelim);
        if ( symbolTable.find( p.source + "_n" ) != symbolTable.end() ) {
            Term* sym = symbolTable.find( p.source + "_n" )->second;
            cout<<" From Symbol["<<sym->name<<"] "<<sym->terminal<<endl;

            if ( productionTable.find(sym) == productionTable.end() ) {
                vector<m_Rule*>* container = new vector<m_Rule*>;
                container->push_back( address );
                productionTable.insert( pair<Term*,vector<m_Rule*>* >( sym, container ) );
            }
            else {
                productionTable.find(sym)->second->push_back( address );
            }
        } else
            cout<<"ERROR...Can't Find LeftTerm["<<p.source<<"] in SymbolTable"<<endl;
    }

    return *this;
}

LR_Syntax& LR_Syntax::showProduction() {
    map<Term*, vector<m_Rule*>* >::iterator itr;
    for(itr=productionTable.begin();itr!=productionTable.end();itr++) {
        Term &x = *itr->first;
        vector<m_Rule*> &y = *itr->second;
        cout<<"Term:"<<x.name<<endl;

        vector<m_Rule*>::iterator jtr;
        int cnt = 1;
        for(jtr=y.begin();jtr!=y.end();jtr++) {
            m_Rule& r = **jtr;
            cout<<"  "<<cnt++<<"  ";
            r.showTerm();
            cout<<endl;
        }
    }
}

LR_Syntax::Rule m_Syntax_Rule[] = {
    {"_P","P"},
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
    {"_P",false},{"P",false},{"D",false},{"S",false},{"BoolExp",false},{"CalcExp",false},{"CpxS",false},{"relop",false}
};
vector<LR_Syntax::Rule> v_Syntax_Rule( m_Syntax_Rule, m_Syntax_Rule+sizeof(m_Syntax_Rule)/sizeof(m_Syntax_Rule[0]) );
vector<LR_Syntax::Term> v_Syntax_VN( m_Syntax_VN, m_Syntax_VN+sizeof(m_Syntax_VN)/sizeof(m_Syntax_VN[0]) );
vector<LR_Syntax::Term> v_Syntax_VT( m_Syntax_VT, m_Syntax_VT+sizeof(m_Syntax_VT)/sizeof(m_Syntax_VT[0]) );

int main() {
    LR_Syntax lr;
    lr.
      initSymbol(v_Syntax_VN).
      initSymbol(v_Syntax_VT).
      initProduction(v_Syntax_Rule).
      showProduction();
    return 0;
}
