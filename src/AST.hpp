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
    BlockAST(std::vector<std::unique_ptr<AST>> body) : m_Body(std::move(body)) {}
    virtual ~BlockAST() = default;

    void print(std::ostream &out, int indent = 0) const override {
        out << std::string(indent, ' ') << "{\n";
        for (const auto &stmt : m_Body) {
            if(!stmt)
                std::clog << "Nullptr in: " << " block" << std::endl;
            else
                stmt->print(out, indent + 2);
            out << ",\n";
        }
        out << std::string(indent, ' ') << "}";
    }
    llvm::Value * codegen(GenContext& gen) override {
        for(auto & expression : m_Body) {
            expression->codegen(gen);
        }
        return nullptr;
    };
};

class TypeAST : public AST {
public:
    enum class Type { INT,
        DOUBLE,
    };
    TypeAST(Type type) : m_type(type) {};
//    void print(std::ostream& os, unsigned indent = 0) const override;
//    llvm::Value* codegen(GenContext& gen) const override;
    void print(std::ostream &out, int indent = 0) const override {
        out << std::string(indent, ' ') << " Type: " << "INT" << "\n";
    }
    llvm::Value * codegen(GenContext& gen) override { return nullptr;};
private:
    Type m_type;
};

class NumberExprAST : public ExprAST {
    int m_Val;
public:
    NumberExprAST(int val) : m_Val(val) {}
    const std::string &getName() const override { return ""; };

    void print(std::ostream &out, int indent = 0) const override {
        out << std::string(indent, ' ') << "{\n";
        out << std::string(indent + 2, ' ') << "\"type\": \"NumberExprAST\",\n";
        out << std::string(indent + 2, ' ') << "\"value\": " << m_Val << "\n";
        out << std::string(indent, ' ') << "}";
    }
    llvm::Value * codegen(GenContext& gen) override {
        return llvm::ConstantInt::get(llvm::Type::getInt32Ty(gen.ctx), m_Val, true);
    }
};


class DeclRefAST : public ExprAST {
    std::string m_Var;
public:
    DeclRefAST(std::string var): m_Var(var) {};

    const std::string &getName() const override { return m_Var; };

    void print(std::ostream &out, int indent = 0) const override {
        out << std::string(indent, ' ') << "{\n";
        out << std::string(indent + 2, ' ') << "\"type\": \"DeclRefASTNode\",\n";
        out << std::string(indent + 2, ' ') << "\"name\": \"" << m_Var << "\"\n";
        out << std::string(indent, ' ') << "}";
    };
    llvm::Value * codegen(GenContext& gen) override {
        // Look up the variable in the symbol table
//        std::clog << "Codegening DeclRefAST: " << m_Var << std::endl;
        auto searchIt = gen.symbTable.find(m_Var);
        if (searchIt == gen.symbTable.end()) {
            throw std::runtime_error("Unknown variable name: " + m_Var);
        }
        // Return the stored LLVM Value for the variable
        llvm::Value * val = gen.builder.CreateLoad(llvm::Type::getInt32Ty(gen.ctx), searchIt->second.store, m_Var);

        return val;
    };
//    llvm::Value* codegen(GenContext& gen) const override;
//    llvm::AllocaInst* getStore(GenContext& gen) const;
};

class VarDeclAST : public StatementAST {
    std::string m_var;
    std::unique_ptr<TypeAST> m_type;
    std::unique_ptr<ExprAST> m_expr;
    bool m_constant;
public:
    VarDeclAST(std::string var, std::unique_ptr<TypeAST> type, std::unique_ptr<ExprAST> expr, bool constant) :
            m_var(var), m_type(std::move(type)), m_expr(std::move(expr)), m_constant(constant) {};
    void print(std::ostream &out, int indent = 0) const override {
        out << std::string(indent, ' ') << "{\n";
        out << std::string(indent + 2, ' ') << "\"type\": \"VarDeclAST\",\n";
        out << std::string(indent + 2, ' ') << "\"mvar\": "<< m_var;
//        out << std::string(indent + 2, ' ') << "\"operator\": \"" << Op << "\",\n";
        out << "\n" << std::string(indent, ' ') << "}";
    };
    llvm::Value* codegen(GenContext &gen) override {
        // Generate the type for the variable
//        llvm::Type* varType = m_type->codegen(gen);
        std::clog << "Codegening VarDeclAST: " << m_var << std::endl;
        llvm::Type * varType = llvm::Type::getInt32Ty(gen.ctx);

        if (!varType) {
            throw std::runtime_error("Unknown type for variable: " + m_var);
        }

        // Create an alloca instruction in the entry block of the function
        llvm::Function* function = gen.builder.GetInsertBlock()->getParent();
        llvm::IRBuilder<> tmpBuilder(&function->getEntryBlock(),
                                     function->getEntryBlock().begin());
        llvm::AllocaInst* alloca = tmpBuilder.CreateAlloca(varType, 0, m_var.c_str());

        // Initialize the variable if an initializer expression is provided
        if (m_expr) {
            llvm::Value* initVal = m_expr->codegen(gen);
            if (!initVal) {
                throw std::runtime_error("Failed to generate initializer for variable: " + m_var);
            }

            // Ensure the types match
//            if (initVal->getType() != varType) {
//                throw std::runtime_error("Type mismatch in initializer for variable: " + m_var);
//            }

            gen.builder.CreateStore(initVal, alloca);
        }

        // Add the variable to the symbol table
        auto searchIt = gen.symbTable.find(m_var);
        if(searchIt != gen.symbTable.end()) {
            throw std::runtime_error("Already exists var: " + m_var);
        }
        gen.symbTable[m_var] = {alloca, m_constant};

        return alloca;
    }

//    llvm::Value* codegen(GenContext& gen) const override;
};


/// BinaryExprAST - Expression class for a binary operator.
class BinaryExprAST : public ExprAST {
    char Op;
    std::unique_ptr<ExprAST> m_LHS, m_RHS;

public:
    BinaryExprAST(char Op, std::unique_ptr<ExprAST> LHS, std::unique_ptr<ExprAST> RHS)
            : Op(Op), m_LHS(std::move(LHS)), m_RHS(std::move(RHS)) {}
    const std::string &getName() const override { return ""; };

    void print(std::ostream &out, int indent = 0) const override {
        out << std::string(indent, ' ') << "{\n";
        out << std::string(indent + 2, ' ') << "\"type\": \"BinaryExprAST\",\n";
        out << std::string(indent + 2, ' ') << "\"operator\": \"" << Op << "\",\n";
        out << std::string(indent + 2, ' ') << "\"LHS\": ";
        m_LHS->print(out, indent + 2);
        out << ",\n";
        out << std::string(indent + 2, ' ') << "\"RHS\": ";
        m_RHS->print(out, indent + 2);
        out << "\n" << std::string(indent, ' ') << "}";
    }
    llvm::Value * codegen(GenContext& gen) override {
        llvm::Value *L = m_LHS->codegen(gen);
        llvm::Value *R = m_RHS->codegen(gen);
        if (!L || !R)
            return nullptr;

        switch (Op) {
            case '+':
                return gen.builder.CreateAdd(L, R, "addtmp");
            case '-':
                return gen.builder.CreateSub(L, R, "subtmp");
            case '*':
                return gen.builder.CreateMul(L, R, "multmp");
            case '<':
                L = gen.builder.CreateICmpSLT(L, R, "cmptmp");
                // Convert bool 0/1 to int 0 or 1
                return gen.builder.CreateIntCast(L, llvm::Type::getInt32Ty(gen.ctx), false, "lesstmp");
            case '>':
                L = gen.builder.CreateICmpSGT(L, R, "cmptmp");
                // Convert bool 0/1 to int 0 or 1
                return gen.builder.CreateIntCast(L, llvm::Type::getInt32Ty(gen.ctx), false, "greatertmp");
            case tok_lessequal:
                L = gen.builder.CreateICmpSLE(L, R, "cmptmp");
                // Convert bool 0/1 to int 0 or 1
                return gen.builder.CreateIntCast(L, llvm::Type::getInt32Ty(gen.ctx), false, "lsetmp");
            case tok_greaterequal:
                L = gen.builder.CreateICmpSGE(L, R, "cmptmp");
                // Convert bool 0/1 to int 0 or 1
                return gen.builder.CreateIntCast(L, llvm::Type::getInt32Ty(gen.ctx), false, "gsetmp");
            case tok_equal:
                L = gen.builder.CreateICmpEQ(L, R, "cmptmp");
                // Convert bool 0/1 to int 0 or 1
                return gen.builder.CreateIntCast(L, llvm::Type::getInt32Ty(gen.ctx), false, "eqtmp");
            case tok_notequal:
                L = gen.builder.CreateICmpNE(L, R, "cmptmp");
                // Convert bool 0/1 to int 0 or 1
                return gen.builder.CreateIntCast(L, llvm::Type::getInt32Ty(gen.ctx), false, "netmp");
            case tok_or:
                L = gen.builder.CreateOr(L, R, "ortmp");
                // Convert bool 0/1 to int 0 or 1
                return gen.builder.CreateIntCast(L, llvm::Type::getInt32Ty(gen.ctx), false, "ortmp");
            case tok_and:
                L = gen.builder.CreateAnd(L, R, "andtmp");
                // Convert bool 0/1 to int 0 or 1
                return gen.builder.CreateIntCast(L, llvm::Type::getInt32Ty(gen.ctx), false, "andtmp");

            case tok_mod:
                return gen.builder.CreateSRem(L, R, "sremtmp");
            case tok_assign:
                return codegenAssignment(gen);


//            case tok_and:
//                L = gen.builder.CreateSRem(L, R, "sremtmp");
            default:
                std::clog << "Exception" << std::endl;
                return nullptr;
        };
    }

    llvm::Value * codegenAssignment(GenContext & gen) {
        // Generate code for the LHS, which should be an address
        llvm::Value* lhs = m_LHS->codegen(gen);
        if (!lhs) {
            throw std::runtime_error("Failed to generate LHS for assignment.");
        }
        auto searchIter = gen.symbTable.find(m_LHS->getName());
        if(searchIter != gen.symbTable.end()) {
            if(searchIter->second.constant) {
                throw std::runtime_error("Trying to change const value");
            }
        }

        // Generate code for the RHS, which should be a value
        llvm::Value* rhs = m_RHS->codegen(gen);
        if (!rhs) {
            throw std::runtime_error("Failed to generate RHS for assignment.");
        }

        llvm::Value * variable = gen.symbTable[m_LHS->getName()].store;
        if(!variable) {
            std::clog << "Var name(from LLVM): " << m_LHS->getName() << std::endl;
            throw std::runtime_error("Unknown variable");
        }

        // Store the RHS value into the LHS address
        gen.builder.CreateStore(rhs, variable);

        // Return the stored value (RHS) for any further use
        return rhs;
    }
};

/// CallExprAST - Expression class for function calls.
class CallExprAST : public ExprAST {
    std::string Callee;
    std::vector<std::unique_ptr<ExprAST>> Args;

public:
    CallExprAST(std::string Callee, std::vector<std::unique_ptr<ExprAST>> Args)
            : Callee(std::move(Callee)), Args(std::move(Args)) {}
    const std::string &getName() const override { return Callee; }

    void print(std::ostream &out, int indent = 0) const override {
        out << std::string(indent, ' ') << "{\n";
        out << std::string(indent + 2, ' ') << "\"type\": \"CallExprAST\",\n";
        out << std::string(indent + 2, ' ') << "\"callee\": \"" << Callee << "\",\n";
        out << std::string(indent + 2, ' ') << "\"args\": [\n";
        for (const auto &arg : Args) {
            arg->print(out, indent + 4);
            out << ",\n";
        }
        out << std::string(indent + 2, ' ') << "]\n";
        out << std::string(indent, ' ') << "}";
    }
    llvm::Value * PredefinedFunctions(GenContext& gen) {
        if(Callee == "dec") {
            if(Args.empty()) return nullptr;
            llvm::AllocaInst * Var = gen.symbTable.find(Args[0]->getName())->second.store;
            llvm::Value * Val = gen.builder.CreateLoad(llvm::Type::getInt32Ty(gen.ctx), Var, Args[0]->getName());
            llvm::Value * Add = gen.builder.CreateSub(Val, NumberExprAST(1).codegen(gen));
            gen.builder.CreateStore(Add, Var);
            gen.symbTable[Args[0]->getName()] = {Var, false};
            return Add;
        }
        return nullptr;
    }
    llvm::Value * codegen(GenContext& gen) override {
        std::clog << "Callee: " << Callee << std::endl;
//        for (const llvm::Function &func : gen.module) {
//            if (!func.isDeclaration()) {
//                llvm::outs() << "Function: " << func.getName() << "\n";
//            }
//        }
//        Check preexisting functions
        llvm::Value * retValue = PredefinedFunctions(gen);
        if(retValue != nullptr) {
            return retValue;
        }

        llvm::Function * calleeF = gen.module.getFunction(Callee);
        if(calleeF == nullptr) {
            throw std::runtime_error("Unknown function referenced: " + Callee);
        }
        // Check if the argument count matches.
        if (calleeF->arg_size() != Args.size()) {
            throw std::runtime_error("Incorrect number of arguments passed to: " + Callee);
        }

        std::vector<llvm::Value *> argsV;
        for (unsigned i = 0; i < Args.size(); ++i) {

            llvm::Value *argValue = Args[i]->codegen(gen);

            // Special case for "readln" function
            if (Callee == "readln") {
                // Create a pointer to the argument if necessary
                llvm::AllocaInst * var = gen.symbTable[Args[i]->getName()].store;
                if(!var) {
                    throw std::runtime_error("Var doesn't exist");
                }
                //                Args[i].
                argsV.push_back(var);
            } else {
                argsV.push_back(argValue);
            }

        }

        // Check if the callee function returns void
        if (calleeF->getReturnType()->isVoidTy()) {
            std::clog << "Callee no return type: " << Callee << std::endl;
            gen.builder.CreateCall(calleeF, argsV);

            return nullptr; // No value to return for void functions
        } else {
            return  gen.builder.CreateCall(calleeF, argsV, "callfunc");; // Return the call instruction for non-void functions
        }
    };
};

/// PrototypeAST - This class represents the "prototype" for a function,
/// which captures its name, and its argument names (thus implicitly the number
/// of arguments the function takes).
class PrototypeAST : StatementAST  {
    std::string m_Name;
    std::vector<std::string> m_Args;
    std::unique_ptr<VarDeclAST> m_Return;
public:
    PrototypeAST(const std::string &Name, std::vector<std::string> Args, std::unique_ptr<VarDeclAST> Return)
            : m_Name(Name), m_Args(std::move(Args)), m_Return(std::move(Return)) {}

    const std::string &getName() const { return m_Name; }

    void print(std::ostream &out, int indent = 0) const override{
        out << std::string(indent, ' ') << "{\n";
        out << std::string(indent + 2, ' ') << "\"type\": \"PrototypeAST\",\n";
        out << std::string(indent + 2, ' ') << "\"name\": \"" << m_Name << "\",\n";
        out << std::string(indent + 2, ' ') << "\"args\": [\n";
        for (const auto &arg : m_Args) {
            out << std::string(indent + 4, ' ') << "\"" << arg << "\",\n";
        }
        out << std::string(indent + 2, ' ') << "]\n";
        out << std::string(indent, ' ') << "}";
    }
    llvm::Function * codegen(GenContext& gen)  {
        std::vector<llvm::Type *> INTS(m_Args.size(), llvm::Type::getInt32Ty(gen.ctx));
        llvm::FunctionType * FT = nullptr;
        if(m_Return == nullptr && m_Name != "main")
            FT = llvm::FunctionType::get(llvm::Type::getVoidTy(gen.ctx), INTS ,false);
        else
            FT = llvm::FunctionType::get(llvm::Type::getInt32Ty(gen.ctx), INTS ,false);
        llvm::Function *F =
                llvm::Function::Create(FT, llvm::Function::ExternalLinkage, m_Name, gen.module);

        // Set names for all arguments.
        unsigned Idx = 0;
        for (auto &arg : F->args())
            arg.setName(m_Args[Idx++]);

        return F;
    };
};

/// FunctionAST - This class represents a function definition itself.
class FunctionAST : public StatementAST {
    std::unique_ptr<PrototypeAST> m_Proto;
    std::vector<std::unique_ptr<VarDeclAST>> m_Vars;
    std::unique_ptr<AST> m_Body;

public:
    FunctionAST(std::unique_ptr<PrototypeAST> Proto, std::vector<std::unique_ptr<VarDeclAST>> Vars, std::unique_ptr<AST> Body)
            : m_Proto(std::move(Proto)), m_Vars(std::move(Vars)), m_Body(std::move(Body)) {}

    void print(std::ostream &out, int indent = 0) const  override {
        out << std::string(indent, ' ') << "{\n";
        out << std::string(indent + 2, ' ') << "\"type\": \"FunctionAST\",\n";
        out << std::string(indent + 2, ' ') << "\"prototype\": ";
        m_Proto->print(out, indent + 2);
        out << ",\n";
        out << std::string(indent + 2, ' ') << "\"body\": ";
        if(m_Body)
            m_Body->print(out, indent + 2);
        out << "\n" << std::string(indent, ' ') << "}";
    }
    llvm::Value * codegen(GenContext& gen) override {
        llvm::Function *TheFunction = gen.module.getFunction(m_Proto->getName());
        if (!TheFunction)
            TheFunction = m_Proto->codegen(gen);
        if(!m_Body) return TheFunction;
        if(m_Proto->getName() == "main") {
            llvm::BasicBlock *MainBB = llvm::BasicBlock::Create(gen.ctx, "entry", TheFunction);
            gen.builder.SetInsertPoint(MainBB);
            gen.symbTable.clear();

            for (auto &variable : m_Vars)
            {
                variable->codegen(gen);
            }

            m_Body->codegen(gen);
            // return 0
            gen.builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(gen.ctx), 0));
            return TheFunction;
        }

        llvm::BasicBlock * BB = llvm::BasicBlock::Create(gen.ctx, m_Proto->getName(), TheFunction);
        gen.builder.SetInsertPoint(BB);

        gen.symbTable.clear();
        // Create return value
        llvm::AllocaInst *AllocaReturnVar = gen.builder.CreateAlloca(llvm::Type::getInt32Ty(gen.ctx), nullptr, m_Proto->getName());
        gen.symbTable[std::string(m_Proto->getName())] = {AllocaReturnVar, false};

        for (auto &Arg : TheFunction->args()) {
            // Create an alloca for this variable
            llvm::AllocaInst *Alloca = gen.builder.CreateAlloca(Arg.getType(), nullptr, Arg.getName());
            // Store the initial value into the alloca
            gen.builder.CreateStore(&Arg, Alloca);
            // Add the variable to the symbol table
            gen.symbTable[std::string(Arg.getName())] = {Alloca, false};
        }

        for(auto &Var : m_Vars)
            Var->codegen(gen);



        m_Body->codegen(gen);

        // If the function has a non-void return type, generate the return instruction
        if (TheFunction->getReturnType()->isVoidTy()) {
            gen.builder.CreateRetVoid();
        } else {
            llvm::Value *RetVal = gen.builder.CreateLoad(llvm::Type::getInt32Ty(gen.ctx), AllocaReturnVar, "return");
            gen.builder.CreateRet(RetVal);
        }

        // Validate the generated code, checking for consistency
        llvm::verifyFunction(*TheFunction);

        return TheFunction;

    };
};
class FunctionExitAST : public StatementAST {
public:
    FunctionExitAST() = default;

    void print(std::ostream &out, int indent = 0) const override {
        out << std::string(indent + 2, ' ') << "\"type\": \">Function Exit<\",\n";
    }
    llvm::Value * codegen(GenContext& gen) override {
        llvm::Function *TheFunction = gen.builder.GetInsertBlock()->getParent();
        llvm::Type *ReturnType = TheFunction->getReturnType();
        std::string functionName = TheFunction->getName().str();
        std::clog << "FUNCTION NAME:::" << functionName << std::endl;
        if (ReturnType->isVoidTy()) {
            gen.builder.CreateRetVoid();
        } else {
            llvm::Value *RetVal = gen.builder.CreateLoad(llvm::Type::getInt32Ty(gen.ctx),
                                                         gen.symbTable[std::string(functionName)].store, functionName);
            gen.builder.CreateRet(RetVal);
        }
        return nullptr;
    };

};


class IfStmtAST : public StatementAST {
    std::unique_ptr<AST> m_Cond, m_Then, m_Else;

public:
    IfStmtAST(std::unique_ptr<AST> cond, std::unique_ptr<AST> then,
              std::unique_ptr<AST> Else)
            : m_Cond(std::move(cond)), m_Then(std::move(then)), m_Else(std::move(Else)) {}


    void print(std::ostream &out, int indent = 0) const override {
        out << std::string(indent, ' ') << "{\n";
        out << std::string(indent + 2, ' ') << "\"type\": \"IF\",\n";
        m_Cond->print(out, indent + 2);
        m_Then->print(out, indent + 2);

        if(m_Else) m_Else->print(out, indent + 2);
        out << std::string(indent, ' ') << "}";
    }

    llvm::Value *codegen(GenContext & gen) override {
        llvm::Value *CondV = m_Cond->codegen(gen);
        if (!CondV)
            return nullptr;

        // Convert condition to a bool by comparing non-equal to 0
        CondV = gen.builder.CreateICmpNE(CondV, llvm::ConstantInt::get(CondV->getType(), 0, true), "ifcond");


        llvm::Function * TheFunction = gen.builder.GetInsertBlock()->getParent();

        // Create blocks for the then, else, and the continuation (merge) block
        llvm::BasicBlock *ThenBB = llvm::BasicBlock::Create(gen.ctx, "then", TheFunction);
        llvm::BasicBlock *MergeBB = llvm::BasicBlock::Create(gen.ctx, "ifcont");

        llvm::BasicBlock *ElseBB = nullptr;
        if (m_Else) {
            ElseBB = llvm::BasicBlock::Create(gen.ctx, "else");
            gen.builder.CreateCondBr(CondV, ThenBB, ElseBB);
        } else {
            gen.builder.CreateCondBr(CondV, ThenBB, MergeBB);
        }

        gen.builder.SetInsertPoint(ThenBB);

        m_Then->codegen(gen);

        gen.builder.CreateBr(MergeBB);

        if(m_Else) {
            TheFunction->getBasicBlockList().push_back(ElseBB);
            gen.builder.SetInsertPoint(ElseBB);
            m_Else->codegen(gen);
            gen.builder.CreateBr(MergeBB);
        }

        // Generate code for the merge block
        TheFunction->getBasicBlockList().push_back(MergeBB);
        gen.builder.SetInsertPoint(MergeBB);
        return MergeBB;
    }
};

class ForStmtAST : public StatementAST {
    std::string m_Var;
    std::unique_ptr<ExprAST> m_Start, m_End;
    std::unique_ptr<NumberExprAST> m_Step;
    std::unique_ptr<AST> m_Body;

public:
    ForStmtAST(const std::string &Var, std::unique_ptr<ExprAST> Start,
               std::unique_ptr<ExprAST> End, std::unique_ptr<NumberExprAST> Step,
               std::unique_ptr<AST> Body)
            : m_Var(Var), m_Start(std::move(Start)), m_End(std::move(End)),
              m_Step(std::move(Step)), m_Body(std::move(Body)) {}

    void print(std::ostream &out, int indent = 0) const override {
        out << std::string(indent, ' ') << "{\n";
        out << std::string(indent + 2, ' ') << "\"type\": \"FOR\",\n";
        m_Start->print(out, indent + 2);
        m_End->print(out, indent + 2);
        m_Step->print(out, indent + 2);
        m_Body->print(out, indent + 2);
        out << std::string(indent + 2, ' ') << "}";
    }

    llvm::Value *codegen(GenContext & gen) override {
        llvm::Value *StartVal = m_Start->codegen(gen);
        if (!StartVal)
            return nullptr;

        // Make the new basic block for the loop header, inserting after current
// block.
        llvm::AllocaInst * VariableAlloca = gen.symbTable.find(m_Var)->second.store;
        gen.builder.CreateStore(StartVal, VariableAlloca);

        llvm::Function *TheFunction = gen.builder.GetInsertBlock()->getParent();

        llvm::BasicBlock *ConditionBB =
                llvm::BasicBlock::Create(gen.ctx);
        llvm::BasicBlock *LoopBB =
                llvm::BasicBlock::Create(gen.ctx);
        llvm::BasicBlock *ExitBB = llvm::BasicBlock::Create(gen.ctx);

        gen.builder.CreateBr(ConditionBB);
        gen.builder.SetInsertPoint(ConditionBB);

//        For I := 0 to 20 do then
//        m_End = <ExprAST>(20)
        llvm::Value *EndCond = m_End->codegen(gen);
        if (!EndCond)
            return nullptr;

        // Convert condition to a bool by comparing non-equal to 0.0.
        TheFunction->getBasicBlockList().push_back(ConditionBB);

        VariableAlloca = gen.symbTable.find(m_Var)->second.store;
        llvm::Value * VariableValue = gen.builder.CreateLoad(llvm::Type::getInt32Ty(gen.ctx), VariableAlloca, "for_assign");
        llvm::Value * condition = gen.builder.CreateICmpSLE(
                VariableValue, EndCond
        );

        gen.builder.CreateCondBr(condition, LoopBB, ExitBB);

        TheFunction->getBasicBlockList().push_back(LoopBB);

        gen.builder.SetInsertPoint(LoopBB);

        m_Body->codegen(gen);


        llvm::Value *StepVal = nullptr;
        if (m_Step) {
            StepVal = m_Step->codegen(gen);
            if (!StepVal)
                return nullptr;
        }
        VariableAlloca = gen.symbTable.find(m_Var)->second.store;
        VariableValue = gen.builder.CreateLoad(llvm::Type::getInt32Ty(gen.ctx), VariableAlloca, "for_assign");
        llvm::Value * NextVal = gen.builder.CreateAdd(VariableValue, StepVal, "nextvar");
        gen.builder.CreateStore(NextVal, VariableAlloca);

        gen.builder.CreateBr(ConditionBB);


        TheFunction->getBasicBlockList().push_back(ExitBB);
        gen.builder.SetInsertPoint(ExitBB);

        return nullptr;
    };
};

class WhileStmtAST : public StatementAST {
    std::unique_ptr<ExprAST> m_Cond;
    std::unique_ptr<AST> m_Body;

public:
    WhileStmtAST(std::unique_ptr<ExprAST> cond, std::unique_ptr<AST> body)
            : m_Cond(std::move(cond)), m_Body(std::move(body)) {}

    void print(std::ostream &out, int indent = 0) const override {
        out << std::string(indent, ' ') << "{\n";
        out << std::string(indent + 2, ' ') << "\"type\": \"WHILE\",\n";
        m_Cond->print(out, indent + 2);
        m_Body->print(out, indent + 2);
        out << std::string(indent, ' ') << "}";
    }

    llvm::Value *codegen(GenContext &gen) override {
        llvm::Function *TheFunction = gen.builder.GetInsertBlock()->getParent();

        llvm::BasicBlock *CondBB = llvm::BasicBlock::Create(gen.ctx, "whilecond", TheFunction);
        llvm::BasicBlock *LoopBB = llvm::BasicBlock::Create(gen.ctx, "whileloop" );
        llvm::BasicBlock *ExitBB = llvm::BasicBlock::Create(gen.ctx, "whileexit");

        gen.builder.CreateBr(CondBB);
        gen.builder.SetInsertPoint(CondBB);

        llvm::Value *CondV = m_Cond->codegen(gen);
        if (!CondV)
            return nullptr;

        CondV = gen.builder.CreateICmpNE(CondV, llvm::ConstantInt::get(CondV->getType(), 0, true), "whilecond");
        gen.builder.CreateCondBr(CondV, LoopBB, ExitBB);

        TheFunction->getBasicBlockList().push_back(LoopBB);
        gen.builder.SetInsertPoint(LoopBB);

        m_Body->codegen(gen);

        gen.builder.CreateBr(CondBB);

        TheFunction->getBasicBlockList().push_back(ExitBB);
        gen.builder.SetInsertPoint(ExitBB);

        return nullptr;
    }
};




#endif // MILA_AST_HPP

//int main() {
//    auto lhs = std::make_unique<VariableExprAST>("a");
//    auto rhs = std::make_unique<NumberExprAST>(5.0);
//    auto assign = std::make_unique<AssignmentAST>(std::move(lhs), std::move(rhs));
//
//    std::clog << *assign << std::endl;
//
//    return 0;
//}