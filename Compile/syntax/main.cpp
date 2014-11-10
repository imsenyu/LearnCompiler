/**
 * @fileoverview syntax
 * @author 郁森<senyu@mail.dlut.edu.cn>/一杉<yusen.ys@alibaba-inc.com>
 * @language C++
 * @description Compiler-基于LL(0)的简易语法分析器实现
 */
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <map>
#include <set>
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
        };
    public:
        struct Production {
            int from;
        };
        class m_Rule {
            public:
                vector<Term*> term;
                Term* from;
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

        class haha
        {
            public:
            bool operator() (m_ExtItem* a, m_ExtItem* b)
            {
                return a->item == b->item ? a->preTerm<b->preTerm : a->item<b->item;
            }
        };
        struct m_IState {
            set<m_ExtItem*,haha> data;
            int stateID;
            m_IState(int a):stateID(a) {}
        };
    public:
        LR_Syntax():ruleDelim("|") {}
        LR_Syntax& initSymbol( vector<LR_Syntax::Term>& );
        LR_Syntax& initProduction( vector<LR_Syntax::Rule>& ); //vector 导入 map
        LR_Syntax& showProduction();
        LR_Syntax& buildItems();
        LR_Syntax& buildAnalyticalTable();
    private:
        int calcClosure(m_IState* );
        bool matchClosureRule(m_ExtItem&,Term*&,vector<Term*>&);
    private:
        string ruleDelim;
        map<string,Term*> symbolTable;
        map<Term*, vector<m_Rule*>* > productionTable;
        map<Term*, vector<m_Item*>* > itemTable;   //LR项目表
        ///计算I状态封装， 以及 [I*][Term*] => 映射表,边构造映射表边计算新的I
        vector<m_IState*> v_IState;
        map<m_IState*, map<Term*, m_IState*>* > ATable;
        int bfsFirstSet( vector<Term*>&, set<Term*>& );
        m_Item* itemNextStep(m_Item*);
        bool sameIState(m_IState*);
        void stateVectorSort(m_IState*);
        static int stateVectorSort_cmp( m_ExtItem*, m_ExtItem* );
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
    cout<<"m_Rule:"<<s<<ends;
    for(vector<Term*>::iterator itr=term.begin(); itr!=term.end(); itr++) {
        Term &p = **itr;
        cout<<" "<<p.name<<" "<<p.terminal<<endl;
    }
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
    for(vector<LR_Syntax::Rule>::iterator itr=r.begin(); itr!=r.end(); itr++) {
        Rule &p = *itr;
        m_Rule *address = new m_Rule(p.value, symbolTable, ruleDelim);
        if ( symbolTable.find( p.source + "_n" ) != symbolTable.end() ) {
            Term* sym = symbolTable.find( p.source + "_n" )->second;
            cout<<" From Symbol["<<sym->name<<"] "<<sym->terminal<<endl;
            address->from = sym;

            if ( productionTable.find(sym) == productionTable.end() ) {
                vector<m_Rule*>* container = new vector<m_Rule*>;
                container->push_back( address );
                productionTable.insert( pair<Term*,vector<m_Rule*>* >( sym, container ) );
            }
            else {
                productionTable.find(sym)->second->push_back( address );
            }
        } else
            cout<<"ERROR...Can't Find LeftTerm["<<p.source<<"] in SymbolTable"<<endl;
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

bool LR_Syntax::matchClosureRule(m_ExtItem& eI,Term* & N,vector<Term*> & after) {
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

int LR_Syntax::calcClosure(m_IState* sta) {
    ///其中任何一个 ExItem 若有 .Nb, a 遍历 B 的Items ，计算 First( ba )
    /// 把 对应 B的Item,b 加入
    cout<<endl<<" In Closure"<<endl;
    set<m_ExtItem*,haha>::iterator itr;
    m_IState tmp(*sta);

    //cout<<"  given IState ";
    cout<<"  ID = "<<tmp.stateID<<endl;
    /*for(int i=0;i<tmp.data.size();i++) {
        cout<<"  "<<tmp.data[i]->item->rule->from->name<<" => ";
        tmp.data[i]->item->rule->showTerm(tmp.data[i]->item->point),
        cout<<" char "<<((tmp.data[i]->preTerm)?(tmp.data[i]->preTerm->name):("#"))<<endl;
    }*/


    for(itr=tmp.data.begin();itr!=tmp.data.end();itr++) {
        m_ExtItem &eItem = **itr;
        Term *n;
        vector<Term*> after;
        if ( matchClosureRule( eItem, n, after ) ) {

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
    cout<<" All of Closure"<<endl;
    set<m_ExtItem*,haha>::iterator jtr;
    for(jtr=sta->data.begin();jtr!=sta->data.end();jtr++) {
        m_ExtItem &x = **jtr;
        cout<<"  "<<x.item->rule->from->name<<" => ";
        x.item->rule->showTerm(x.item->point);
        cout<<" char "<<(x.preTerm?x.preTerm->name:"#")<<endl;
    }
    cout<<" Out Closure"<<endl<<endl;
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

void LR_Syntax::stateVectorSort(m_IState* sta) {
    set<m_ExtItem*,haha> &v = sta->data;
    //sort(v.begin(),v.end(),LR_Syntax::stateVectorSort_cmp);
    return;
}

int LR_Syntax::stateVectorSort_cmp( m_ExtItem* a, m_ExtItem* b ) {
    m_ExtItem &x = *a, &y = *b;
    if ( x.item == y.item ) {
        return x.preTerm < y.preTerm;
    }
    else return x.item < y.item;
}

bool LR_Syntax::sameIState(m_IState* s) {
    ///按照地址排序，然后依次对比 先Item* 后Term*，不要看ExtItem
    /// 之后再检查闭包有无错误
    set<m_ExtItem*,haha> &v = s->data;
    for(int i=0;i<v_IState.size();i++) {
        set<m_ExtItem*,haha> &cur = v_IState[i]->data;
        if ( v.size() != cur.size() ) continue;
        else {
            bool flag = false;
            set<m_ExtItem*,haha>::iterator j,k;
            for(j=v.begin(),k=cur.begin();j!=v.end() && k!=cur.end();j++,k++) {
                if ( (*j)->item != (*k)->item || (*j)->preTerm != (*k)->preTerm ) {
                    flag = true;
                    break;
                }
            }
            if ( !flag ) return true;
        }
    }
    return false;
}

LR_Syntax& LR_Syntax::buildAnalyticalTable() {
///计算I状态封装， 以及 [I*][Term*] => 映射表,边构造映射表边计算新的I
/// map<m_IState*, map<Term*, m_IState*>* > ATable;

    int curIState = 0;
    v_IState.clear();

    m_Item &begin = *(*itemTable.begin()->second)[0];
    m_ExtItem* _P = new m_ExtItem(&begin, NULL);

    v_IState.push_back( new m_IState(0) );
    v_IState[0]->data.insert( _P );

    calcClosure( v_IState[0] );

    while( curIState < v_IState.size() ) {
        cout<<"CUR STEP = "<<curIState<<endl;
        //cin.get();
        map<string,Term*>::iterator itr;
        vector<m_IState*> v_tmp;
        for(itr=symbolTable.begin();itr!=symbolTable.end();itr++) {
            set<m_ExtItem*,haha>::iterator jtr;
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


                    if ( ATable.find(v_IState[curIState]) == ATable.end() ) {
                        ATable.insert( pair<m_IState*,map<Term*, m_IState*>* >( v_IState[curIState], new map<Term*, m_IState*> ) );
                    }
                    map<Term*, m_IState*>& tiMap = *ATable.find(v_IState[curIState])->second;
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
            calcClosure( v_tmp[i] );
            //stateVectorSort(v_tmp[i]);
            if ( !sameIState( v_tmp[i] ) ) {
                v_IState.push_back( v_tmp[i] );
            }
            else {
                ///删除 这个state
            }
        }
        ++curIState;
    }
    ///使用vector 记录IState 编号，记录当前执行编号，和整体数目
    /// while 执行编号< size
    /// 先把 map<string,Term*> symbolTable;
    /// 然后 按照 m_Item point 匹配 下一个 是否相同，相同则 加入 ATable(不存在new，存在find),new 了之后记得马上加入vector<I*>

    return *this;
}

LR_Syntax::Rule m_Syntax_Rule[] = {
    {"_P","P"},
    {"P","{|D|S|}"},
    {"D","D|int|ID|;"},{"D","int|ID|;"},
    {"S","if|(|BoolExp|)|then|S|else|S"},{"S","if|(|BoolExp|)|then|S"},{"S","while|(|BoolExp|)|do|S"},{"S","ID|=|CalcExp"},{"S","{|CpxS|}"},
    {"CpxS","S|;|CpxS"},{"CpxS","S"},
    {"BoolExp","BoolExp|and|BoolExp"},{"BoolExp","BoolExp|or|BoolExp"},{"BoolExp","ID|relop|ID"},{"BoolExp","ID"},
    {"CalcExp","CalcExp|+|CalcExp"},{"CalcExp","CalcExp|-|CalcExp"},{"CalcExp","CalcExp|*|CalcExp"},{"CalcExp","CalcExp|/|CalcExp"},{"CalcExp","(|CalcExp|)"},{"CalcExp","ID"},{"CalcExp","NUM"},
    {"relop","<"},{"relop",">"},{"relop","<="},{"relop",">="},{"relop","!="},{"relop","=="}
};

LR_Syntax::Term m_Syntax_VN[] = {
    {"int",true},{"if",true},{"then",true},{"else",true},{"while",true},{"do",true},
    {"ID",true},{"NUM",true},
    {"+",true},{"-",true},{"*",true},{"/",true},{"and",true},{"or",true},
    {"<",true},{">",true},{"<=",true},{">=",true},{"!=",true},{"==",true},
    {"{",true},{"}",true},{";",true},{"(",true},{")",true},{"=",true}
};

LR_Syntax::Term m_Syntax_VT[] = {
    {"_P",false},{"P",false},{"D",false},{"S",false},{"BoolExp",false},{"CalcExp",false},{"CpxS",false},{"relop",false}
};
vector<LR_Syntax::Rule> v_Syntax_Rule( m_Syntax_Rule, m_Syntax_Rule+sizeof(m_Syntax_Rule)/sizeof(m_Syntax_Rule[0]) );
vector<LR_Syntax::Term> v_Syntax_VN( m_Syntax_VN, m_Syntax_VN+sizeof(m_Syntax_VN)/sizeof(m_Syntax_VN[0]) );
vector<LR_Syntax::Term> v_Syntax_VT( m_Syntax_VT, m_Syntax_VT+sizeof(m_Syntax_VT)/sizeof(m_Syntax_VT[0]) );

int main() {
    LR_Syntax lr;
    lr.
      initSymbol(v_Syntax_VN).
      initSymbol(v_Syntax_VT).
      initProduction(v_Syntax_Rule).
      //showProduction().
      buildItems().
      buildAnalyticalTable();
    return 0;
}
