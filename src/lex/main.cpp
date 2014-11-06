/**
 * @fileoverview lex
 * @author 郁森<senyu@mail.dlut.edu.cn>
 * @language C++
 * @description Compiler-LEX
 */
#include <cstdio>
#include <iostream>
#include <vector>
#include <map>
#include <string>

using namespace std;


/*
 *
//TODO
1. 由map确定的 读取字符并进入下一个状态的 自动函数
2.   包括 othermap判定
3.   每次读取完成结束状态则 重新开始0状态
4. 把查找到的 标识符 1状态 进行 文本匹配，找出各种关键字
 *
 */

enum finishElement {_ID=1001,_NUM,_OPERATOR};
enum tokenType { NVAL=1, AVAL, RELOP};

struct DFA_Trans {
    int sfrom;
    char ch;
    int sto;
    bool ignore;
};

struct DFA_RestTrans {
    int sfrom;
    int sto;
};

struct DFA_StateId {
    int sta;
    finishElement to;
};

struct DFA_Token {
    finishElement f;
    int id;
    string token;
    tokenType type;
};

DFA_Trans m_DFA_Trans[] = {
    /*空格换行*/
    {0,' ',0,true},{0,'\n',0,true},{0,'\r',0,true},
    /*字母*/
    {0,'a',1},{0,'b',1},{0,'c',1},{0,'d',1},{0,'e',1},{0,'f',1},{0,'g',1},{0,'h',1},{0,'i',1},{0,'j',1},{0,'k',1},{0,'l',1},{0,'m',1},{0,'n',1},{0,'o',1},{0,'p',1},{0,'q',1},{0,'r',1},{0,'s',1},{0,'t',1},{0,'u',1},{0,'v',1},{0,'w',1},{0,'x',1},{0,'y',1},{0,'z',1},
    {0,'A',1},{0,'B',1},{0,'C',1},{0,'D',1},{0,'E',1},{0,'F',1},{0,'G',1},{0,'H',1},{0,'I',1},{0,'J',1},{0,'K',1},{0,'L',1},{0,'M',1},{0,'N',1},{0,'O',1},{0,'P',1},{0,'Q',1},{0,'R',1},{0,'S',1},{0,'T',1},{0,'U',1},{0,'V',1},{0,'W',1},{0,'X',1},{0,'Y',1},{0,'Z',1},
    /*字母数字*/
    {1,'a',1},{1,'b',1},{1,'c',1},{1,'d',1},{1,'e',1},{1,'f',1},{1,'g',1},{1,'h',1},{1,'i',1},{1,'j',1},{1,'k',1},{1,'l',1},{1,'m',1},{1,'n',1},{1,'o',1},{1,'p',1},{1,'q',1},{1,'r',1},{1,'s',1},{1,'t',1},{1,'u',1},{1,'v',1},{1,'w',1},{1,'x',1},{1,'y',1},{1,'z',1},
    {1,'A',1},{1,'B',1},{1,'C',1},{1,'D',1},{1,'E',1},{1,'F',1},{1,'G',1},{1,'H',1},{1,'I',1},{1,'J',1},{1,'K',1},{1,'L',1},{1,'M',1},{1,'N',1},{1,'O',1},{1,'P',1},{1,'Q',1},{1,'R',1},{1,'S',1},{1,'T',1},{1,'U',1},{1,'V',1},{1,'W',1},{1,'X',1},{1,'Y',1},{1,'Z',1},
    {1,'0',1},{1,'1',1},{1,'2',1},{1,'3',1},{1,'4',1},{1,'5',1},{1,'6',1},{1,'7',1},{1,'8',1},{1,'9',1},
    /*数字*/
    {0,'0',3},{0,'1',3},{0,'2',3},{0,'3',3},{0,'4',3},{0,'5',3},{0,'6',3},{0,'7',3},{0,'8',3},{0,'9',3},
    {3,'0',3},{3,'1',3},{3,'2',3},{3,'3',3},{3,'4',3},{3,'5',3},{3,'6',3},{3,'7',3},{3,'8',3},{3,'9',3},
    /*运算符,括号*/
    {0,'+',5},{0,'-',5},{0,'*',5},{0,'/',5},{0,'(',5},{0,')',5},{0,'{',5},{0,'}',5},{0,';',5},
    {0,'<',6},{0,'>',6},{0,'!',6},
    {6,'=',7},{0,'=',9},{9,'=',10}
};
DFA_RestTrans m_DFA_RestTrans[] = {
    {1,2},{3,4},{6,8},{9,11}
};
DFA_StateId m_DFA_FinishState[] = {
    {2,_ID},{4,_NUM},{5,_OPERATOR},{7,_OPERATOR},{8,_OPERATOR},{10,_OPERATOR},{11,_OPERATOR}
};
/*
 * 按照finishElement分组， 再按照NVAL分组， 然后做map

 或者
 把 finishElement & string 做 key把NVAL,RELOP的 全部加入map
 剩下AVAL 加入另一个 vector
 */
DFA_Token m_DFA_Token[] = {
    {_ID, 1, "int", NVAL},
    {_ID, 2, "if", NVAL},
    {_ID, 3, "then", NVAL},
    {_ID, 4, "else", NVAL},
    {_ID, 5, "while", NVAL},
    {_ID, 6, "do", NVAL},
    {_ID, 7, "ID", AVAL},
    {_NUM, 8, "NUM", AVAL},
    {_OPERATOR, 9, "+", NVAL},
    {_OPERATOR, 10, "-", NVAL},
    {_OPERATOR, 11, "*", NVAL},
    {_OPERATOR, 12, "/", NVAL},
    {_ID, 13, "and", NVAL},
    {_ID, 14, "or", NVAL},
    {_OPERATOR, 15, "<", RELOP},
    {_OPERATOR, 15, ">", RELOP},
    {_OPERATOR, 15, "<=", RELOP},
    {_OPERATOR, 15, ">=", RELOP},
    {_OPERATOR, 15, "!=", RELOP},
    {_OPERATOR, 15, "==", RELOP},
    {_OPERATOR, 16, "{", NVAL},
    {_OPERATOR, 17, "}", NVAL},
    {_OPERATOR, 18, ";", NVAL},
    {_OPERATOR, 19, "(", NVAL},
    {_OPERATOR, 20, ")", NVAL},
    {_OPERATOR, 21, "=", NVAL}
};

vector<DFA_Trans>       v_DFA_Trans( m_DFA_Trans, m_DFA_Trans+sizeof(m_DFA_Trans)/sizeof(m_DFA_Trans[0]) );
vector<DFA_RestTrans>   v_DFA_RestTrans( m_DFA_RestTrans, m_DFA_RestTrans+sizeof(m_DFA_RestTrans)/sizeof(m_DFA_RestTrans[0]) );
vector<DFA_StateId>     v_DFA_FinishState( m_DFA_FinishState, m_DFA_FinishState+sizeof(m_DFA_FinishState)/sizeof(m_DFA_FinishState[0]) );
vector<DFA_Token>       v_DFA_Token( m_DFA_Token, m_DFA_Token+sizeof(m_DFA_Token)/sizeof(m_DFA_Token[0]));

class DFA {
public:
    enum charValid {NORMAL=1,ERROR,PREDICATE};
private:
	struct t_DFA_Trans{
		int state;
		charValid valid;
		bool ignore;
	};
	struct t_DFA_Token{
        map<string, DFA_Token> N;
        map<finishElement,DFA_Token> A;
        void clear() { N.clear(); A.clear(); }
    };
public:
    DFA():trans(NULL),maxStates(0),startState(0) {}
    DFA& initTrans(vector<DFA_Trans> &, vector<DFA_RestTrans> &);
    DFA& initStart(int s) {startState = s;}
    DFA& initFinish(vector<DFA_StateId> &);
    DFA& initToken(vector<DFA_Token> &);
    DFA& run();

private:
    t_DFA_Trans (* trans)[256];
    map<int, finishElement> finish;
    t_DFA_Token token;
    int maxStates;
    int startState;
    int currentState;
    template<typename T> int getMaxStateId(T &);
    bool hasFinish(int);
    void matchToken(const string &, finishElement);

};
template<typename T>
int DFA::getMaxStateId(T &m) {
    typename T::iterator itr;
    int ret = -1;
    for(itr = m.begin(); itr!=m.end(); itr++) {
        ret = itr->sfrom > ret ? itr->sfrom : ret;
        ret = itr->sto > ret ? itr->sto : ret;
    }
    return ret;
}
DFA& DFA::initTrans(vector<DFA_Trans> &m, vector<DFA_RestTrans> &r) {
    delete[] trans;

    int maxStates = getMaxStateId< vector<DFA_Trans> >(m);
    trans = new t_DFA_Trans[ maxStates+1 ][256];

    for(int state = 0; state<maxStates; state++) {
        for(int ch = 0; ch<256; ch++) {
            trans[state][ch].valid = ERROR;
        }
    }
    for(vector<DFA_Trans>::iterator itr = m.begin(); itr!=m.end(); itr++) {
        DFA_Trans &p = *itr;
        trans[p.sfrom][p.ch].valid = NORMAL;
        trans[p.sfrom][p.ch].ignore = p.ignore;
        trans[p.sfrom][p.ch].state = p.sto;
    }
    for(vector<DFA_RestTrans>::iterator itr = r.begin(); itr!=r.end(); itr++) {
        DFA_RestTrans &p = *itr;
        for(int ch = 0; ch<256; ch++) {
            if ( trans[p.sfrom][ch].valid == ERROR) {
                trans[p.sfrom][ch].valid = PREDICATE;
                trans[p.sfrom][ch].state = p.sto;
            }
        }
    }
    return *this;
}

DFA& DFA::initFinish(vector<DFA_StateId> &f) {
    finish.clear();
    for( vector<DFA_StateId>::iterator itr = f.begin(); itr!=f.end(); itr++) {
        finish.insert(pair<int,finishElement>(itr->sta,itr->to));
    }
    return *this;
}
bool DFA::hasFinish(int s) {
    return finish.find(s) != finish.end();
    //return finish.upper_bound(s)!=finish.lower_bound(s);
}
DFA& DFA::run() {
    char input;
    bool runnable = true;
    bool inputable = true;
    currentState = startState;
    string buffer = "";

    while(runnable) {
        if (inputable && scanf("%c",&input) == EOF) {
            break;
        }
        inputable = true;
        t_DFA_Trans next = trans[currentState][input];
        //非法字符
        if ( next.valid == ERROR ) {
                runnable = false;
            cout<<"ERROR"<<endl;
        }

        else if ( next.valid == NORMAL ) {
            if ( !next.ignore )buffer += input;
            if ( hasFinish(next.state) ) {
                //cout<<"end"<<next.state;
                matchToken(buffer, finish.find(next.state)->second);
                buffer = "";
                currentState = startState;
            }
            else {
                currentState = next.state;
            }
            //继续扫描
        }
        else if ( next.valid == PREDICATE ) {
            inputable = false;
            if ( hasFinish(next.state) ) {
                //cout<<"end"<<(next.state);
                matchToken(buffer, finish.find(next.state)->second);
                buffer = "";
                currentState = startState;
            }
            else {
                currentState = next.state;
            }
        }

    }
    return *this;
}

DFA& DFA::initToken(vector<DFA_Token> &t) {
    token.clear();
    for(vector<DFA_Token>::iterator itr = t.begin();itr!=t.end();itr++) {
        DFA_Token &p = *itr;
        if ( p.type != AVAL ) {
            token.N.insert( pair<string, DFA_Token>( p.token + "_" + (char)p.f , p ) );
        }
        else {
            token.A.insert( pair<finishElement, DFA_Token>( p.f, p ) );
        }
    }
    return *this;
}

void DFA::matchToken(const string &buf, finishElement f) {

    string tokenKey = buf + "_" + (char)f;

    map<string, DFA_Token>::iterator itrN = token.N.find(tokenKey);
    map<finishElement,DFA_Token>::iterator itrA = token.A.find(f);
    if ( itrN != token.N.end() ) {
        DFA_Token &p = itrN->second;
        cout<<"< "<<p.token<<" , "<<p.id<<" , "<<( p.type==RELOP ? "relop" : "_" )<<" >"<<endl;
    }
    else if ( itrA != token.A.end() ) {
        DFA_Token &p = itrA->second;
        cout<<"< "<<p.token<<" , "<<p.id<<" , "<<buf<<" >"<<endl;
    }

}

int main()
{
    freopen("in.txt","r",stdin);
    DFA dfa;
    dfa.
      initTrans(v_DFA_Trans, v_DFA_RestTrans).
      initFinish(v_DFA_FinishState).
      initToken(v_DFA_Token).
      run();

    return 0;
}
