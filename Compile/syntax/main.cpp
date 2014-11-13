/**
 * @fileoverview syntax
 * @author 郁森<senyu@mail.dlut.edu.cn>/一杉<yusen.ys@alibaba-inc.com>
 * @language C++
 * @description Compiler-基于LL(0)的简易语法分析器实现
 */
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <map>
#include <set>
#include <stack>
#include <algorithm>
#include "strtool.h"

using namespace std;


///初始化delim[|]，generator[ ]，仅限于单字符，目前仅支持 单个非终结符左部,加入终止符[#],终结符初始化列表，非终结符初始化列表，不在列表报错



class LR_Syntax {
    public:
        struct Rule {
            string source;
            string value;
        };
        struct Term {
            string name;
            bool terminal;
            int typeId;
            bool extra;
        };
    private:
        struct Production {
            int from;
        };
        class m_Rule {
            public:
                vector<Term*> term;
                Term* from;
                int ruleID;
                m_Rule(string & , map<string,Term*> , string);
                void showTerm(int=-1);
        };
        struct m_Item {
            m_Rule *rule; //具体规则指针
            int itemID; //LR项目 编号
            int point;  //小圆点位置，从0开始
            m_Item(m_Rule* a, int b, int c):rule(a),itemID(b),point(c) {}
        };
        struct m_ExtItem {
            m_Item *item;
            Term *preTerm;
            m_ExtItem(m_Item* a, Term* b=NULL):item(a),preTerm(b) {}
        };

        class ExtItemPtrCmp
        {
            public:
            bool operator() (m_ExtItem* a, m_ExtItem* b)
            {
                return a->item == b->item ? a->preTerm<b->preTerm : a->item<b->item;
            }
        };
        struct m_IState {
            set<m_ExtItem*,ExtItemPtrCmp> data;
            int stateID;
            m_IState(int a):stateID(a) {}
        };
        enum actionType { Step=1,Recur,Gto,Acc,Err };
        struct m_Action {
            actionType type;
            m_IState* _step;
            m_Rule* _recur;
            m_IState* _gto;
            m_Action():type(Err),_step(NULL),_recur(NULL),_gto(NULL) {}
        };
        struct m_LexGroup {
            Term* term;
            int typeId;
            string extra;
            m_LexGroup(int a,char*b, Term* c=NULL):typeId(a),term(c) {
                if ( b[0]!='_' || b[1] != 0 ) {
                    extra = b;
                }
            }
        };

    public:
        LR_Syntax():ruleDelim("|"),lexFile(NULL) {}
        LR_Syntax& initSymbol( vector<LR_Syntax::Term>& );
        LR_Syntax& initProduction( vector<LR_Syntax::Rule>& ); //vector 导入 map
        LR_Syntax& showProduction();
        LR_Syntax& buildItems();
        LR_Syntax& buildAnalyticalTable();
        LR_Syntax& showIState(int = -1);
        LR_Syntax& buildActionGotoTable();
        LR_Syntax& showAGTable();
        LR_Syntax& initLEXSymbol(string);
        LR_Syntax& runSyntaxAutoman();
    private:
        int calcClosure(m_IState* );
        int fullCalcClosure(m_IState* );
        bool matchClosurePattern(m_ExtItem&,Term*&,vector<Term*>&);
        int bfsFirstSet( vector<Term*>&, set<Term*>& );
        m_Item* itemNextStep(m_Item*);
        int sameIState(m_IState*);
        bool destroyTmpIState(m_IState*);
        m_IState* ATableSearch(m_IState*, Term*);
        ///自动解决 if-then-else 样式的问题，遇到 需要 char 移近项，忽略，转而使用#结束规约
        void autoSolveConflict(map<Term*, m_Action*> &, Term*, m_ExtItem&);
        m_Action* ActionTableSearch( m_IState*, Term* );
    private:
        string ruleDelim;
        map<string,Term*> symbolTable;
        map<Term*, vector<m_Rule*>* > productionTable;
        map<Term*, vector<m_Item*>* > itemTable;   //LR项目表
        ///计算I状态封装， 以及 [I*][Term*] => 映射表,边构造映射表边计算新的I
        vector<m_IState*> v_IState;
        map<m_IState*, map<Term*, m_Action*>* > ActionTable;
        map<m_IState*, map<Term*, m_IState*>* > StateTable;
        queue<m_LexGroup*> queueLex;
        FILE *lexFile;



};



LR_Syntax::m_Rule::m_Rule(string& s, map<string,Term*> m, string delim) {
    vector<string> vt;
    strtool::split(s, vt, delim);
    for(vector<string>::iterator itr=vt.begin(); itr!=vt.end(); itr++) {
        if ( m.find(*itr + "_t") != m.end() ) {
            term.push_back( m.find(*itr + "_t")->second );
        } else if ( m.find(*itr + "_n") != m.end() ) {
            term.push_back( m.find(*itr + "_n")->second );
        } else
            cout<<"ERROR...Can't Find RightTerm["<<*itr<<"] in SymbolTable"<<endl;
    }
    /*cout<<"m_Rule:"<<s<<ends;
    for(vector<Term*>::iterator itr=term.begin(); itr!=term.end(); itr++) {
        Term &p = **itr;
        cout<<" "<<p.name<<" "<<p.terminal<<endl;
    }*/
}

void LR_Syntax::m_Rule::showTerm(int point) {
    vector<Term*>::iterator itr;
    int cnt=1;
    if(point==0) cout<<"[#]   ";
    for(itr=term.begin();itr!=term.end();itr++,cnt++) {
        Term &p = **itr;
        cout<<p.name<<p.terminal;
        if(cnt==point)cout<<"   [#]";
        cout<<"   ";
    }
}

LR_Syntax& LR_Syntax::initSymbol( vector<LR_Syntax::Term>& t) {
    for(vector<LR_Syntax::Term>::iterator itr = t.begin(); itr!=t.end(); itr++) {
        Term &p = *itr;
        symbolTable.insert( pair<string,Term*>( p.name + "_" + (p.terminal?"t":"n"), &p ) );
    }
    return *this;
}
LR_Syntax& LR_Syntax::initProduction( vector<LR_Syntax::Rule>& r) {
    cout<<"Production"<<endl;
    int ruleCnt = 0;
    for(vector<LR_Syntax::Rule>::iterator itr=r.begin(); itr!=r.end(); itr++) {
        Rule &p = *itr;
        m_Rule *address = new m_Rule(p.value, symbolTable, ruleDelim);
        if ( symbolTable.find( p.source + "_n" ) != symbolTable.end() ) {
            Term* sym = symbolTable.find( p.source + "_n" )->second;
            //cout<<" From Symbol["<<sym->name<<"] "<<sym->terminal<<endl;
            address->from = sym;
            address->ruleID = ruleCnt ++;

            cout<<" Rule["<<address->ruleID<<"]: "<<sym->name<<sym->terminal<<" => ";
            address->showTerm();
            cout<<endl;

            if ( productionTable.find(sym) == productionTable.end() ) {
                vector<m_Rule*>* container = new vector<m_Rule*>;
                container->push_back( address );
                productionTable.insert( pair<Term*,vector<m_Rule*>* >( sym, container ) );
            }
            else {
                productionTable.find(sym)->second->push_back( address );
            }
        } else {
            cout<<"ERROR...Can't Find LeftTerm["<<p.source<<"] in SymbolTable"<<endl;
            delete address;
        }
    }
    return *this;
}

LR_Syntax& LR_Syntax::showProduction() {
    map<Term*, vector<m_Rule*>* >::iterator itr;
    for(itr=productionTable.begin();itr!=productionTable.end();itr++) {
        Term &x = *itr->first;
        vector<m_Rule*> &y = *itr->second;
        cout<<"Term:"<<x.name<<endl;

        vector<m_Rule*>::iterator jtr;
        int cnt = 1;
        for(jtr=y.begin();jtr!=y.end();jtr++) {
            m_Rule& r = **jtr;
            cout<<"  "<<cnt++<<"  ";
            r.showTerm();
            cout<<endl;
        }
    }
    return *this;
}

LR_Syntax& LR_Syntax::buildItems() {
    ///map<Term*, vector<m_Rule*>* > productionTable;
    ///map<Term*, vector<m_Item*>* > itemTable;   //LR项目表
    map<Term*, vector<m_Rule*>* >::iterator itr;
    int termCnt = 0;
    for(itr=productionTable.begin();itr!=productionTable.end();itr++) {
        vector<m_Rule*> &y = *itr->second;
        vector<m_Rule*>::iterator jtr;

        for(jtr=y.begin();jtr!=y.end();jtr++) {
            m_Rule& r = **jtr;
            vector<Term*>::iterator jtr;
            for(int cnt=0;cnt<=r.term.size();cnt++) {
                m_Item* p = new m_Item(&r, termCnt++,cnt);
                if ( itemTable.find( r.from ) !=itemTable.end() ) {
                    vector<m_Item*> &vm = *itemTable.find(r.from)->second;
                    vm.push_back( p );
                }
                else {
                    vector<m_Item*> *vm = new vector<m_Item*>;
                    vm->push_back( p );
                    itemTable.insert( pair<Term*,vector<m_Item*>*>( r.from, vm ) );
                }
            }
        }
    }

    map<Term*, vector<m_Item*>* >::iterator mtr;
    for(mtr=itemTable.begin();mtr!=itemTable.end();mtr++) {
        vector<m_Item*> &vy = *mtr->second;
        vector<m_Item*>::iterator vtr;
        for(vtr=vy.begin();vtr!=vy.end();vtr++) {
            m_Item &p = **vtr;
            cout<<"["<<p.itemID<<"] "<<p.rule->from->name<<" => ";
            p.rule->showTerm(p.point);
            cout<<endl;
        }
    }
    return *this;
}

bool LR_Syntax::matchClosurePattern(m_ExtItem& eI,Term* & N,vector<Term*> & after) {
    vector<Term*> &term = eI.item->rule->term;
    int step = eI.item->point;
    if ( step<term.size() ) {
        N = term[step];
        for(int i=step+1;i<term.size();i++)
            after.push_back( term[i] );
        return true;
    }
    else {
        return false;
    }
}

int LR_Syntax::bfsFirstSet( vector<Term*>& source, set<Term*>& result ) {
    ///从 source 中取 [0] 空就算了，如果是 N下一步 ，T push return
    ///扔到queue中，去productionTable中查 所有rule
    ///对 每个rule.term 这个vector 取 [0] N则扔到 queue中，T则push
    /// 用map<rule*, bool> 记录 是否遍历过
    ///!!!不支持 查找 含有 空规则 的 产生式！ 空规则 需要使用 dfa查找，bfs不记录路径不能回溯
    if ( source.size() == 0 ) return 0;
    if ( source[0]->terminal ) {
        result.insert( source[0] );
        return 1;
    }


    queue<Term*> q;
    map<m_Rule*, bool> visit;

    q.push( source[0] );
    while( !q.empty() ) {
        Term &top = *q.front();
        if ( productionTable.find(&top) != productionTable.end() ) {
            vector<m_Rule*> &y = *productionTable.find(&top)->second;
            vector<m_Rule*>::iterator itr;
            for(itr=y.begin();itr!=y.end();itr++) {
                m_Rule &r = **itr;
                if ( visit.find( &r ) != visit.end() )continue;
                if ( r.term.size() == 0 ) continue;
                else if ( r.term[0]->terminal ) {
                    result.insert( r.term[0] );
                }
                else {
                    q.push( r.term[0] );
                }
                visit.insert( pair<m_Rule*,bool>( &r, true ) );
            }
        }
        q.pop();
    }

    return 0;
}

int LR_Syntax::fullCalcClosure(m_IState* sta) {
    int oriCnt;
    do {
        oriCnt = sta->data.size();
        calcClosure(sta);
    }
    while( oriCnt<sta->data.size() );

    return sta->data.size();
}

int LR_Syntax::calcClosure(m_IState* sta) {
    ///其中任何一个 ExItem 若有 .Nb, a 遍历 B 的Items ，计算 First( ba )
    /// 把 对应 B的Item,b 加入
    //cout<<endl<<" In Closure"<<endl;
    set<m_ExtItem*,ExtItemPtrCmp>::iterator itr;
    m_IState tmp(*sta);

    //cout<<"  given IState ";
    //cout<<"  ID = "<<tmp.stateID<<endl;
    /*for(int i=0;i<tmp.data.size();i++) {
        cout<<"  "<<tmp.data[i]->item->rule->from->name<<" => ";
        tmp.data[i]->item->rule->showTerm(tmp.data[i]->item->point),
        cout<<" char "<<((tmp.data[i]->preTerm)?(tmp.data[i]->preTerm->name):("#"))<<endl;
    }*/

    ///需要循环对 新增项进行 闭包运算
    for(itr=tmp.data.begin();itr!=tmp.data.end();itr++) {
        m_ExtItem &eItem = **itr;
        Term *n;
        vector<Term*> after;
        if ( matchClosurePattern( eItem, n, after ) ) {

            if ( eItem.preTerm ) after.push_back( eItem.preTerm ); //NULL就不加入了
            set<Term*> firstResult;
            bfsFirstSet( after, firstResult );

            /*for(vector<Term*>::iterator i=after.begin();i!=after.end();i++) {
                cout<<"   after "<<(*i)->name<<endl;
            }
            for(set<Term*>::iterator i=firstResult.begin();i!=firstResult.end();i++) {
                cout<<"   result "<<(*i)->name<<endl;
            }*/
            if ( firstResult.size() == 0 ) {
                firstResult.insert( NULL );
            }
            if ( itemTable.find( n )!= itemTable.end() ) {
                vector<m_Item*> &y = *itemTable.find(n)->second;
                vector<m_Item*>::iterator jtr;
                for(jtr=y.begin();jtr!=y.end();jtr++) {
                    if ( (*jtr)->point != 0 ) continue;
                    for(set<Term*>::iterator ktr=firstResult.begin();ktr!=firstResult.end();ktr++) {

                        m_ExtItem* p = new m_ExtItem( *jtr, *ktr );
                        ///扔进去的时候 判重
                        sta->data.insert( p );

                   /*     cout<<"  ADD "<<p->item->rule->from->name<<" ";
                        if ( p->preTerm == NULL ) cout<<" preTerm:NULL  ";
                        else cout<<" preTerm:"<<p->preTerm->name<<"  ";
                        p->item->rule->showTerm( p->item->point );
                        cout<<endl;*/
                    }
                }
            }
            ///计算 after的 First集 ,需要写一个 BFS
            /// 把 n Term开头的 构造newExtItem 把itemTable中的 找出来
            ///以上两者乘积 放入sta->data
        }
    }
    /*cout<<" All of Closure"<<endl;
    set<m_ExtItem*,ExtItemPtrCmp>::iterator jtr;
    for(jtr=sta->data.begin();jtr!=sta->data.end();jtr++) {
        m_ExtItem &x = **jtr;
        cout<<"  "<<x.item->rule->from->name<<" => ";
        x.item->rule->showTerm(x.item->point);
        cout<<" char "<<(x.preTerm?x.preTerm->name:"#")<<endl;
    }
    cout<<" Out Closure"<<endl<<endl;*/
    return sta->data.size();
}
LR_Syntax::m_Item* LR_Syntax::itemNextStep(m_Item* s) {

    //itemTable.find(s->from)
    map<Term*, vector<m_Item*>* >::iterator mtr;
    for(mtr=itemTable.begin();mtr!=itemTable.end();mtr++) {
        //vector<m_Item*> &vy = *itemTable.find(s->rule->from)->second;
        vector<m_Item*> &vy = *mtr->second;
        vector<m_Item*>::iterator vtr;
        for(vtr=vy.begin();vtr!=vy.end();vtr++) {
            m_Item &p = **vtr;
            if ( p.rule == s->rule && p.point == s->point+1 )
                return &p;
        }
    }
}


int LR_Syntax::sameIState(m_IState* s) {
    ///按照地址排序，然后依次对比 先Item* 后Term*，不要看ExtItem
    /// 之后再检查闭包有无错误
    set<m_ExtItem*,ExtItemPtrCmp> &v = s->data;
    for(int i=0;i<v_IState.size();i++) {
        set<m_ExtItem*,ExtItemPtrCmp> &cur = v_IState[i]->data;
        if ( v.size() != cur.size() ) continue;
        else {
            bool flag = false;
            set<m_ExtItem*,ExtItemPtrCmp>::iterator j,k;
            for(j=v.begin(),k=cur.begin();j!=v.end() && k!=cur.end();j++,k++) {
                if ( (*j)->item != (*k)->item || (*j)->preTerm != (*k)->preTerm ) {
                    flag = true;
                    break;
                }
            }
            if ( !flag ) return i;
        }
    }
    return 0;
}

bool LR_Syntax::destroyTmpIState(m_IState* sta) {
    set<m_ExtItem*,ExtItemPtrCmp>::iterator itr;
    for(itr=sta->data.begin();itr!=sta->data.end();itr++) {
        m_ExtItem* add = *itr;
        delete add;
    }
    return true;
}

LR_Syntax& LR_Syntax::buildAnalyticalTable() {
///计算I状态封装， 以及 [I*][Term*] => 映射表,边构造映射表边计算新的I
/// map<m_IState*, map<Term*, m_IState*>* > StateTable;

    int curIState = 0;
    v_IState.clear();

    m_Item &begin = *(*itemTable.begin()->second)[0];
    m_ExtItem* _P = new m_ExtItem(&begin, NULL);

    v_IState.push_back( new m_IState(0) );
    v_IState[0]->data.insert( _P );

    //calcClosure( v_IState[0] );
    fullCalcClosure( v_IState[0] );

    while( curIState < v_IState.size() ) {
        cout<<"CUR STEP = "<<curIState<<endl;
        //cin.get();
        map<string,Term*>::iterator itr;
        vector<m_IState*> v_tmp;
        vector<m_IState*> map_X;
        vector<Term*> map_Y;
        for(itr=symbolTable.begin();itr!=symbolTable.end();itr++) {
            set<m_ExtItem*,ExtItemPtrCmp>::iterator jtr;
            Term &y = *itr->second;
            ///遍历IState中每个ExtItem
            for(jtr=v_IState[ curIState ]->data.begin(); jtr!=v_IState[ curIState ]->data.end();jtr++) {
                m_Item& vItem = *(*jtr)->item;
                ///终止则跳过
                if ( vItem.point >= vItem.rule->term.size() ) continue;
                ///下一个 项 等于 遍历项
                if ( vItem.rule->term[ vItem.point ] == &y ) {

                    cout<<" X-Term "<<y.name<<endl;
                    cout<<" Trans Rule "<<vItem.rule->from->name<<" => ";
                    vItem.rule->showTerm(vItem.point);
                    cout<<((*jtr)->preTerm?(*jtr)->preTerm->name:"#")<<endl;


                    if ( StateTable.find(v_IState[curIState]) == StateTable.end() ) {
                        StateTable.insert( pair<m_IState*,map<Term*, m_IState*>* >( v_IState[curIState], new map<Term*, m_IState*> ) );
                    }
                    map<Term*, m_IState*>& tiMap = *StateTable.find(v_IState[curIState])->second;
                    //cout<<"NUM"<<v_IState.size()<<endl;
                    bool isNewState = false;
                    if ( tiMap.find(&y) == tiMap.end() ) {
                        isNewState = true;
                        tiMap.insert( pair<Term*, m_IState*>( &y, new m_IState( v_IState.size() ) ) );
                    }
                    m_IState& newIState = *tiMap.find(&y)->second;
                    ///扔进去的时候 判重
                    m_ExtItem* newExt = new m_ExtItem( itemNextStep(&vItem), (*jtr)->preTerm);
                    newIState.data.insert( newExt );

                    ///不能在这里算闭包，先全部扔到 临时v_ISTATE_tmp 中,判定完重复后再扔回去
                    ///先看下 newIState 的data与之前有无 一致的，一致则重复
                    ///先算完 闭包再那啥
                    if ( isNewState ) {
                        v_tmp.push_back( &newIState );
                        map_X.push_back( v_IState[curIState] );
                        map_Y.push_back( &y );
                    }

                }
            }
        }

        /// 从 cur 到 size 计算闭包,在这里算闭包，如果遇到状态呵呵的 需要扔掉
        ///for(int i=curIState+1;i<v_IState.size();i++)
            ///calcClosure( v_IState[i] );
        ///if ( !sameIState( &newIState ) )
                     ///   v_IState.push_back( &newIState );
        for(int i=0;i<v_tmp.size();i++) {
            v_tmp[i]->stateID = v_IState.size();
            //calcClosure( v_tmp[i] );
            fullCalcClosure( v_tmp[i] );
            //stateVectorSort(v_tmp[i]);
                int notsame;
            ///注 ：如果和 0 状态same的话，理论上应该是 产生式写错，所以先不考虑这种情况
            if ( !(notsame = sameIState( v_tmp[i] )) ) {
                v_IState.push_back( v_tmp[i] );
                cout<<"  not same "<<v_tmp[i]->stateID<<endl;
                ///
            }
            else {
                destroyTmpIState(v_tmp[i]);
                /// 设置映射关系
                StateTable.find(map_X[i])->second->find(map_Y[i])->second = v_IState[notsame];
                cout<<"  same to "<<notsame<<endl;
                ///删除 这个state
            }
        }
        ++curIState;
    }
    ///使用vector 记录IState 编号，记录当前执行编号，和整体数目
    /// while 执行编号< size
    /// 先把 map<string,Term*> symbolTable;
    /// 然后 按照 m_Item point 匹配 下一个 是否相同，相同则 加入 StateTable(不存在new，存在find),new 了之后记得马上加入vector<I*>

    return *this;
}

LR_Syntax& LR_Syntax::showIState(int index) {
    if ( index >= 0 && index < v_IState.size() ) {
        set<m_ExtItem*,ExtItemPtrCmp> &st = v_IState[index]->data;
        set<m_ExtItem*,ExtItemPtrCmp>::iterator jtr;
        for(jtr=st.begin();jtr!=st.end();jtr++) {
            m_ExtItem& eItem = **jtr;
            cout<<" "<<eItem.item->rule->from->name<<"  char[ "<<(eItem.preTerm?eItem.preTerm->name:"#")<<" ]"<<endl<<"  ";
            eItem.item->rule->showTerm( eItem.item->point );
            cout<<endl;
        }
        cout<<endl;
        return *this;
    }

    vector<m_IState*>::iterator itr;
    for(itr=v_IState.begin();itr!=v_IState.end();itr++) {
        m_IState& sta = **itr;
        set<m_ExtItem*,ExtItemPtrCmp> &st = sta.data;
        set<m_ExtItem*,ExtItemPtrCmp>::iterator jtr;

        cout<<"IState ID "<<sta.stateID<<" :"<<endl;
        for(jtr=st.begin();jtr!=st.end();jtr++) {
            m_ExtItem& eItem = **jtr;
            cout<<" "<<eItem.item->rule->from->name<<"  char[ "<<(eItem.preTerm?eItem.preTerm->name:"#")<<" ]"<<endl<<"  ";
            eItem.item->rule->showTerm( eItem.item->point );
            cout<<endl;
        }
        cout<<endl;
    }

    return *this;
}

LR_Syntax::m_IState* LR_Syntax::ATableSearch(m_IState* sta, Term* term) {

    if ( StateTable.find(sta) == StateTable.end() ) {
        return NULL;
    }
    else {
        map<Term*, m_IState*>& tiMap = *StateTable.find(sta)->second;
        if ( tiMap.find(term) != tiMap.end() ) {
            return tiMap.find(term)->second;
        }
        else {
            return NULL;
        }
    }
}

LR_Syntax& LR_Syntax::buildActionGotoTable() {
    ///先对 Term* 遍历，需要添加#
    ///从 v_IState 0开始遍历, 得到m_IState* 找指针去 看 StateTable中 关于 对应Term*的 状态映射
    ///如果 IState 的话， 如果Term* 是 non-terminal， 更新到 Gto状态，m_IState*
    ///如果是 terminal的话，更新到 Step状态 ，Term*
    ///如果 被遍历的IState中 存在 .在结束位置的项，在对应上面加上 recur标记，冲突则指示出来
    ///如果是v[1] 对 # 加入  ACC状态


    map<string,Term*> symbolWithNull( symbolTable );
    symbolWithNull.insert( pair<string,Term*>("",NULL) );

    map<string,Term*>::iterator itr;
    for(itr=symbolWithNull.begin();itr!=symbolWithNull.end();itr++) {
        int stateIndex = 0;
        Term *termPtr = itr->second;
        for(stateIndex=0;stateIndex<v_IState.size();stateIndex++) {
            m_IState* curState = v_IState[stateIndex];
            m_IState* found;
            ///如果找到 或者 Term*#都进去
            //if ( termPtr != NULL  ) {
            if ( ActionTable.find(curState) == ActionTable.end() ) {
                ActionTable.insert( pair<m_IState*,map<Term*, m_Action*>* >( curState, new map<Term*, m_Action*> ) );
            }
            map<Term*, m_Action*> &acMap = *ActionTable.find(curState)->second;
            if ( acMap.find(termPtr) == acMap.end() ) {
                acMap.insert( pair<Term*, m_Action*>( termPtr, new m_Action ) );
            }
            m_Action &action = *acMap.find(termPtr)->second;
            found = ATableSearch( curState, termPtr );
            if ( termPtr && termPtr->terminal && found ) {
                /// step
                action.type = Step;
                action._step = found;
            }
            else if ( termPtr && !termPtr->terminal && found ) {
                /// gto
                action.type = Gto;
                action._gto = found;
            }
            else {
                action.type = Err;
            }
        }
    }
    ///最后开始添加 Recur 规约规则
    int stateIndex = 0;
    for(stateIndex=0;stateIndex<v_IState.size();stateIndex++) {
        m_IState* curState = v_IState[stateIndex];
        cout<<"State ID "<<curState->stateID<<endl;
        ///迭代data
        set<m_ExtItem*,ExtItemPtrCmp>::iterator itr;
        bool flag = false;
        for(itr=curState->data.begin();itr!=curState->data.end();itr++) {
            m_ExtItem &eItem = **itr;
            Term *termPtr = eItem.preTerm;
            if ( eItem.item->point != eItem.item->rule->term.size() ) continue;
            if ( ActionTable.find(curState) == ActionTable.end() ) {
                cout<<" ERR:"<<" Recur State Not Found "<<endl;
                continue;
                //ActionTable.insert( pair<m_IState*,map<Term*, m_Action*>* >( curState, new map<Term*, m_Action*> ) );
            }
            map<Term*, m_Action*> &acMap = *ActionTable.find(curState)->second;
            if ( acMap.find(termPtr) == acMap.end() ) {
                if ( termPtr ) {
                    cout<<" ERR:"<<" Recur Term Not Found ";
                    cout<<( termPtr?termPtr->name:"#" )<<endl;
                    continue;
                }
                else {
                    acMap.insert( pair<Term*, m_Action*>( termPtr, new m_Action ) );
                }
                //
            }
            m_Action &action = *acMap.find(termPtr)->second;
            if ( action.type != Err ) {
                    flag = true;
                cout<<" ERR:"<<" Recur Action Conflict "<< action.type<<endl;
                if ( action.type == Step ) {
                    cout<<"   TransTo"<<action._step->stateID<<endl;
                    cout<<"  Auto-Set Action.preTerm ["<<(termPtr?termPtr->name:"#")<<"] to [#]"<<endl;
                    autoSolveConflict( acMap, NULL, eItem );
                }
            }
            else {
                action.type = Recur;
                action._recur = eItem.item->rule;
            }
        }
        if ( flag ) {
            ///打出该 State 闭包
            showIState(curState->stateID);
        }

    }

    return *this;
}

LR_Syntax& LR_Syntax::showAGTable() {

    map<string,Term*>::iterator itr;
    map<string,Term*> symbolWithNull( symbolTable );
    symbolWithNull.insert( pair<string,Term*>("",NULL) );

    cout<<"\t";
    for(itr=symbolWithNull.begin();itr!=symbolWithNull.end();itr++) {

        Term *termPtr = itr->second;
        cout<<"\t"<<(termPtr?termPtr->name:"#");
    }
    cout<<endl;

    int stateIndex = 0;
    for(stateIndex=0;stateIndex<v_IState.size();stateIndex++) {
        m_IState* curState = v_IState[stateIndex];

        cout<<"sta-"<<curState->stateID<<"\t";
        for(itr=symbolWithNull.begin();itr!=symbolWithNull.end();itr++) {
            Term *termPtr = itr->second;
            m_Action* action = ActionTableSearch(curState, termPtr);

            if ( action == NULL ) {
                cout<<"\t";
            }
            else {
                switch(action->type) {
                case Err:
                    cout<<" \t";
                    break;
                case Acc:
                    cout<<"Acc\t";
                    break;
                case Step:
                    cout<<"S"<<action->_step->stateID<<"\t";
                    break;
                case Recur:
                    cout<<"R"<<action->_recur->ruleID<<"\t";
                    break;
                case Gto:
                    cout<<"G"<<action->_gto->stateID<<"\t";
                    break;
                default:
                    cout<<"\t";
                    break;
                }

            }

        }
        cout<<endl;
    }


    return *this;
}

void LR_Syntax::autoSolveConflict( map<Term*, m_Action*> & acMap, Term* termPtr, m_ExtItem& eItem ) {
    if ( acMap.find(termPtr) == acMap.end() ) {
        acMap.insert( pair<Term*, m_Action*>( termPtr, new m_Action ) );
    }
    m_Action &action = *acMap.find(NULL)->second;
    action.type = Recur;
    action._recur = eItem.item->rule;
}

LR_Syntax& LR_Syntax::initLEXSymbol(string filename) {
    if ( lexFile ) fclose(lexFile);
    lexFile = fopen(filename.c_str(), "r+");
    char buf[2][100];
    int typeId;
    string termKey, extraVal;
    while(3==fscanf(lexFile, "< %s , %d , %s >\n",buf[0],&typeId,buf[1])) {
        termKey = buf[0];
        if ( symbolTable.find(termKey + "_t") == symbolTable.end() ) {
            cout<<" Can't Find "<<termKey<<endl;
            continue;
        }
        Term* t = symbolTable.find(termKey+ "_t")->second;
        queueLex.push( new m_LexGroup( typeId, buf[1], t ) );
    }
    cout<<endl<<" LEX Input "<<queueLex.size()<<endl;

    return *this;
}

LR_Syntax::m_Action* LR_Syntax::ActionTableSearch( m_IState* s, Term* t) {
    //map<m_IState*, map<Term*, m_Action*>* >
    if ( ActionTable.find( s ) == ActionTable.end() ) {
        return NULL;
    }
    else {
        map<Term*, m_Action*> &y = *ActionTable.find(s)->second;
        if ( y.find(t) == y.end() ) {
            return NULL;
        }
        else {
            return y.find(t)->second;
        }
    }
}

LR_Syntax& LR_Syntax::runSyntaxAutoman() {
///TODO:    自动分析器 开始

    ///需要 stepCnt, 符号栈, 字符串, 动作, goto
    stack<int> stateStack;
    stack<Term*> termStack;
    int stepCnt=1;
    int stateTop=0;
    Term* termTop=NULL;

    stateStack.push(0);
    termStack.push(NULL);
    while ( !stateStack.empty() && !termStack.empty() ) {
        stateTop = stateStack.top();
        termTop = termStack.top();
        m_IState* curState = v_IState[ stateTop ];
        m_Action* action = ActionTableSearch( curState, termTop );
        if ( action == NULL ) {
            cout<<" Can't Find Action "<<curState->stateID<<" "<<termTop<<endl;
        }
        else {
            switch(action->type) {
            case Err:
                cout<<" Action ERR";
                ///遇到错误，然后考虑怎么跳错
                break;
            case Step:
                ///移近,
                stateStack.push( action->_step->stateID );
                termStack.push( termTop );
                break;
            case Recur:

                break;
            case Gto:

                break;
            case Acc:

                break;
            default:break;
            }
        }



    }
    return *this;
}

/*书上7.4样例测试AC
LR_Syntax::Rule m_Syntax_Rule[] = {
    {"_S","S"},{"S","a|A|d"},{"S","b|A|c"},{"S","a|e|c"},{"S","b|e|d"},{"A","e"}
};

LR_Syntax::Term m_Syntax_VT[] = {
    {"a",true},{"b",true},{"c",true},{"d",true},{"e",true},
};

LR_Syntax::Term m_Syntax_VN[] = {
    {"_S",false},{"S",false},{"A",false},
};*/

/*书上p146
LR_Syntax::Rule m_Syntax_Rule[] = {
    {"_S","S"},{"S","B|B"},{"B","a|B"},{"B","b"}
};

LR_Syntax::Term m_Syntax_VT[] = {
    {"a",true},{"b",true}
};

LR_Syntax::Term m_Syntax_VN[] = {
    {"_S",false},{"S",false},{"B",false},
};*/

/**/
LR_Syntax::Rule m_Syntax_Rule[] = {
    {"_P","P"},
    {"P","{|D|S|}"},
    {"D","D|int|ID|;"},{"D","int|ID|;"},
    {"S","ST"},
    {"ST","MST"},{"ST","OST"},
    {"MST","if|(|BEp|)|thn|MST|els|MST"},
    {"OST","if|(|BEp|)|thn|ST"},{"OST","if|(|BEp|)|thn|MST|els|OST"},
    // if-then-else SOLVE http://blog.csdn.net/alwaysslh/article/details/4157348
    //{"S","if|(|BEp|)|then|S|else|S"},{"S","if|(|BEp|)|then|S"},
    {"S","whl|(|BEp|)|do|S"},{"S","ID|=|CEp"},{"S","{|CS|}"},
    {"CS","S|;|CS"},{"CS","S"},
    {"BEp","BEp|and|BEp"},{"BEp","BEp|or|BEp"},{"BEp","ID|rlp|ID"},{"BEp","ID"},
    {"CEp","CEp|+|CEp"},{"CEp","CEp|-|CEp"},{"CEp","CEp|*|CEp"},{"CEp","CEp|/|CEp"},{"CEp","(|CEp|)"},{"CEp","ID"},{"CEp","NUM"},
  //  {"relop","<"},{"relop",">"},{"relop","<="},{"relop",">="},{"relop","!="},{"relop","=="}
};

LR_Syntax::Term m_Syntax_VT[] = {
    {"int",true,1},{"if",true,2},{"thn",true,3},{"els",true,4},{"whl",true,5},{"do",true,6},
    {"ID",true,7,true,},{"NUM",true,8,true},
    {"+",true,9},{"-",true,10},{"*",true,11},{"/",true,12},{"and",true,13},{"or",true,14},
   // {"<",true},{">",true},{"<=",true},{">=",true},{"!=",true},{"==",true},
    {"{",true,16},{"}",true,17},{";",true,18},{"(",true,19},{")",true,20},{"=",true,21},{"rlp",true,15,true}
};

LR_Syntax::Term m_Syntax_VN[] = {
    {"_P",false},{"P",false},{"D",false},{"S",false},{"BEp",false},{"CEp",false},{"CS",false}
    //,{"relop",false}
    ,{"ST",false}
    ,{"MST",false}
    ,{"OST",false}
};

vector<LR_Syntax::Rule> v_Syntax_Rule( m_Syntax_Rule, m_Syntax_Rule+sizeof(m_Syntax_Rule)/sizeof(m_Syntax_Rule[0]) );
vector<LR_Syntax::Term> v_Syntax_VN( m_Syntax_VN, m_Syntax_VN+sizeof(m_Syntax_VN)/sizeof(m_Syntax_VN[0]) );
vector<LR_Syntax::Term> v_Syntax_VT( m_Syntax_VT, m_Syntax_VT+sizeof(m_Syntax_VT)/sizeof(m_Syntax_VT[0]) );

int main() {
    freopen("out.txt","w",stdout);

    LR_Syntax lr;
    lr.
      initSymbol(v_Syntax_VN).
      initSymbol(v_Syntax_VT).
      initProduction(v_Syntax_Rule).
      //showProduction().
      buildItems().
      buildAnalyticalTable().
      showIState().
      buildActionGotoTable().
      showAGTable();
      //initLEXSymbol("in.txt");
      //runSyntaxAutoman();
    return 0;
}
