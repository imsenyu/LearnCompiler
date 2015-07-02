#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include "stdafx.h"
#include "clUtils.h"
#include "term.h"
#include "production.h"
#include "stateItem.h"
#include "stateExtItem.h"
#include "stateSet.h"
#include "action.h"
#include "syntaxTree.h"

class syntaxParser;

class syntaxParser {
private:
    template<class _T, class _KEY>
    auto HashFind(_T container, _KEY key)->decltype(container.find( key )->second) {
        typename _T::iterator iter = container.find( key );
        if ( iter != container.end() ) {
            return iter->second;
        }
        else return NULL;
    }
public:
    bool isDebug;
public:
    ///所有数据存储使用vector
    vector<Term*> vecTerm;
    vector<Production*> vecATerm;
    vector<StateSet*> vecStates;

    ///供数据查找
    map<string, Term*> mpTerm; ///TermMap
    map<string, vector<Production*>> mpATerm; ///ProductionMap
   // map<Production*, vector<StateItem*>> mpSItems; ///存储兼查找
    D2Map<int,Term*,int> mpStateTable;/// stateset - ATerm* 的二维查找表
    ActionGotoTable* ptrAGTable;
    syntaxNode* ptrSyntaxTreeRoot;
    //map<int, map<Term*, int>> mpStateTable;/// stateset - ATerm* 的二维查找表
    ///供标号，delete

    //map<int, map<Production*
    ///供顺序读取
    vector<Token*> vecToken;



    syntaxParser(bool _debug = true):isDebug(_debug), ptrAGTable(NULL), ptrSyntaxTreeRoot(NULL) { }
    inline void nop() {}

    syntaxParser& inputTerm(istream& in);
    syntaxParser& showTerm();

    syntaxParser& inputProduction(istream& in);
    syntaxParser& showProduction();

    syntaxParser& buildStateItems();
    syntaxParser& showStateItems();

    syntaxParser& buildStateSet();
    syntaxParser& showStateSet();

    syntaxParser& buildActionGotoTable();
    syntaxParser& showActionGotoTable();

    syntaxParser& inputLex(istream& in);
    syntaxParser& runSyntaxAnalyse();
    syntaxParser& showSyntaxTree();

    void dfs(syntaxNode* root, function<bool(syntaxNode* root, int pos)> *func) {
        deque<syntaxNode*>& child = root->child;
        if ( NULL == root->ptrPdt ) return;
        int pId = root->ptrPdt->pId;
        function<bool(syntaxNode* root, int pos)>& f = func[pId];

        f(root, -1);
        for(int c=0;c<child.size();c++) {
            dfs(child[c], func);
            f(root, c);
        }

    }

    int getLineNum(bool start = false, bool _plus = true) {
        static int line = 1;
        if ( true == start ) line = 1;
        if ( _plus ) return line++;
        else return line;
    }

    int getNewTmp(bool start = false) {
        static int cnt = 1;
        if ( true == start ) cnt = 1;
        return cnt++;
    }

    ///虚函数扩展，可自行定义，然后根据自行定义执行完整的翻译
    virtual syntaxParser& syntaxDirectedTranslation() {
        ///根据数组中每个函数的定义
        /*
_S 1 S
S 1 E
E 1 T
E 3 E + T
T 1 P
T 3 T * P
P 1 NUM
P 3 ( E )
         */
        const string strValue("value"), strPlace("place");
        function<bool(syntaxNode* root, int pos)> transFuncArr[] = {
            /* 0 _S->S */
            [&](syntaxNode* root, int pos){ /*do nothing*/ return true; },
            /* 1 S-> E */
            [&](syntaxNode* root, int pos){
                switch(pos) {
                    case -1: {
                        root->hashData.add( strPlace, new int(getNewTmp()) );
                    } break;
                    case 0: {
                        /*
                         * 需要：1. 查找符号表; 2.加入符号表; 3.获得指定元素 最好,Item(k)-> 4. 方便的设置指定元素 hash值和获取， 对获取的数据快速类型转换
                         */
                        /*
                        _LookUp("str") -> Addr
                        _AddVar("str") -> bool 是否成功
                        _GetItem(root,pos) -> syntaxNode*
                        _SetData( item, "hashStr", data )
                        _GetData( item, "hashStr" )
                        _GenCode( line, 'op', '1' ,'2', 'dest' );
                        ///需要做的 把 E的value设置给 S的value
                        */


                        syntaxNode* target = root->child[pos];
                        assert(target);
                        assert( target->hashData.has(strValue) );
                        int* ptrValFromE = *(int**)target->hashData.get(strValue);
                        assert(ptrValFromE);
                        root->hashData.add(strValue, new int(*ptrValFromE));
                        printf("L-%d\t",getLineNum());
                        printf("t%d = t%d\n", *(int*)*root->hashData.get(strPlace), *(int*)*target->hashData.get(strPlace));
                        printf("Need t%d\n", *(int*)*root->hashData.get(strPlace));
                    } break;
                }
                return true;
            },
            /* 2 E-> T */
            [&](syntaxNode* root, int pos){
                switch(pos) {
                    case -1: {
                        //root->hashData.add( strPlace, new int(getNewTmp()) );
                    } break;
                    case 0: {
                        syntaxNode* target = root->child[pos];
                        assert(target);
                        assert( target->hashData.has(strValue) );
                        int* ptrValFromT = *(int**)target->hashData.get(strValue);
                        assert(ptrValFromT);
                        root->hashData.add(strValue, new int(*ptrValFromT));
                        //printf("L-%d\t",getLineNum());
                        root->hashData.add( strPlace, new int(*(int*)*target->hashData.get(strPlace)) );
                        //printf("t%d = t%d\n",*(int*)*root->hashData.get(strPlace), *(int*)*target->hashData.get(strPlace));
                    } break;
                }
                return true;
            },
            /* 3 E-> E + T */
            [&](syntaxNode* root, int pos){
                switch(pos) {
                    case -1: {
                        root->hashData.add( strPlace, new int(getNewTmp()) );
                    } break;
                    case 1: break;
                    case 0: {
                        syntaxNode* target = root->child[pos];
                        assert(target);
                        assert( target->hashData.has(strValue) );
                        int* ptrValFromE = *(int**)target->hashData.get(strValue);
                        assert(ptrValFromE);
                        root->hashData.add(strValue, new int(*ptrValFromE));
                    } break;
                    case 2: {
                        syntaxNode* target0 = root->child[0];
                        syntaxNode* target = root->child[pos];
                        assert(target0);
                        assert(target);
                        assert( target->hashData.has(strValue) );
                        int* ptrValFromT = *(int**)target->hashData.get(strValue);
                        assert(ptrValFromT);
                        root->hashData.add(strValue, new int(*ptrValFromT));
                        printf("L-%d\t",getLineNum());
                        printf("t%d = t%d + t%d\n",*(int*)*root->hashData.get(strPlace), *(int*)*target0->hashData.get(strPlace), *(int*)*target->hashData.get(strPlace));
                    } break;
                }
                return true;
            },
            /* 4 T-> P */
            [&](syntaxNode* root, int pos){
                switch(pos) {
                    case -1: {
                        //root->hashData.add( strPlace, new int(getNewTmp()) );
                    } break;
                    case 0: {
                        syntaxNode* target = root->child[pos];
                        assert(target);
                        assert( target->hashData.has(strValue) );
                        int* ptrValFromP = *(int**)target->hashData.get(strValue);
                        assert(ptrValFromP);
                        root->hashData.add(strValue, new int(*ptrValFromP));
                        //printf("L-%d\t",getLineNum());
                        root->hashData.add( strPlace, new int(*(int*)*target->hashData.get(strPlace)) );
                        //printf("t%d = t%d\n", *(int*)*root->hashData.get(strPlace), *(int*)*target->hashData.get(strPlace));
                    } break;
                }
                return true;
            },
            /* 5 T-> T + P */
            [&](syntaxNode* root, int pos){
                switch(pos) {
                    case -1: {
                        root->hashData.add( strPlace, new int(getNewTmp()) );
                    } break;
                    case 1: break;
                    case 0: {
                        syntaxNode* target = root->child[pos];
                        assert(target);
                        assert( target->hashData.has(strValue) );
                        int* ptrValFromT = *(int**)target->hashData.get(strValue);
                        assert(ptrValFromT);
                        root->hashData.add(strValue, new int(*ptrValFromT));
                    } break;
                    case 2: {
                        syntaxNode* target0 = root->child[0];
                        syntaxNode* target = root->child[pos];
                        assert(target0);
                        assert(target);
                        assert( target->hashData.has(strValue) );
                        int* ptrValFromP = *(int**)target->hashData.get(strValue);
                        assert(ptrValFromP);
                        root->hashData.add(strValue, new int(*ptrValFromP));
                        printf("L-%d\t",getLineNum());
                        printf("t%d = t%d + t%d\n", *(int*)*root->hashData.get(strPlace), *(int*)*target0->hashData.get(strPlace), *(int*)*target->hashData.get(strPlace));
                    } break;
                }
                return true;
            },
            /* 6 P-> NUM */
            [&](syntaxNode* root, int pos){
                switch(pos) {
                    case -1: {
                        root->hashData.add( strPlace, new int(getNewTmp()) );
                    } break;
                    case 0: {
                        syntaxNode* target = root->child[pos];
                        assert(target);
                        int valFromNum = clUtils::atoi( target->getLex() );
                        root->hashData.add(strValue, new int(valFromNum));
                        printf("L-%d\t",getLineNum());
                        printf("t%d = [%d]\n", *(int*)*root->hashData.get(strPlace), *(int*)*root->hashData.get(strValue));
                    } break;
                }
                return true;
            },
            /* 7 P-> ( E ) */
            [&](syntaxNode* root, int pos){
                switch(pos) {
                    case -1: {
                        //root->hashData.add( strPlace, new int(getNewTmp()) );
                    } break;
                    case 0: case 2: break;
                    case 1: {
                        syntaxNode* target = root->child[pos];
                        assert(target);
                        assert( target->hashData.has(strValue) );
                        int* ptrValFromE = *(int**)target->hashData.get(strValue);
                        assert(ptrValFromE);
                        root->hashData.add(strValue, new int(*ptrValFromE));
                        //printf("L-%d\t",getLineNum());
                        root->hashData.add( strPlace, new int(*(int*)*target->hashData.get(strPlace)) );
                        //printf("t%d = t%d\n", *(int*)*root->hashData.get(strPlace), *(int*)*target->hashData.get(strPlace));
                    } break;
                }
                return true;
            },
        };

        ///准备怎么制导呢？

        dfs(ptrSyntaxTreeRoot, transFuncArr);
        printf("\n");
        return *this;
    }


    //syntaxParse* buildStateTable
};

#endif // PARSER_H_INCLUDED
