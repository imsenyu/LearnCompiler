#include "action.h"

void Action::print(bool breakLine) const{
    switch(type) {
        case Err: printf("Err"); break;
        case Acc: printf("Acc"); break;
        case Step: printf("S%d",toId); break;
        case Goto: printf("G%d",toId); break;
        case Recur: printf("R%d",toId); break;
    }
    breakLine && printf("\n");
}

bool ActionGotoTable::build() {
    if ( NULL == ptrData || NULL == ptrVec || NULL == origin ) return false;
    printf("Action Goto Table\n");
    printf("  Step, Goto\n");
    TypeStateTable& mpStateTable = *ptrData;
    for(auto row : mpStateTable) {
        int stateFromId = row.first;
        for(auto col : row.second) {
            Term* ptrStepTerm = col.first;
            int stateToId = col.second;
            if ( true == ptrStepTerm->isTerminal ) {
                ///Step 移进
                bool ret = table.add( stateFromId, ptrStepTerm, Action(Action::Type::Step, stateToId) );
                if ( false == ret ) {
                    printf("Conflict\n");
                    table.get(stateFromId,ptrStepTerm)->print(true);
                    printf("Step, %d\n",stateToId);

                }
            }
            else {
                bool ret = table.add( stateFromId, ptrStepTerm, Action(Action::Type::Goto, stateToId) );
                if ( false == ret ) {
                    printf("Conflict\n");
                    table.get(stateFromId,ptrStepTerm)->print(true);
                    printf("Goto, %d\n",stateToId);

                }
            }
        }
    }
    ///还有一个Recur
    printf("  Recur\n");
    TypeVectorStates& vecStates = *ptrVec;
    for(int fromId = 0;fromId < vecStates.size();fromId++) {
        for( auto ptrSEItem : vecStates[fromId]->collection ) {
            if ( false == ptrSEItem->hasNextSItem() ) {
                bool ret = table.add( fromId, ptrSEItem->next, Action(Action::Type::Recur, ptrSEItem->ptrItem->ptrPdt->pId) );
                if ( false == ret ) {
                    printf("Conflict\n");
                    table.get(fromId,ptrSEItem->next)->print(true);
                    printf("Recur, %d\n",ptrSEItem->ptrItem->ptrPdt->pId);

                }
            }
        }
    }
    ///还有最后一个ACC
    printf("  Acc");
    vector<int> vecAcc;
    StateExtItem* target = new StateExtItem( origin->vecSItems[ origin->toTerms.size() ], endTermPtr );
    target->print(true);
    for(int vId = 0; vId < vecStates.size(); vId++) {
        StateSet::StateCollection& collection = vecStates[vId]->collection;
        if ( vId == 1 ) {
            (*collection.begin())->print(true);
            printf(" bool [%d]\n", *(*collection.begin()) == *target);

        }
        if ( collection.find( target ) != collection.end() ) {
            vecAcc.push_back( vId );
        }
    }
    printf(" %d\n",vecAcc.size());
    for(auto accId : vecAcc) {
        bool ret = table.add( accId, endTermPtr, Action(Action::Type::Acc, -1), true );
        ///可覆盖
        if ( false == ret ) {
            printf("Conflict\n");
            table.get(accId, endTermPtr)->print(true);
            printf("Acc, %d\n",-1);

        }
    }
    delete target;
    return true;
}
