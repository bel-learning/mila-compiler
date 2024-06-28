//
// Created by Bilguudei Baljinnyam on 14.06.2024.
//
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <iostream>
#include <memory>

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

#include <deque>
#include "Lexer.hpp"
#include <stack>

#ifndef MILA_AST_HPP
#define MILA_AST_HPP


class TypeAST;

struct Symbol {
    llvm::AllocaInst* store;
//    Whether constant variable or not
    bool constant;
};


//struct SymbolTable {
//public:
//    SymbolTable() {
//        newScope();
//    };
//    void newScope() {
//        pages.emplace_front();
//    }
//    void deleteScope() {
//        pages.pop_front();
//    }
//    Symbol * search(const std::string & var) {
//        for(auto it = pages.begin(); it != pages.end(); it++) {
//            std::map<std::string, Symbol> & page = *it;
//            auto searchIter = page.find(var);
//            if(searchIter != page.end()) {
//                return &page.at(var);
//            }
//        }
//
//        return nullptr;
//    }
//    bool exists (const std::string & var) {
//        if(pages.front().find(var) == pages.front().end()) return false;
//        return true;
//    }
//
//    Symbol & operator [] (const std::string & var) {
//        return pages.front()[var];
//    }
//    void add (const std::string & var, llvm::AllocaInst * alloca) {
//        pages.front()[var] = {alloca};
//    }
//
//
//    void erase(const std::string & var) {
//        pages.front().erase(var);
//    }
//    void clear() {
//        pages.front().clear();
//    }
//
//    std::deque<std::map<std::string, Symbol> > pages;
//};

using SymbolTable = std::map<std::string, Symbol>;


struct GenContext {
    GenContext(const std::string& moduleName) : ctx(), builder(ctx), module(moduleName, ctx) {};

    llvm::LLVMContext ctx;
    llvm::IRBuilder<> builder;
    llvm::Module module;

    std::stack<llvm::BasicBlock*> loopExitBlocks;
    SymbolTable symbTable;
};


class AST {
public:
    virtual ~AST() = default;
    virtual void print(std::ostream &out, int indent = 0) const = 0;

    friend std::ostream &operator<<(std::ostream &out, const AST &ast) {
        ast.print(out);
        return out;
    }
    virtual llvm::Value * codegen(GenContext& gen ) = 0;
};

class ExprAST : public AST {
public:
    virtual ~ExprAST() = default;
//    virtual void print(std::ostream &out, int indent = 0) const = 0;
    virtual const std::string &getName() const = 0;
};

class StatementAST : public AST {
public:
    virtual ~StatementAST() = default;
//    virtual void print(std::ostream &out, int indent = 0) const = 0;
};

class BlockAST : public AST {
    std::vector<std::unique_ptr<AST>> m_Body;
public:
    BlockAST(std::vector<std::unique_ptr<AST>> body) ;
    virtual ~BlockAST() = default;

    void print(std::ostream &out, int indent = 0) const override ;
    llvm::Value * codegen(GenContext& gen) override ;
};

class TypeAST : public AST {
public:
    enum class Type { INT,
        DOUBLE,
    };
    TypeAST(Type type);
//    void print(std::ostream& os, unsigned indent = 0) const override;
//    llvm::Value* codegen(GenContext& gen) const override;
    void print(std::ostream &out, int indent = 0) const override;
    llvm::Value * codegen(GenContext& gen) override;
private:
    Type m_type;
};

class NumberExprAST : public ExprAST {
    int m_Val;
public:
    NumberExprAST(int val) ;
    const std::string &getName() const override ;

    void print(std::ostream &out, int indent = 0) const override ;
    llvm::Value * codegen(GenContext& gen) override ;
};


class DeclRefAST : public ExprAST {
    std::string m_Var;
public:
    DeclRefAST(std::string var);

    const std::string &getName() const override;

    void print(std::ostream &out, int indent = 0) const override;
    llvm::Value * codegen(GenContext& gen) override;
//    llvm::Value* codegen(GenContext& gen) const override;
//    llvm::AllocaInst* getStore(GenContext& gen) const;
};

class VarDeclAST : public StatementAST {
    std::string m_var;
    std::unique_ptr<TypeAST> m_type;
    std::unique_ptr<ExprAST> m_expr;
    bool m_constant;
public:
    VarDeclAST(std::string var, std::unique_ptr<TypeAST> type, std::unique_ptr<ExprAST> expr, bool constant);

    void print(std::ostream &out, int indent = 0) const override;
    llvm::Value* codegen(GenContext &gen) override;

//    llvm::Value* codegen(GenContext& gen) const override;
};


/// BinaryExprAST - Expression class for a binary operator.
class BinaryExprAST : public ExprAST {
    char Op;
    std::unique_ptr<ExprAST> m_LHS, m_RHS;

public:
    BinaryExprAST(char Op, std::unique_ptr<ExprAST> LHS, std::unique_ptr<ExprAST> RHS);

    const std::string &getName() const override;

    void print(std::ostream &out, int indent = 0) const override;
    llvm::Value * codegen(GenContext& gen) override;

    llvm::Value * codegenAssignment(GenContext & gen);
};

/// CallExprAST - Expression class for function calls.
class CallExprAST : public ExprAST {
    std::string Callee;
    std::vector<std::unique_ptr<ExprAST>> Args;

public:
    CallExprAST(std::string Callee, std::vector<std::unique_ptr<ExprAST>> Args);
    const std::string &getName() const override ;

    void print(std::ostream &out, int indent = 0) const override ;
    llvm::Value * PredefinedFunctions(GenContext& gen) ;
    llvm::Value * codegen(GenContext& gen) override ;
};

/// PrototypeAST - This class represents the "prototype" for a function,
/// which captures its name, and its argument names (thus implicitly the number
/// of arguments the function takes).
class PrototypeAST : StatementAST  {
    std::string m_Name;
    std::vector<std::string> m_Args;
    std::unique_ptr<VarDeclAST> m_Return;
public:
    PrototypeAST(const std::string &Name, std::vector<std::string> Args, std::unique_ptr<VarDeclAST> Return);

    const std::string &getName() const ;

    void print(std::ostream &out, int indent = 0) const override ;
    llvm::Function * codegen(GenContext& gen) override;
};

/// FunctionAST - This class represents a function definition itself.
class FunctionAST : public StatementAST {
    std::unique_ptr<PrototypeAST> m_Proto;
    std::vector<std::unique_ptr<VarDeclAST>> m_Vars;
    std::unique_ptr<AST> m_Body;

public:
    FunctionAST(std::unique_ptr<PrototypeAST> Proto, std::vector<std::unique_ptr<VarDeclAST>> Vars, std::unique_ptr<AST> Body);

    void print(std::ostream &out, int indent = 0) const  override ;
    llvm::Value * codegen(GenContext& gen) override ;
};

class FunctionExitAST : public StatementAST {
public:
    FunctionExitAST() = default;

    void print(std::ostream &out, int indent = 0) const override ;
    llvm::Value * codegen(GenContext& gen) override ;
};
class LoopBreakAST : public StatementAST {
public:
    LoopBreakAST() = default;

    void print(std::ostream &out, int indent = 0) const override ;
    llvm::Value * codegen(GenContext& gen) override ;
};



class IfStmtAST : public StatementAST {
    std::unique_ptr<AST> m_Cond, m_Then, m_Else;

public:
    IfStmtAST(std::unique_ptr<AST> cond, std::unique_ptr<AST> then,
              std::unique_ptr<AST> Else);

    void print(std::ostream &out, int indent = 0) const override ;

    llvm::Value *codegen(GenContext & gen) override;
};

class ForStmtAST : public StatementAST {
    std::string m_Var;
    std::unique_ptr<ExprAST> m_Start, m_End;
    std::unique_ptr<NumberExprAST> m_Step;
    std::unique_ptr<AST> m_Body;

public:
    ForStmtAST(const std::string &Var, std::unique_ptr<ExprAST> Start,
               std::unique_ptr<ExprAST> End, std::unique_ptr<NumberExprAST> Step,
               std::unique_ptr<AST> Body) ;
    void print(std::ostream &out, int indent = 0) const override  ;

    llvm::Value *codegen(GenContext & gen) override ;
};

class WhileStmtAST : public StatementAST {
    std::unique_ptr<ExprAST> m_Cond;
    std::unique_ptr<AST> m_Body;

public:
    WhileStmtAST(std::unique_ptr<ExprAST> cond, std::unique_ptr<AST> body);

    void print(std::ostream &out, int indent = 0) const override ;
    llvm::Value *codegen(GenContext &gen) override ;
};




#endif // MILA_AST_HPP

