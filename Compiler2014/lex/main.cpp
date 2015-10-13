/**
 * @fileoverview lex
 * @author 郁森<senyu@mail.dlut.edu.cn>/一杉<yusen.ys@alibaba-inc.com>
 * @language C++
 * @description Compiler-基于DFA的简易词法分析器实现
 */

#include "dfa.h"
#include <fstream>
using namespace std;


int main() {


    fstream in; fstream out;
    in.open( "in.txt", ios_base::in );
    out.open( "lex.txt", ios_base::out);


    if ( !in.is_open() ) {
        printf("in.txt As Source\n");
        exit(-1);
    }

    istreambuf_iterator<char> ibeg(in), iend;
    string strIn(ibeg, iend); string strOut;


    DFA dfa;
    dfa.
      initTrans(v_DFA_Trans, v_DFA_RestTrans).
      initFinish(v_DFA_FinishState).
      initToken(v_DFA_Token).
      run(strIn, strOut);
    out<<strOut;

    in.close();
    out.close();

    return 0;
}
