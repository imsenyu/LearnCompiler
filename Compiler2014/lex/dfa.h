#ifndef DFS_H_INCLUDED
#define DFS_H_INCLUDED

#include <cstdio>
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
using namespace std;

/* * //TODO:
 * @description 具体算法描述
1. 把struct数据导入二维数组中,由此构造DFA的状态转换表
    1.1 转换表定义: 跳转项 + 预判定项 + 终止状态项
    1.2 标识符定义: 关键字,标识
2. 依次读取 输入流字符,按照转换表依次跳转
3. 运行到终止状态后, 转入标识符判定,然后重置状态继续运行
4. 每次判定完成标识符,输出对应词
 *
  */

enum finishElement {T_ID=1,T_NUM,T_OP,T_FLOAT};
enum tokenType { NVAL=1, AVAL, RELOP};

/* *
 * @description 状态转换表跳转项结构
  */
struct DFA_Trans {
    int sfrom;
    char ch;
    int sto;
    bool ignore;
};
/* *
 * @description 状态转换表预判定项结构
  */
struct DFA_RestTrans {
    int sfrom;
    int sto;
};
/* *
 * @description 状态转换表终止状态项结构
  */
struct DFA_StateId {
    int sta;
    finishElement to;
};
/* *
 * @description 标识符识别结构
  */
struct DFA_Token {
    finishElement f;
    int id;
    string token;
    tokenType type;
};

extern DFA_Trans m_DFA_Trans[];
extern DFA_RestTrans m_DFA_RestTrans[];
extern DFA_StateId m_DFA_FinishState[];
extern DFA_Token m_DFA_Token[];
extern vector<DFA_Trans>       v_DFA_Trans;
extern vector<DFA_RestTrans>   v_DFA_RestTrans;
extern vector<DFA_StateId>     v_DFA_FinishState;
extern vector<DFA_Token>       v_DFA_Token;



class DFA {
    public:
        enum charValid {NORMAL=1,_ERROR_,PREDICATE};
    private:
        /* *
         * @description 类内 转换项+标识符 存储结构
          */
        int tokenCnt;
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
        DFA():trans(NULL),maxStates(0),startState(0), tokenCnt(0) {}
        DFA& initTrans(vector<DFA_Trans> &, vector<DFA_RestTrans> &);
        DFA& initStart(int s) {
            startState = s;
            return *this;
        }
        DFA& initFinish(vector<DFA_StateId> &);
        DFA& initToken(vector<DFA_Token> &);
        DFA& run(string& _in, string& _out);

    private:
        t_DFA_Trans (* trans)[256];
        map<int, finishElement> finish;
        t_DFA_Token token;
        int maxStates;
        int startState;
        int currentState;
        template<typename T> int getMaxStateId(T &);
        bool hasFinish(int);
        void matchToken(stringstream&, const string &, finishElement);

};


#endif // DFS_H_INCLUDED
