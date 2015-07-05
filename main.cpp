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

    fstream fileSyntax, fileLex;
    fileSyntax.open("./test/cfg.clike.txt",ios_base::in );
    fileLex.open("./test/lex.clike.txt", ios_base::in);

    syntaxParser parser(false);
    parser.
        ConstructLR1(fileSyntax).
        ConstructTree(fileLex).
        nop();

    CLikeTranslator trans( &parser );
    trans.
        translate().
        nop();

    cout << "Hello world!" << endl;

    Json::Value value;
    value["test"].append(1);
    value["test"][1u]="333";;
    value["place"] = 123;
    value["place"] = "123133";
    cout<<value.toStyledString()<<endl;

    return 0;
}
