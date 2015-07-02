#include "term.h"
#include "iostream"

Term* const nullTermPtr = new Term("",true);
Term* const endTermPtr = new Term("#",true);

Term::Term(istream& in) {
    in>>tName>>isTerminal;
}
void Term::print(bool breakLine) const {
    cout<<"["<<tName<<"]";
    breakLine && printf("\n");
}
