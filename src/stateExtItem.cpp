#include "stateExtItem.h"

bool StateExtItem::operator()(const StateExtItem* a, const StateExtItem* b) const {
    return (*a) < (*b);
}
bool StateExtItem::operator==(const StateExtItem& b) const {
    return ptrItem == b.ptrItem && next == b.next;
}
bool StateExtItem::operator!=(const StateExtItem& b) const {
    return !(*this == b);
}
bool StateExtItem::operator<(const StateExtItem& b) const {
    if ( ptrItem != b.ptrItem ) return ptrItem < b.ptrItem;
    return next < b.next;
}
bool StateExtItem::hasNextSItem() const {
    if ( NULL == ptrItem ) return false;
    return ptrItem->hasNextSItem();
}
Term* StateExtItem::getFromTerm() const {
    if ( NULL == ptrItem ) return NULL;
    return ptrItem->getFromTerm();
}
StateItem* StateExtItem::getNextSItem() const {
    if ( NULL == ptrItem ) return NULL;
    return ptrItem->getNextSItem();
}
void StateExtItem::print(bool breakLine) const {
    if ( ptrItem ) {
        cout<<getString();
        breakLine && printf("\n");
    }
}

string StateExtItem::getString() const {
    string ret;
    if ( ptrItem ) {
        ret += ptrItem->getString();
        ret += "  char";
        ret += next->getString();
    }
    return ret;
}
