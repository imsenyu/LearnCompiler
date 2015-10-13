#include "dfa.h"
#include <sstream>


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
/* *
 * @description 初始化状态转换表
  */
DFA& DFA::initTrans(vector<DFA_Trans> &m, vector<DFA_RestTrans> &r) {
    delete[] trans;

    int maxStates = max(getMaxStateId< vector<DFA_Trans> >(m), getMaxStateId< vector<DFA_RestTrans> >(r)) ;
    trans = new t_DFA_Trans[ maxStates+1 ][256];
    /* 全初始化为失败跳转 */
    for(int state = 0; state<maxStates; state++) {
        for(int ch = 0; ch<256; ch++) {
            trans[state][ch].valid = _ERROR_;
        }
    }
    /* 跳转项 */
    for(vector<DFA_Trans>::iterator itr = m.begin(); itr!=m.end(); itr++) {
        DFA_Trans &p = *itr;
        trans[p.sfrom][p.ch].valid = NORMAL;
        trans[p.sfrom][p.ch].ignore = p.ignore;
        trans[p.sfrom][p.ch].state = p.sto;
    }
    /* 预判定项 */
    for(vector<DFA_RestTrans>::iterator itr = r.begin(); itr!=r.end(); itr++) {
        DFA_RestTrans &p = *itr;
        for(int ch = 0; ch<256; ch++) {
            if ( trans[p.sfrom][ch].valid == _ERROR_) {
                trans[p.sfrom][ch].valid = PREDICATE;
                trans[p.sfrom][ch].state = p.sto;
            }
        }
    }
    return *this;
}

DFA& DFA::initFinish(vector<DFA_StateId> &f) {
    finish.clear();
    /* 终止状态项 */
    for( vector<DFA_StateId>::iterator itr = f.begin(); itr!=f.end(); itr++) {
        finish.insert(pair<int,finishElement>(itr->sta,itr->to));
    }
    return *this;
}
bool DFA::hasFinish(int s) {
    return finish.find(s) != finish.end();
}
DFA& DFA::run(string& fIn, string& fOut) {
    char input;
    bool runnable = true;
    bool inputable = true;
    tokenCnt = 0;
    currentState = startState;
    string buffer = "";

    int inputCnt = 0;
    while(runnable) {
        if (inputable &&  inputCnt >= fIn.size() ) {
            break;
        }
        if ( inputable ) {
            input = fIn[ inputCnt++ ];
        }

        inputable = true;
        t_DFA_Trans next = trans[currentState][input];

        stringstream buf;

        if ( next.valid == _ERROR_ ) {
            runnable = false;

            buf<<"ERROR Character ["<<input+","<<(int)input<<"]"<<endl;
        } else if ( next.valid == NORMAL || next.valid == PREDICATE) {
            if ( next.valid == NORMAL ) {
                if ( !next.ignore )buffer += input;
            } else if ( next.valid == PREDICATE ) {
                inputable = false;
            }
            if ( hasFinish(next.state) ) {
                matchToken(buf, buffer, finish.find(next.state)->second);
                buffer = "";
                currentState = startState;
            } else {
                currentState = next.state;
            }
        }
        fOut += buf.str();
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

void DFA::matchToken(stringstream& fOut, const string &buf, finishElement f) {

    string tokenKey = buf + "_" + (char)f;

    map<string, DFA_Token>::iterator itrN = token.N.find(tokenKey);
    map<finishElement,DFA_Token>::iterator itrA = token.A.find(f);
    if ( itrN != token.N.end() ) {
        DFA_Token &p = itrN->second;
        if ( p.type == RELOP ) {
            //printf("< relop , %-2d , %s >\n",p.id,p.token.c_str());
            fOut<<"relop "<<(tokenCnt++)<<" "<<p.token<<endl;
            //printf("relop %-2d %s\n",p.id,p.token.c_str());
        } else {
            //printf("< %-5s , %-2d , %s >\n",p.token.c_str(),p.id,"_");
            fOut<<p.token<<" "<<(tokenCnt++)<<" _"<<endl;
            //printf("%-5s %-2d %s\n",p.token.c_str(),p.id,"_");
        }

    } else if ( itrA != token.A.end() ) {
        DFA_Token &p = itrA->second;
        //printf("< %-5s , %-2d , %s >\n",p.token.c_str(),p.id,buf.c_str());
        //printf("%-5s %-2d %s\n",p.token.c_str(),p.id,buf.c_str());
        fOut<<p.token<<" "<<(tokenCnt++)<<" "<<buf<<endl;
    }
    return;
}


/* *
 * @description 把struct初始化成vector结构，后续迭代调用方便
  */

DFA_Trans m_DFA_Trans[] = {
    /* 空格换行 */
    {0,' ',0,true},{0,'\n',0,true},{0,'\r',0,true},
    /* 字母 */
    {0,'a',1},{0,'b',1},{0,'c',1},{0,'d',1},{0,'e',1},{0,'f',1},{0,'g',1},{0,'h',1},{0,'i',1},{0,'j',1},{0,'k',1},{0,'l',1},{0,'m',1},{0,'n',1},{0,'o',1},{0,'p',1},{0,'q',1},{0,'r',1},{0,'s',1},{0,'t',1},{0,'u',1},{0,'v',1},{0,'w',1},{0,'x',1},{0,'y',1},{0,'z',1},
    {0,'A',1},{0,'B',1},{0,'C',1},{0,'D',1},{0,'E',1},{0,'F',1},{0,'G',1},{0,'H',1},{0,'I',1},{0,'J',1},{0,'K',1},{0,'L',1},{0,'M',1},{0,'N',1},{0,'O',1},{0,'P',1},{0,'Q',1},{0,'R',1},{0,'S',1},{0,'T',1},{0,'U',1},{0,'V',1},{0,'W',1},{0,'X',1},{0,'Y',1},{0,'Z',1},
    /* 字母数字 */
    {1,'a',1},{1,'b',1},{1,'c',1},{1,'d',1},{1,'e',1},{1,'f',1},{1,'g',1},{1,'h',1},{1,'i',1},{1,'j',1},{1,'k',1},{1,'l',1},{1,'m',1},{1,'n',1},{1,'o',1},{1,'p',1},{1,'q',1},{1,'r',1},{1,'s',1},{1,'t',1},{1,'u',1},{1,'v',1},{1,'w',1},{1,'x',1},{1,'y',1},{1,'z',1},
    {1,'A',1},{1,'B',1},{1,'C',1},{1,'D',1},{1,'E',1},{1,'F',1},{1,'G',1},{1,'H',1},{1,'I',1},{1,'J',1},{1,'K',1},{1,'L',1},{1,'M',1},{1,'N',1},{1,'O',1},{1,'P',1},{1,'Q',1},{1,'R',1},{1,'S',1},{1,'T',1},{1,'U',1},{1,'V',1},{1,'W',1},{1,'X',1},{1,'Y',1},{1,'Z',1},
    {1,'0',1},{1,'1',1},{1,'2',1},{1,'3',1},{1,'4',1},{1,'5',1},{1,'6',1},{1,'7',1},{1,'8',1},{1,'9',1},
    /* 数字 */
    {0,'0',3},{3,'.',12},
    {12,'0',12},{12,'1',12},{12,'2',12},{12,'3',12},{12,'4',12},{12,'5',12},{12,'6',12},{12,'7',12},{12,'8',12},{12,'9',12},
    {0,'1',4},{0,'2',4},{0,'3',4},{0,'4',4},{0,'5',4},{0,'6',4},{0,'7',4},{0,'8',4},{0,'9',4},
    {4,'0',4},{4,'1',4},{4,'2',4},{4,'3',4},{4,'4',4},{4,'5',4},{4,'6',4},{4,'7',4},{4,'8',4},{4,'9',4},
    /* {0,'0',3},{0,'1',3},{0,'2',3},{0,'3',3},{0,'4',3},{0,'5',3},{0,'6',3},{0,'7',3},{0,'8',3},{0,'9',3},
    {3,'0',3},{3,'1',3},{3,'2',3},{3,'3',3},{3,'4',3},{3,'5',3},{3,'6',3},{3,'7',3},{3,'8',3},{3,'9',3}, */
    /* 运算符,括号 */
    {0,'+',5},{0,'-',5},{0,'*',5},{0,'/',5},{0,'(',5},{0,')',5},{0,'{',5},{0,'}',5},{0,';',5},
    {0,'<',6},{0,'>',6},{0,'!',6},
    {6,'=',7},{0,'=',9},{9,'=',10}
};
DFA_RestTrans m_DFA_RestTrans[] = {
    /* 预判定，不读取next的状态 */
    {1,2},{3,13},{12,14},{4,13},{6,8},{9,11}
};
DFA_StateId m_DFA_FinishState[] = {
    {2,T_ID},{13,T_NUM},{14,T_FLOAT},{5,T_OP},{7,T_OP},{8,T_OP},{10,T_OP},{11,T_OP}
};
DFA_Token m_DFA_Token[] = {
    {T_ID, 1, "int", NVAL},
    {T_ID, 2, "if", NVAL},
    {T_ID, 3, "then", NVAL},
    {T_ID, 4, "else", NVAL},
    {T_ID, 23, "endif", NVAL},
    {T_ID, 5, "while", NVAL},
    {T_ID, 6, "do", NVAL},
    {T_FLOAT, 22, "FLOAT", AVAL},
    {T_ID, 7, "ID", AVAL},
    {T_NUM, 8, "NUM", AVAL},
    {T_OP, 9, "+", NVAL},
    {T_OP, 10, "-", NVAL},
    {T_OP, 11, "*", NVAL},
    {T_OP, 12, "/", NVAL},
    {T_ID, 13, "and", NVAL},/* 由于是多字符与ID冲突，不作T_OP来判断 */
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

vector<DFA_Trans>       v_DFA_Trans( m_DFA_Trans, m_DFA_Trans+sizeof(m_DFA_Trans)/sizeof(m_DFA_Trans[0]) );
vector<DFA_RestTrans>   v_DFA_RestTrans( m_DFA_RestTrans, m_DFA_RestTrans+sizeof(m_DFA_RestTrans)/sizeof(m_DFA_RestTrans[0]) );
vector<DFA_StateId>     v_DFA_FinishState( m_DFA_FinishState, m_DFA_FinishState+sizeof(m_DFA_FinishState)/sizeof(m_DFA_FinishState[0]) );
vector<DFA_Token>       v_DFA_Token( m_DFA_Token, m_DFA_Token+sizeof(m_DFA_Token)/sizeof(m_DFA_Token[0]));
