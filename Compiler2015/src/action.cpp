#include "action.h"

void Action::print(bool breakLine) const{
    cout<<getString();
    breakLine && printf("\n");
}

string Action::getString() const {
    string ret;
    switch(type) {
        case Err: return "Err";
        case Acc: return "Acc";
        case Step: return "S"+clUtils::itoa(toId);
        case Goto: return "G"+clUtils::itoa(toId);
        case Recur: return "R"+clUtils::itoa(toId);
    }
    return "";
}

bool ActionGotoTable::build() {
    if ( NULL == ptrData || NULL == ptrVec || NULL == origin ) return false;

    ///Step & Goto
    TypeStateTable& mpStateTable = *ptrData;
    for(auto row : mpStateTable) {
        int fromId = row.first;
        for(auto col : row.second) {
            Term* ptrStepTerm = col.first;
            int toId = col.second;
            ///Terminal True -> Step  Else Goto
            Action newAct( ptrStepTerm->isTerminal ? Action::Type::Step : Action::Type::Goto, toId );

            bool ret = table.add( fromId, ptrStepTerm, newAct );
            if ( false == ret )
                displayConflict( fromId, ptrStepTerm, newAct );
        }
    }

    ///Recur
    TypeVectorStates& vecStates = *ptrVec;
    for(int fromId = 0;fromId < vecStates.size();fromId++) {
        for( auto ptrSEItem : vecStates[fromId]->collection ) {
            if ( false == ptrSEItem->hasNextSItem() ) {
                Action newAct(Action::Type::Recur, ptrSEItem->ptrItem->ptrPdt->pId);
                bool ret = table.add( fromId, ptrSEItem->next, newAct );
                if ( false == ret )
                    displayConflict( fromId, ptrSEItem->next, newAct );
            }
        }
    }
    ///Acc
    vector<int> vecAcc;
    StateExtItem* target = new StateExtItem( origin->vecSItems[ origin->toTerms.size() ], endTermPtr );
    target->print(true);
    for(int vId = 0; vId < vecStates.size(); vId++) {
        StateSet::StateCollection& collection = vecStates[vId]->collection;
        if ( collection.find( target ) != collection.end() )
            vecAcc.push_back( vId );
    }

    Action actACC(Action::Type::Acc, -1);
    for(auto accId : vecAcc) {
        bool ret = table.add( accId, endTermPtr, actACC, true );
        if ( false == ret )
            displayConflict( accId, endTermPtr, actACC );
    }
    delete target;
    return true;
}

void ActionGotoTable::displayConflict(const int fromId, Term* ptrNextTerm, const Action& act ) {
    printf("Conflict Boom Up!\n");
    printf("  Exist: ");table.get(fromId, ptrNextTerm )->print(true);
    printf("  New:   ");act.print(true);
}
