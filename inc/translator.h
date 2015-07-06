#ifndef TRANSLATOR_H_INCLUDED
#define TRANSLATOR_H_INCLUDED

#include "stdafx.h"
#include "parser.h"
#include "json/json.h"

class AssembleCode {
public:
//    enum OperationType { Halt = 1, Jmp, Goto, Add, Multiply, Mov, Inc, Dec };
//    OperationType action;
    int lineNum;
    Json::Value data;
    AssembleCode(int _line = 0): lineNum(_line){}
    Json::Value& operator[](const unsigned int& key) {
        return data[key];
    }
    const Json::Value& operator[](const unsigned int& key) const {
        return data[key];
    }
};

class Translator{
public:
    typedef function<bool(syntaxNode& root, int pos)> TransFuncType;
protected:
    syntaxParser* ptrParser;
    int numLine;
    int numTmp;
    int numLineOffset;
    vector<AssembleCode> vecCodes;
    vector<TransFuncType> vecTransFunc;
public:
    inline void nop() {}
    Translator(syntaxParser* _parser = NULL, int _offset = 0): ptrParser( _parser ), numLineOffset(_offset) {
        init( ptrParser );
    }
    Translator& init( syntaxParser* _newParser );
    Translator& translate();

protected:
    ///定义成虚 =0函数，必须派生。
    virtual vector<TransFuncType> getTransFunc() = 0;
    void translateRecur(syntaxNode* root, vector<TransFuncType>& func);
    int getLineNum(bool _plus = true, bool start = false);
    int getNewTmp(bool start = false);
    void Backpatch( Json::Value& p, Json::Value t );
    Json::Value Merge( Json::Value& p1, Json::Value p2 );

    template<typename T1, typename T2, typename T3, typename T4>
    int Emit( const int _line, const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4) {
        AssembleCode ret(_line);
        ret.data.append(arg1);
        ret.data.append(arg2);
        ret.data.append(arg3);
        ret.data.append(arg4);
        vecCodes.push_back(ret);
        return _line;
    }

};

class ArithmeticTranslator : public Translator {
public:
    ArithmeticTranslator(syntaxParser* _parser = NULL): Translator(_parser) {}
    virtual vector<TransFuncType> getTransFunc();
};

class CLikeTranslator : public Translator {
public:
    CLikeTranslator(syntaxParser* _parser = NULL): Translator(_parser) {}
    virtual vector<TransFuncType> getTransFunc();
};

#endif // TRANSLATOR_H_INCLUDED
