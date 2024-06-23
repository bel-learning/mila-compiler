//
// Created by Bilguudei Baljinnyam on 14.06.2024.
//
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
    llvm::AllocaInst* store; };
//using SymbolTable = std::map<std::string, Symbol>;

struct SymbolTable {
public:
    SymbolTable() {
        newScope();
    };
    void newScope() {
        pages.push_front(std::map<std::string, Symbol>());
    }
    void deleteScope() {
        pages.pop_front();
    }
    Symbol * search(const std::string & var) {
       for(auto it = pages.begin(); it != pages.end(); it++) {
           std::map<std::string, Symbol> & page = *it;
           auto searchIter = page.find(var);
           if(searchIter != page.end()) {
               return &page.at(var);
           }
       }

       return nullptr;
    }
    Symbol & operator [] (const std::string & var) {
        return pages.front()[var];
    }
    void erase(const std::string & var) {
        pages.front().erase(var);
    }


    std::deque<std::map<std::string, Symbol> > pages;
};

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
        Symbol * symb = gen.symbTable.search(m_Var);
        if (symb == nullptr) {
            throw std::runtime_error("Unknown variable name: " + m_Var);
        }

        // Return the stored LLVM Value for the variable
        llvm::Value * val = gen.builder.CreateLoad(llvm::Type::getInt32Ty(gen.ctx), symb->store);

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
        gen.symbTable[m_var] = { alloca };

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
                return gen.builder.CreateIntCast(L, llvm::Type::getInt32Ty(gen.ctx), false, "booltmp");
            case '>':
                L = gen.builder.CreateICmpSGT(L, R, "cmptmp");
                // Convert bool 0/1 to int 0 or 1
                return gen.builder.CreateIntCast(L, llvm::Type::getInt32Ty(gen.ctx), false, "booltmp");
            case '=':
                L = gen.builder.CreateICmpEQ(L, R, "cmptmp");
                // Convert bool 0/1 to int 0 or 1
                return gen.builder.CreateIntCast(L, llvm::Type::getInt32Ty(gen.ctx), false, "booltmp");
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
    llvm::Value * codegen(GenContext& gen) override {
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
            std::clog << "Parsing argument: " << std::endl;
            Args[i]->print(std::clog);

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
        llvm::CallInst *call = gen.builder.CreateCall(calleeF, argsV);

        // Check if the callee function returns void
        if (calleeF->getReturnType()->isVoidTy()) {
            std::clog << "Callee no return type: " << Callee << std::endl;
            return nullptr; // No value to return for void functions
        } else {
            return call; // Return the call instruction for non-void functions
        }
    };
};

/// PrototypeAST - This class represents the "prototype" for a function,
/// which captures its name, and its argument names (thus implicitly the number
/// of arguments the function takes).
class PrototypeAST  {
    std::string m_Name;
    std::vector<std::string> m_Args;

public:
    PrototypeAST(const std::string &Name, std::vector<std::string> Args)
            : m_Name(Name), m_Args(std::move(Args)) {}

    const std::string &getName() const { return m_Name; }

    void print(std::ostream &out, int indent = 0) const {
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
        llvm::FunctionType * FT = llvm::FunctionType::get(llvm::Type::getInt32Ty(gen.ctx), INTS ,false);
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
    std::unique_ptr<AST> m_Body;

public:
    FunctionAST(std::unique_ptr<PrototypeAST> Proto, std::unique_ptr<AST> Body)
            : m_Proto(std::move(Proto)), m_Body(std::move(Body)) {}

    void print(std::ostream &out, int indent = 0) const {
        out << std::string(indent, ' ') << "{\n";
        out << std::string(indent + 2, ' ') << "\"type\": \"FunctionAST\",\n";
        out << std::string(indent + 2, ' ') << "\"prototype\": ";
        m_Proto->print(out, indent + 2);
        out << ",\n";
        out << std::string(indent + 2, ' ') << "\"body\": ";
        m_Body->print(out, indent + 2);
        out << "\n" << std::string(indent, ' ') << "}";
    }
    llvm::Value * codegen(GenContext& gen) override {
        llvm::Function *TheFunction = gen.module.getFunction(m_Proto->getName());

        if (!TheFunction)
            TheFunction = m_Proto->codegen(gen);
        if (!TheFunction)
            return nullptr;
        if (!TheFunction->empty())
            throw std::runtime_error("Function cannot be redefined");

        llvm::BasicBlock * BB = llvm::BasicBlock::Create(gen.ctx, "entry", TheFunction);

        gen.builder.SetInsertPoint(BB);

        for (auto &Arg : TheFunction->args()) {
            // Create an alloca for this variable
            llvm::AllocaInst *Alloca = gen.builder.CreateAlloca(Arg.getType(), nullptr, Arg.getName());
            // Store the initial value into the alloca
            gen.builder.CreateStore(&Arg, Alloca);
            // Add the variable to the symbol table
            gen.symbTable[std::string(Arg.getName())] = {Alloca};
        }

        // Generate code for the function body
        if (llvm::Value *RetVal = m_Body->codegen(gen)) {
            // If the function has a non-void return type, generate the return instruction
            if (TheFunction->getReturnType()->isVoidTy()) {
                gen.builder.CreateRetVoid();
            } else {
                gen.builder.CreateRet(RetVal);
            }

            // Validate the generated code, checking for consistency
            llvm::verifyFunction(*TheFunction);

            return TheFunction;
        }

        // Error reading body, remove the function
        TheFunction->eraseFromParent();
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
        m_Else->print(out, indent + 2);
        m_Then->print(out, indent + 2);
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
        llvm::Value *ThenV = m_Then->codegen(gen);
        if (!ThenV) return nullptr;
        gen.builder.CreateBr(MergeBB);

        ThenBB = gen.builder.GetInsertBlock();

        if(m_Else) {
            TheFunction->getBasicBlockList().push_back(ElseBB);
            gen.builder.SetInsertPoint(ElseBB);
            llvm::Value *ElseV = m_Else->codegen(gen);
            if(!ElseV) return nullptr;

            // Add the else block to the function
            ElseBB = gen.builder.GetInsertBlock();
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
        llvm::Function *TheFunction = gen.builder.GetInsertBlock()->getParent();
        llvm::BasicBlock *PreheaderBB = gen.builder.GetInsertBlock();
        llvm::BasicBlock *LoopBB =
                llvm::BasicBlock::Create(gen.ctx, "loop", TheFunction);

        gen.builder.CreateBr(LoopBB);

        gen.builder.SetInsertPoint(LoopBB);

        llvm::PHINode * Variable = gen.builder.CreatePHI(llvm::Type::getInt32Ty(gen.ctx), 2, m_Var);
        Variable->addIncoming(StartVal, PreheaderBB);

        llvm::Value * oldVal = gen.symbTable.search(m_Var)->store;
        llvm::AllocaInst *Alloca = gen.builder.CreateAlloca(StartVal->getType(), nullptr, m_Var);
        // Store the initial value into the alloca.
        gen.builder.CreateStore(StartVal, Alloca);

        if (!m_Body->codegen(gen))
            return nullptr;

        llvm::Value *StepVal = nullptr;
        if (m_Step) {
            StepVal = m_Step->codegen(gen);
            if (!StepVal)
                return nullptr;
        }
        llvm::Value * NextVal = gen.builder.CreateAdd(Variable, StepVal, "nextvar");

        // Compute the end condition.
        llvm::Value *EndCond = m_End->codegen(gen);
        if (!EndCond)
            return nullptr;

        // Convert condition to a bool by comparing non-equal to 0.0.

        EndCond = gen.builder.CreateICmpEQ(
                EndCond, NextVal, "loopcmp"
                );

        // Create the "after loop" block and insert it.
        llvm::BasicBlock *LoopEndBB = gen.builder.GetInsertBlock();
        llvm::BasicBlock *AfterBB =
                llvm::BasicBlock::Create(gen.ctx, "afterloop", TheFunction);

        gen.builder.CreateCondBr(EndCond, LoopBB, AfterBB);
        gen.builder.SetInsertPoint(AfterBB);

        // Add a new entry to the PHI node for the backedge.
        Variable->addIncoming(NextVal, LoopEndBB);

        // Restore the unshadowed variable.
        if (oldVal)
            gen.symbTable[m_Var] = {Alloca};
        else
            gen.symbTable.erase(m_Var);

        return nullptr;
    };
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