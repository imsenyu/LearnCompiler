#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <fstream>

#include "translator.h"

using namespace std;

int main()
{

    fstream fileSyntax, fileLex;
    fileSyntax.open("cfg.4opera.txt",ios_base::in );
    fileLex.open("lex.4opera.txt", ios_base::in);

    Translator parser;
    parser.
        inputTerm(fileSyntax).
        showTerm().
        inputProduction(fileSyntax).
        showProduction().
        buildStateItems().
        showStateItems().
        buildStateSet().
        showStateSet().
        buildActionGotoTable().
        showActionGotoTable().
        inputLex(fileLex).
        runSyntaxAnalyse().
        showSyntaxTree().
        syntaxDirectedTranslation().
        nop();

    cout << "Hello world!" << endl;
    return 0;
}
