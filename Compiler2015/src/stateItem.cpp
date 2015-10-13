#include "stateItem.h"

bool StateItem::hasNextSItem() const {
    if ( NULL == ptrPdt ) return false;
    return pos < ptrPdt->toTerms.size();
}
Term* StateItem::getNextTerm() const {
    if ( NULL == ptrPdt || pos >= ptrPdt->toTerms.size()) return NULL;
    return ptrPdt->toTerms[ pos ];
}
Term* StateItem::getFromTerm() const {
    if ( NULL == ptrPdt ) return NULL;
    return ptrPdt->ptrTerm;
}
StateItem* StateItem::getNextSItem() const {
    if ( NULL == ptrPdt ) return NULL;
    return ptrPdt->vecSItems[ pos+1 ];
}
void StateItem::print(bool breakLine) const {
    cout<<getString();
    breakLine && printf("\n");
}

string StateItem::getString() const {
    if ( NULL != ptrPdt ) {
        return ptrPdt->getString(pos);
    }
    else return "";
}
