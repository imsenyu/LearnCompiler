#ifndef TRANSLATOR_H_INCLUDED
#define TRANSLATOR_H_INCLUDED

#include "stdafx.h"
#include "parser.h"

class Translator{
public:
    typedef function<bool(syntaxNode& root, int pos)> TransFuncType;
protected:
    syntaxParser* ptrParser;
    int numLine;
    int numTmp;
    vector<TransFuncType> vecTransFunc;
public:
    inline void nop() {}
    Translator(syntaxParser* _parser = NULL): ptrParser( _parser ) {
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
