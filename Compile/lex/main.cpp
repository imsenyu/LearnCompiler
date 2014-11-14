/**
 * @fileoverview lex
 * @author 郁森<senyu@mail.dlut.edu.cn>/一杉<yusen.ys@alibaba-inc.com>
 * @language C++
 * @description Compiler-基于DFA的简易词法分析器实现
 */
#include <cstdio>
#include <iostream>
#include <vector>
#include <map>
#include <string>

using namespace std;

/** //TODO:
 * @description 具体算法描述
1. 把struct数据导入二维数组中,由此构造DFA的状态转换表
    1.1 转换表定义: 跳转项 + 预判定项 + 终止状态项
    1.2 标识符定义: 关键字,标识
2. 依次读取 输入流字符,按照转换表依次跳转
3. 运行到终止状态后, 转入标识符判定,然后重置状态继续运行
4. 每次判定完成标识符,输出对应词
 *
 */

enum finishElement {T_ID=1,T_NUM,T_OP};
enum tokenType { NVAL=1, AVAL, RELOP};

/**
 * @description 状态转换表跳转项结构
 */
struct DFA_Trans {
    int sfrom;
    char ch;
    int sto;
    bool ignore;
};
/**
 * @description 状态转换表预判定项结构
 */
struct DFA_RestTrans {
    int sfrom;
    int sto;
};
/**
 * @description 状态转换表终止状态项结构
 */
struct DFA_StateId {
    int sta;
    finishElement to;
};
/**
 * @description 标识符识别结构
 */
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
    /*预判定，不读取next的状态*/
    {1,2},{3,4},{6,8},{9,11}
};
DFA_StateId m_DFA_FinishState[] = {
    {2,T_ID},{4,T_NUM},{5,T_OP},{7,T_OP},{8,T_OP},{10,T_OP},{11,T_OP}
};
DFA_Token m_DFA_Token[] = {
    {T_ID, 1, "int", NVAL},
    {T_ID, 2, "if", NVAL},
    {T_ID, 3, "then", NVAL},
    {T_ID, 4, "else", NVAL},
    {T_ID, 5, "while", NVAL},
    {T_ID, 6, "do", NVAL},
    {T_ID, 22, "endif", NVAL},
    {T_ID, 7, "ID", AVAL},
    {T_NUM, 8, "NUM", AVAL},
    {T_OP, 9, "+", NVAL},
    {T_OP, 10, "-", NVAL},
    {T_OP, 11, "*", NVAL},
    {T_OP, 12, "/", NVAL},
    {T_ID, 13, "and", NVAL},/*由于是多字符与ID冲突，不作T_OP来判断*/
    {T_ID, 14, "or", NVAL},
    {T_OP, 15, "<", RELOP},
    {T_OP, 15, ">", RELOP},
    {T_OP, 15, "<=", RELOP},
    {T_OP, 15, ">=", RELOP},
    {T_OP, 15, "!=", RELOP},
    {T_OP, 15, "==", RELOP},
    {T_OP, 16, "{", NVAL},
    {T_OP, 17, "}", NVAL},
    {T_OP, 18, ";", NVAL},
    {T_OP, 19, "(", NVAL},
    {T_OP, 20, ")", NVAL},
    {T_OP, 21, "=", NVAL}
};
/**
 * @description 把struct初始化成vector结构，后续迭代调用方便
 */
vector<DFA_Trans>       v_DFA_Trans( m_DFA_Trans, m_DFA_Trans+sizeof(m_DFA_Trans)/sizeof(m_DFA_Trans[0]) );
vector<DFA_RestTrans>   v_DFA_RestTrans( m_DFA_RestTrans, m_DFA_RestTrans+sizeof(m_DFA_RestTrans)/sizeof(m_DFA_RestTrans[0]) );
vector<DFA_StateId>     v_DFA_FinishState( m_DFA_FinishState, m_DFA_FinishState+sizeof(m_DFA_FinishState)/sizeof(m_DFA_FinishState[0]) );
vector<DFA_Token>       v_DFA_Token( m_DFA_Token, m_DFA_Token+sizeof(m_DFA_Token)/sizeof(m_DFA_Token[0]));

class DFA {
    public:
        enum charValid {NORMAL=1,ERROR,PREDICATE};
    private:
        /**
         * @description 类内 转换项+标识符 存储结构
         */
        struct t_DFA_Trans {
            int state;
            charValid valid;
            bool ignore;
        };
        struct t_DFA_Token {
            map<string, DFA_Token> N;
            map<finishElement,DFA_Token> A;
            void clear() {
                N.clear();
                A.clear();
            }
        };
    public:
        DFA():trans(NULL),maxStates(0),startState(0) {}
        DFA& initTrans(vector<DFA_Trans> &, vector<DFA_RestTrans> &);
        DFA& initStart(int s) {
            startState = s;
            return *this;
        }
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
/**
 * @description 初始化状态转换表
 */
DFA& DFA::initTrans(vector<DFA_Trans> &m, vector<DFA_RestTrans> &r) {
    delete[] trans;

    int maxStates = getMaxStateId< vector<DFA_Trans> >(m);
    trans = new t_DFA_Trans[ maxStates+1 ][256];

    /*全初始化为失败跳转*/
    for(int state = 0; state<maxStates; state++) {
        for(int ch = 0; ch<256; ch++) {
            trans[state][ch].valid = ERROR;
        }
    }
    /*跳转项*/
    for(vector<DFA_Trans>::iterator itr = m.begin(); itr!=m.end(); itr++) {
        DFA_Trans &p = *itr;
        trans[p.sfrom][p.ch].valid = NORMAL;
        trans[p.sfrom][p.ch].ignore = p.ignore;
        trans[p.sfrom][p.ch].state = p.sto;
    }
    /*预判定项*/
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
    /*终止状态项*/
    for( vector<DFA_StateId>::iterator itr = f.begin(); itr!=f.end(); itr++) {
        finish.insert(pair<int,finishElement>(itr->sta,itr->to));
    }
    return *this;
}
bool DFA::hasFinish(int s) {
    return finish.find(s) != finish.end();
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

        if ( next.valid == ERROR ) {
            runnable = false;
            cout<<"ERROR Character ["<<input<<","<<(int)input<<"]"<<endl;
        } else if ( next.valid == NORMAL || next.valid == PREDICATE) {
            if ( next.valid == NORMAL ) {
                if ( !next.ignore )buffer += input;
            } else if ( next.valid == PREDICATE ) {
                inputable = false;
            }
            if ( hasFinish(next.state) ) {
                matchToken(buffer, finish.find(next.state)->second);
                buffer = "";
                currentState = startState;
            } else {
                currentState = next.state;
            }
        }
    }
    return *this;
}

DFA& DFA::initToken(vector<DFA_Token> &t) {
    token.clear();
    for(vector<DFA_Token>::iterator itr = t.begin(); itr!=t.end(); itr++) {
        DFA_Token &p = *itr;
        if ( p.type != AVAL ) {
            token.N.insert( pair<string, DFA_Token>( p.token + "_" + (char)p.f , p ) );
        } else {
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
        if ( p.type == RELOP ) {
            printf("< relop , %-2d , %s >\n",p.id,p.token.c_str());
        } else {
            printf("< %-5s , %-2d , %s >\n",p.token.c_str(),p.id,"_");
        }

    } else if ( itrA != token.A.end() ) {
        DFA_Token &p = itrA->second;
        printf("< %-5s , %-2d , %s >\n",p.token.c_str(),p.id,buf.c_str());
    }
    return;
}

int main() {

    freopen("in.txt","r",stdin);
    freopen("lex.txt","w",stdout);

    DFA dfa;
    dfa.
      initTrans(v_DFA_Trans, v_DFA_RestTrans).
      initFinish(v_DFA_FinishState).
      initToken(v_DFA_Token).
      run();

    return 0;
}
