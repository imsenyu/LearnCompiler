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
    fileSyntax.open("cfg.4opera.txt",ios_base::in );
    fileLex.open("lex.4opera.txt", ios_base::in);

    syntaxParser parser(false);
    parser.
        ConstructLR1(fileSyntax).
        ConstructTree(fileLex).
        nop();

    ArithmeticTranslator trans( &parser );
    trans.
        translate().
        nop();

    cout << "Hello world!" << endl;
    return 0;
}
