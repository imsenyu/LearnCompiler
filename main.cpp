#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <fstream>

#include "parser.h"
#include "translator.h"

using namespace std;

int main()
{
    fstream fileSyntax, fileLex;
    fileSyntax.open("cfg.clike.txt",ios_base::in );
    fileLex.open("lex.clike.txt", ios_base::in);

    syntaxParser parser(true);
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
