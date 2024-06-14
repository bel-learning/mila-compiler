#ifndef PJPPROJECT_PARSER_HPP
#define PJPPROJECT_PARSER_HPP

#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <fstream>

#include "Lexer.hpp"
#include "AST.hpp"

class Parser {
public:
    Parser();
    ~Parser() = default;

    bool Parse();             // parse
    void initLexer(std::istream& ifs);  // initialize lexer
    const llvm::Module& Generate();  // generate
    void printCurrentToken();
private:
    int getNextToken();
    Lexer m_Lexer;                   // lexer is used to read tokens
    int CurTok;                      // to keep the current token

    llvm::LLVMContext MilaContext;   // llvm context
    llvm::IRBuilder<> MilaBuilder;   // llvm builder
    llvm::Module MilaModule;         // llvm module

    void printAST();
    bool consume(int token);
    std::unique_ptr<ExprAST> Lmax();
    std::unique_ptr<ExprAST> L4();
    std::unique_ptr<ExprAST> L3();
    std::unique_ptr<ExprAST> L2();
    std::unique_ptr<ExprAST> L1();
    std::unique_ptr<ExprAST> L0();
    std::unique_ptr<ExprAST> ParseNumberExpr();
    std::unique_ptr<ExprAST> ParseParenExpr();
    std::unique_ptr<ExprAST> ParseExpression();

};

#endif //PJPPROJECT_PARSER_HPP
