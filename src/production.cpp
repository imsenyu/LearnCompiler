#include "production.h"

void Production::print(int pos, bool breakLine) const{
    printf("[%d] ",pId);
    ptrTerm->print(false);
    if ( false == ptrTerm->isTerminal ) {
        printf(" => ");
        int cnt = 0;
        for(auto iPtrTerm : toTerms) {
            if ( cnt == pos ) {
                printf("* ");
            }
            iPtrTerm->print(false);
            printf(" ");
            cnt++;
        }
        if ( cnt == pos ) {
            printf("* ");
        }
    }
    breakLine && printf("\n");
}
