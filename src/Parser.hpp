#ifndef PJPPROJECT_PARSER_HPP
#define PJPPROJECT_PARSER_HPP


#include <fstream>

#include "Lexer.hpp"
#include "AST.hpp"


static std::map<int, std::string> tokenMap = {
        {-1, "tok_eof"},
        {-2, "tok_identifier"},
        {-3, "tok_number"},
        {-4, "tok_begin"},
        {-5, "tok_end"},
        {-6, "tok_const"},
        {-7, "tok_procedure"},
        {-8, "tok_forward"},
        {-9, "tok_function"},
        {-10, "tok_if"},
        {-11, "tok_then"},
        {-12, "tok_else"},
        {-13, "tok_program"},
        {-14, "tok_while"},
        {-15, "tok_exit"},
        {-16, "tok_var"},
        {-17, "tok_integer"},
        {-18, "tok_for"},
        {-19, "tok_do"},
        {-20, "!="},
        {-21, "<="},
        {-22, ">="},
        {-23, ":="},
        {-24, "||"},
        {-25, "tok_mod"},
        {-26, "tok_div"},
        {-27, "tok_not"},
        {-28, "tok_and"},
        {-29, "tok_xor"},
        {-30, "tok_to"},
        {-31, "tok_downto"},
        {-32, "tok_array"},
        {'+', "+"},
        {'-', "-"},
        {'*', "*"},
        {'/', "/"},
        {'(', "("},
        {')', ")"},
        {'{', "{"},
        {'}', "}"},
        {'[', "["},
        {']', "]"},
        {',', ","},
        {';', ";"},
        {'>', ">"},
        {'<', "<"},
        {'=', "="},
        {'!', "!"},
        {'|', "|"},
        {'&', "&"},
        {'^', "^"},
        {':', ":"},
        {'.', "tok_dot"}
};

static std::map<char, int> BinopPrecedence = {
        {'<', 10},
        {'+', 20},
        {'-', 20},
        {'*', 40},
        {'/', 40},
        {tok_not, 5},
        {tok_mod, 40},
        {tok_div, 40},
        {tok_and, 80},
        {tok_xor, 90},

};

class Parser {
public:
    Parser();
    ~Parser() = default;

    // Program identifier;
    bool Parse();             // parse
    const llvm::Module& Generate();  // generate
    void printCurrentToken();
private:
    int getNextToken();
    Lexer m_Lexer;                   // lexer is used to read tokens
    int CurTok;                      // to keep the current token


    GenContext gen;

    std::unique_ptr<AST> m_AstTree;
    void printAST();
    bool consume(int token);

    // Root of the tree

    // Parse Statements
    // Statement
    // -> Declaration & Definition
    //      -> CONST
    //      -> VAR
    //      -> ARRAY
    // -> BLOCK Expression
    //      -> BLOCK of one line expressions
    //            -> Assignment
    //            -> one line expression (function call, etc...)

    std::unique_ptr<AST> ParseModule();

    std::unique_ptr<AST> ParseDeclaration(); // can be definition as well
    std::unique_ptr<AST> ParseBlock();
    std::unique_ptr<AST> ParseOneLineBlock();

    std::unique_ptr<AST> ParseStatement();
    std::unique_ptr<ExprAST> ParseExpression();
    std::unique_ptr<ExprAST> ParseAssignmentExpr(std::string & idLHS);

    std::unique_ptr<ExprAST> ParsePrimary();

    std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec,
                                               std::unique_ptr<ExprAST> LHS);
    std::unique_ptr<ExprAST> ParseNumberExpr();
    std::unique_ptr<ExprAST> ParseParenExpr();
    std::unique_ptr<ExprAST> ParseIdentifierExpr();

    std::unique_ptr<AST> ParseIfStmt();

    int GetTokenPrecedence();

    void PrintToken(int token);
    std::string ReturnTokenString(int token);
    std::unique_ptr<ExprAST> LogError(const char *string);

    };

#endif //PJPPROJECT_PARSER_HPP
