#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <fstream>

#include "parser.h"
#include "translator.h"
#include "json/json.h"

using namespace std;

int main()
{
    ///TODOS:
    /// shared_ptr
    /// solveException 唯一Step /Recur封装
    /// 整合dfa
    /// 加注释
    fstream fileSyntax, fileLex;
    fileSyntax.open("cfg.txt",ios_base::in );
    fileLex.open("lex.txt", ios_base::in);

    if ( !fileSyntax.is_open() || !fileLex.is_open() ) {
        printf("cfg.txt As Grammar\nlex.txt As TokenArray\n");
        exit(-1);
    }

    CLikeSyntaxParser parser(true);
    parser.
        ConstructLR1(fileSyntax).
        ConstructTree(fileLex).
        nop();

    CLikeTranslator trans( &parser );
    trans.
        translate().
        nop();

    cout << "Hello world!" << endl;

    return 0;
}
