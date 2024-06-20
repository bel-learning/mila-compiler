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

#ifndef MILA_AST_HPP
#define MILA_AST_HPP

class TypeAST;

struct Symbol { llvm::AllocaInst* store; TypeAST* type; };
using SymbolTable = std::map<std::string, Symbol>;

struct GenContext {
    GenContext(const std::string& moduleName);

    llvm::LLVMContext * ctx;
    llvm::IRBuilder<> * builder;
    llvm::Module * module;

    SymbolTable * symbTable;
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
    virtual void print(std::ostream &out, int indent = 0) const = 0;
    llvm::Value * codegen(GenContext& gen) override = 0;
};

class StatementAST : public AST {
public:
    virtual ~StatementAST() = default;
    virtual void print(std::ostream &out, int indent = 0) const = 0;
    llvm::Value * codegen(GenContext& gen) override = 0;
};

class BlockAST : public StatementAST {
    std::vector<std::unique_ptr<AST>> m_Body;
public:
    BlockAST(std::vector<std::unique_ptr<AST>> body) : m_Body(std::move(body)) {}
    virtual ~BlockAST() = default;

    void print(std::ostream &out, int indent = 0) const override {
        out << std::string(indent, ' ') << "{\n";
        for (const auto &stmt : m_Body) {
            stmt->print(out, indent + 2);
            out << ",\n";
        }
        out << std::string(indent, ' ') << "}";
    }
    llvm::Value * codegen(GenContext& gen) override {

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
    llvm::Value * codegen(GenContext& gen) override;
private:
    Type m_type;
};

class NumberExprAST : public ExprAST {
    double m_Val;
public:
    NumberExprAST(double val) : m_Val(val) {}

    void print(std::ostream &out, int indent = 0) const override {
        out << std::string(indent, ' ') << "{\n";
        out << std::string(indent + 2, ' ') << "\"type\": \"NumberExprAST\",\n";
        out << std::string(indent + 2, ' ') << "\"value\": " << m_Val << "\n";
        out << std::string(indent, ' ') << "}";
    }
    llvm::Value * codegen(GenContext& gen) override {
        llvm::ConstantInt::get(llvm::Type::getInt32Ty(*gen.ctx), m_Val, true);
    }
};

class AssignmentAST : public ExprAST {
    std::unique_ptr<ExprAST> m_LHS;
    std::unique_ptr<ExprAST> m_RHS;
public:
    AssignmentAST(std::unique_ptr<ExprAST> LHS, std::unique_ptr<ExprAST> RHS)
            : m_LHS(std::move(LHS)), m_RHS(std::move(RHS)) {}

    void print(std::ostream &out, int indent = 0) const override {
        out << std::string(indent, ' ') << "{\n";
        out << std::string(indent + 2, ' ') << "\"type\": \"AssignmentAST\",\n";
        out << std::string(indent + 2, ' ') << "\"LHS\": ";
        m_LHS->print(out, indent + 2);
        out << ",\n";
        out << std::string(indent + 2, ' ') << "\"RHS\": ";
        m_RHS->print(out, indent + 2);
        out << "\n" << std::string(indent, ' ') << "}";
    }
    llvm::Value * codegen(GenContext& gen) override;
};


class DeclRefAST : public ExprAST {
    std::string m_Var;
public:
    DeclRefAST(std::string var): m_Var(var) {};
    void print(std::ostream &out, int indent = 0) const override {
        out << std::string(indent, ' ') << "{\n";
        out << std::string(indent + 2, ' ') << "\"type\": \"DeclRefASTNode\",\n";
        out << std::string(indent + 2, ' ') << "\"name\": \"" << m_Var << "\"\n";
        out << std::string(indent, ' ') << "}";
    };
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
    VarDeclAST(std::string var, std::unique_ptr<TypeAST> type, std::unique_ptr<ExprAST> expr, bool constant) :
        m_var(var), m_type(std::move(type)), m_expr(std::move(expr)), m_constant(constant) {};
    void print(std::ostream &out, int indent = 0) const override {
        out << std::string(indent, ' ') << "{\n";
        out << std::string(indent + 2, ' ') << "\"type\": \"VarDeclAST\",\n";
        out << std::string(indent + 2, ' ') << "\"mvar\": "<< m_var  << "\"VarDeclAST\",\n";
//        out << std::string(indent + 2, ' ') << "\"operator\": \"" << Op << "\",\n";
        out << "\n" << std::string(indent, ' ') << "}";
    };
    llvm::Value * codegen(GenContext& gen) override;
//    llvm::Value* codegen(GenContext& gen) const override;
};

/// BinaryExprAST - Expression class for a binary operator.
class BinaryExprAST : public ExprAST {
    char Op;
    std::unique_ptr<AST> LHS, RHS;

public:
    BinaryExprAST(char Op, std::unique_ptr<AST> LHS, std::unique_ptr<AST> RHS)
            : Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}

    void print(std::ostream &out, int indent = 0) const override {
        out << std::string(indent, ' ') << "{\n";
        out << std::string(indent + 2, ' ') << "\"type\": \"BinaryExprAST\",\n";
        out << std::string(indent + 2, ' ') << "\"operator\": \"" << Op << "\",\n";
        out << std::string(indent + 2, ' ') << "\"LHS\": ";
        LHS->print(out, indent + 2);
        out << ",\n";
        out << std::string(indent + 2, ' ') << "\"RHS\": ";
        RHS->print(out, indent + 2);
        out << "\n" << std::string(indent, ' ') << "}";
    }
    llvm::Value * codegen(GenContext& gen) override;
};

/// CallExprAST - Expression class for function calls.
class CallExprAST : public ExprAST {
    std::string Callee;
    std::vector<std::unique_ptr<AST>> Args;

public:
    CallExprAST(std::string Callee, std::vector<std::unique_ptr<AST>> Args)
            : Callee(std::move(Callee)), Args(std::move(Args)) {}

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
    llvm::Value * codegen(GenContext& gen) override;
};

/// PrototypeAST - This class represents the "prototype" for a function,
/// which captures its name, and its argument names (thus implicitly the number
/// of arguments the function takes).
class PrototypeAST : StatementAST {
    std::string Name;
    std::vector<std::string> Args;

public:
    PrototypeAST(const std::string &Name, std::vector<std::string> Args)
            : Name(Name), Args(std::move(Args)) {}

    const std::string &getName() const { return Name; }

    void print(std::ostream &out, int indent = 0) const {
        out << std::string(indent, ' ') << "{\n";
        out << std::string(indent + 2, ' ') << "\"type\": \"PrototypeAST\",\n";
        out << std::string(indent + 2, ' ') << "\"name\": \"" << Name << "\",\n";
        out << std::string(indent + 2, ' ') << "\"args\": [\n";
        for (const auto &arg : Args) {
            out << std::string(indent + 4, ' ') << "\"" << arg << "\",\n";
        }
        out << std::string(indent + 2, ' ') << "]\n";
        out << std::string(indent, ' ') << "}";
    }
    llvm::Value * codegen(GenContext& gen) override;
};

/// FunctionAST - This class represents a function definition itself.
class FunctionAST : public StatementAST {
    std::unique_ptr<PrototypeAST> Proto;
    std::unique_ptr<AST> Body;

public:
    FunctionAST(std::unique_ptr<PrototypeAST> Proto, std::unique_ptr<AST> Body)
            : Proto(std::move(Proto)), Body(std::move(Body)) {}

    void print(std::ostream &out, int indent = 0) const {
        out << std::string(indent, ' ') << "{\n";
        out << std::string(indent + 2, ' ') << "\"type\": \"FunctionAST\",\n";
        out << std::string(indent + 2, ' ') << "\"prototype\": ";
        Proto->print(out, indent + 2);
        out << ",\n";
        out << std::string(indent + 2, ' ') << "\"body\": ";
        Body->print(out, indent + 2);
        out << "\n" << std::string(indent, ' ') << "}";
    }
    llvm::Value * codegen(GenContext& gen) override;
};

#endif // MILA_AST_HPP

//int main() {
//    auto lhs = std::make_unique<VariableExprAST>("a");
//    auto rhs = std::make_unique<NumberExprAST>(5.0);
//    auto assign = std::make_unique<AssignmentAST>(std::move(lhs), std::move(rhs));
//
//    std::cout << *assign << std::endl;
//
//    return 0;
//}
