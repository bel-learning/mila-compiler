//
// Created by Bilguudei Baljinnyam on 14.06.2024.
//
#include <string>
#include <utility>
#include <vector>
#include <iostream>
#include <memory>

#ifndef MILA_AST_HPP
#define MILA_AST_HPP

class AST {
public:
    virtual ~AST() = default;
    virtual void print(std::ostream &out, int indent = 0) const = 0;

    friend std::ostream &operator<<(std::ostream &out, const AST &ast) {
        ast.print(out);
        return out;
    }
};

class StatementAST : public AST {
public:
    virtual ~StatementAST() = default;
};

class BlockAST : public AST {
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
};

class NumberExprAST : public AST {
    double m_Val;
public:
    NumberExprAST(double val) : m_Val(val) {}

    void print(std::ostream &out, int indent = 0) const override {
        out << std::string(indent, ' ') << "{\n";
        out << std::string(indent + 2, ' ') << "\"type\": \"NumberExprAST\",\n";
        out << std::string(indent + 2, ' ') << "\"value\": " << m_Val << "\n";
        out << std::string(indent, ' ') << "}";
    }
};

/// VariableExprAST - Expression class for referencing a variable, like "a".
class VariableExprAST : public AST {
    std::string Name;

public:
    VariableExprAST(std::string Name) : Name(std::move(Name)) {}

    void print(std::ostream &out, int indent = 0) const override {
        out << std::string(indent, ' ') << "{\n";
        out << std::string(indent + 2, ' ') << "\"type\": \"VariableExprAST\",\n";
        out << std::string(indent + 2, ' ') << "\"name\": \"" << Name << "\"\n";
        out << std::string(indent, ' ') << "}";
    }
};

class AssignmentAST : public AST {
    std::unique_ptr<AST> m_LHS;
    std::unique_ptr<AST> m_RHS;
public:
    AssignmentAST(std::unique_ptr<AST> LHS, std::unique_ptr<AST> RHS)
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
};

/// BinaryExprAST - Expression class for a binary operator.
class BinaryExprAST : public AST {
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
};

/// CallExprAST - Expression class for function calls.
class CallExprAST : public AST {
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
};

/// PrototypeAST - This class represents the "prototype" for a function,
/// which captures its name, and its argument names (thus implicitly the number
/// of arguments the function takes).
class PrototypeAST {
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
};

/// FunctionAST - This class represents a function definition itself.
class FunctionAST {
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
