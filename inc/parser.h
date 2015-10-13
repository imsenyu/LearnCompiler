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

using namespace clUtils;

class syntaxParser;
class CLikeSyntaxParser;
/*
 * class syntaxParser
 * description: 通用语法分析程序，完成LR1 ActionGoto表构造，语法规约，语法树生成
 * data:    vecTerm 存储Term*指针
 *          vecATerm 存储Production*指针
 *          vecStates 存储项目规范簇StateSet*状态指针
 *          mpTerm 字符串到Term*的查找映射
 *          mpStateTable <int,Term*,int> 状态转换表
 *          ptrAGTable ActionGoto表指针
 *          ptrSyntaxTreeRoot 语法树根指针
 *          vecToken 存储输入词法
 *
 * method:  ConstructLR1 根据输入流构造LR1文法分析的所有数据
 *          ConstructTree 根据输入流构造语法树
 *          solveSyntaxException 虚函数，重写特定文法的 LR规约错误处理
 *
 */
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
private:
    bool _isDebug;
    bool _isLR;
    bool _isTree;
public:
    syntaxParser(bool _debug = true):_isDebug(_debug), ptrAGTable(NULL), ptrSyntaxTreeRoot(NULL) { }
//protected
public:
    ///所有数据存储使用vector
    ///!!!考虑使用C++11 Move语义后引入 vector< unique_ptr<Term*> > 这个样子
    vector<Term*> vecTerm;
    vector<Production*> vecATerm;
    vector<StateSet*> vecStates;
    vector<string> vecError;
    ///供数据查找
    map<string, Term*> mpTerm;  ///Term查找
    D2Map<int,Term*,int> mpStateTable;  /// StateSet的int编号的 关于Term*的转移表
    ActionGotoTable* ptrAGTable;    ///AGTable指针
    syntaxNode* ptrSyntaxTreeRoot;  ///语法树指针
    vector<Token*> vecToken;    ///顺序读取


protected:

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

    syntaxParser& destoryAll() {

        for(auto ptrTerm : vecTerm )
            delete ptrTerm;
        ///为什么不在production的析构函数中重载，因为Term*,Production*,StateItem*是连一起的。
        for(auto ptrPdt : vecATerm ) {
            for(auto ptrSItem : ptrPdt->vecSItems )
                delete ptrSItem;
            delete ptrPdt;
        }
        ///collection中的被重载delete了
        for(auto ptrState : vecStates )
            delete ptrState;
        if ( NULL != ptrAGTable ) {
            delete ptrAGTable;
            ptrAGTable = NULL;
        }
        if ( NULL != ptrSyntaxTreeRoot ) {
            delete ptrSyntaxTreeRoot;
            ptrSyntaxTreeRoot = NULL;
        }
        for(auto ptrToken : vecToken)
            delete ptrToken;

        _isLR = false;
        _isTree = false;

        return *this;
    }

public:
    inline void nop() {}
    syntaxParser& ConstructLR1(istream& grammarIn);
    syntaxParser& ConstructTree(istream& lexIn);
    virtual bool solveSyntaxException(stack<int>& stkState, stack<syntaxNode*>& stkSyntaxNode, int& vTokenCnt);
    syntaxNode* getSyntaxTree() const{
        return _isTree ? ptrSyntaxTreeRoot : NULL;
    }
    vector<const Production*> getProduction() const {
        vector<const Production*> ret;
        for(auto ptrPdt : vecATerm)
            ret.push_back(ptrPdt);
        return ret;
    }
    inline bool getIsTree() const { return _isTree; }
    inline bool getIsLR() const { return _isLR; }
    //syntaxParse* buildStateTable
};

class CLikeSyntaxParser : public syntaxParser {
public:
    CLikeSyntaxParser(bool _debug = false):syntaxParser(_debug) {}
    bool solveSyntaxException(stack<int>& stkState, stack<syntaxNode*>& stkSyntaxNode, int& vTokenCnt);
};

#endif // PARSER_H_INCLUDED
