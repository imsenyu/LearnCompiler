#include "production.h"

void Production::print(int pos, bool breakLine) const{
    cout<<getString(pos);
    breakLine && printf("\n");
}

string Production::getString(int pos) const {
    string ret;
    ret += "[" + clUtils::itoa(pId) +"]";
    ret += ptrTerm->getString();
    if ( false == ptrTerm->isTerminal ) {
        ret +=" => ";
        int cnt = 0;
        for(auto iPtrTerm : toTerms) {
            if ( cnt == pos ) {
                ret +="* ";
            }
            ret += iPtrTerm->getString() + " ";

            cnt++;
        }
        if ( cnt == pos ) {
            ret += "* ";
        }
    }
    return ret;
}
