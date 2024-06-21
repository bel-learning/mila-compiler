#include "Parser.hpp"

Parser::Parser()
    : gen("mila")
{
}

void Parser::printCurrentToken() {
    std::map<int, std::string> tokenMap = {
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
            {-33, "tok_range"},
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
            {'.', "."},
    };

    if(tokenMap.find(CurTok) == tokenMap.end()) {
        std::clog << "Not known token: " << CurTok << std::endl;
        return;
    };
    if(CurTok == TokenType::tok_identifier) {
        std::clog << "\'" << m_Lexer.identifierStr() << "\'" << ", ";
    }
    else if(CurTok == TokenType::tok_number) {
        std::clog << "\'" << m_Lexer.numVal() << "\'" << ", ";
    }
    else
        std::clog << "\'" << tokenMap[CurTok]  << "\'" << ", ";
}

bool Parser::Parse()
{
    getNextToken();
    consume(TokenType::tok_program);
    consume(TokenType::tok_identifier);
    consume(TokenType::tok_semicolon);
    // Main logic
    auto result = ParseModule();
    std::clog << *result << std::endl;
    m_AstTree = std::move(result);
    consume(TokenType::tok_dot);
    return true;
}
std::unique_ptr<AST> Parser::ParseModule() {
    // Expressions or BLOCK
    std::vector<std::unique_ptr<AST>> modules;
    while(true) {
        switch(CurTok) {
            case tok_dot:
                return std::make_unique<BlockAST>( std::move(modules) );
            case tok_semicolon:
                getNextToken();
                break;
            case tok_begin:
                modules.push_back(ParseBlock());
                break;
            case tok_const:
            case tok_var:
                modules.push_back(ParseDeclaration());
                break;
            default:
                std::clog << "Unexpected token" << std::endl;
                return nullptr;
        }
    }
    return nullptr;
}

// Begin
// End
std::unique_ptr<AST> Parser::ParseBlock() {

    consume(TokenType::tok_begin);
    std::vector<std::unique_ptr<AST>> body;

    while(CurTok != TokenType::tok_end) {
//        auto res = Parse
        switch(CurTok) {
            case TokenType::tok_semicolon:
                getNextToken();
                continue;
                // L5 -> L4 = L5 <- right->left
                // A = B  A + B  C * D  W / R
            case TokenType::tok_identifier:
//                ParseAssignmentExpr();
                body.push_back(std::move( ParseIdentifierExpr() ));
                break;
            case TokenType::tok_begin:
                body.push_back(std::move( ParseBlock() ));
                break;
            case TokenType::tok_while:
            case TokenType::tok_if:
            case TokenType::tok_for:
                ParseStatement();
        }
    }
    consume(tok_end);

    return std::make_unique<BlockAST>( std::move(body) );
};


// CONST block -> VAR block, Func declare
std::unique_ptr<AST> Parser::ParseDeclaration() {
    std::vector<std::unique_ptr<AST> > vars;
    if(CurTok == tok_const) {
        consume(tok_const);
        // Const variable name
        while(CurTok == tok_identifier) {
            std::string idName = m_Lexer.identifierStr();
            consume(tok_identifier);
            // Todo: needs to parse multiple var with , , , ,
            consume('=');

            std::unique_ptr<ExprAST> expr = ParseExpression();
            std::unique_ptr<TypeAST> type = std::make_unique<TypeAST>(TypeAST::Type::INT);

            vars.push_back(std::make_unique<VarDeclAST>(idName,
                                                        std::move(type),
                                                        std::move(expr),
                                                        true));
            consume(';');
        }
    }
    if(CurTok == tok_var) {
        consume(tok_var);
        // Const variable name
        while(CurTok == tok_identifier) {
            std::string idName = m_Lexer.identifierStr();
            consume(tok_identifier);
            // Todo: needs to parse multiple var with , , , ,
            consume(':');

            // Can be extended
            TypeAST::Type typeValue;
            if (CurTok == tok_integer) {
                typeValue = TypeAST::Type::INT;
            }
            consume(tok_integer);

            std::unique_ptr<TypeAST> type = std::make_unique<TypeAST>(typeValue);

            std::unique_ptr<ExprAST> expr = nullptr;

            vars.push_back(std::make_unique<VarDeclAST>(idName,
                                                        std::move(type),
                                                        std::move(expr),
                                                        false));
            consume(';');
        }
    }

    return std::make_unique<BlockAST>( std::move(vars) );
};

std::unique_ptr<ExprAST> Parser::ParseAssignmentExpr(std::string & idLHS) {
    auto LHS = std::make_unique<DeclRefAST>(idLHS);
    consume(TokenType::tok_assign);
    auto RHS = ParseExpression();

    return std::make_unique<AssignmentAST>(std::move(LHS), std::move(RHS));
}


/**
 * @brief Simple token buffer.
 *
 * CurTok is the current token the parser is looking at
 * getNextToken reads another token from the lexer and updates curTok with ts result
 * Every function in the parser will assume that CurTok is the cureent token that needs to be parsed
 */
int Parser::getNextToken()
{
    return CurTok = m_Lexer.gettok();
}

std::unique_ptr<AST> Parser::ParseStatement() {
    return ParsePrimary();
}

std::unique_ptr<ExprAST> Parser::ParseExpression() {
    // With program and others and then begin.
    auto LHS = ParsePrimary();
//    std::clog << "Parse primary: " << std::endl;
//    LHS->print(std::clog);
    if (!LHS)
        return nullptr;
    // If there is no right hand side
    if(CurTok == ';') return LHS;
    return ParseBinOpRHS(0, std::move(LHS));
}

std::unique_ptr<ExprAST> Parser::ParsePrimary() {
    switch (CurTok) {
        default:
            return LogError("unknown token when expecting an expression from ParsePrimary");
        case tok_identifier:
            return ParseIdentifierExpr();
        case tok_number:
            return ParseNumberExpr();
        case '(':
            return ParseParenExpr();
    }
}


std::unique_ptr<ExprAST> Parser::ParseBinOpRHS(int ExprPrec,
                                       std::unique_ptr<ExprAST> LHS) {
    // If this is a binop, find its precedence.
//    std::clog << "Parsing Binary expression" << std::endl;
//    std::clog << "LHS: " << std::endl;
//    LHS->print(std::clog);
    while (true) {
        int TokPrec = GetTokenPrecedence();

        // If this is a binop that binds at least as tightly as the current binop,
        // consume it, otherwise we are done.
        if (TokPrec < ExprPrec)
            return LHS;

        // Okay, we know this is a binop.
        int BinOp = CurTok;
        getNextToken(); // eat binop

        // Parse the primary expression after the binary operator.
        auto RHS = ParsePrimary();
//        std::clog << "RHS: " << std::endl;
//        RHS->print(std::clog);
        if (!RHS)
            return nullptr;

        // If BinOp binds less tightly with RHS than the operator after RHS, let
        // the pending operator take RHS as its LHS.
        int NextPrec = GetTokenPrecedence();
        if (TokPrec < NextPrec) {
            RHS = ParseBinOpRHS(TokPrec + 1, std::move(RHS));
            if (!RHS)
                return nullptr;
        }
        // Merge LHS/RHS.
        LHS = std::make_unique<BinaryExprAST>(BinOp, std::move(LHS), std::move(RHS));
    }
}


/// numberexpr ::= number
std::unique_ptr<ExprAST> Parser::ParseNumberExpr() {
    auto result = std::make_unique<NumberExprAST>(m_Lexer.numVal());
    consume(tok_number); // consume the number
    return std::move(result);
}

std::unique_ptr<ExprAST> Parser::ParseParenExpr() {
    consume(tok_lparen);
    auto result = ParseExpression();
    if (!result)
        return nullptr;
    result->print(std::clog);
    consume(TokenType::tok_rparen);

    return result;
}

/// identifierexpr
///   ::= identifier
///   ::= identifier '(' expression* ')'
std::unique_ptr<ExprAST> Parser::ParseIdentifierExpr() {
    std::string idName = m_Lexer.identifierStr();

    consume(TokenType::tok_identifier);  // eat identifier.

    if (CurTok != '(') {
        // + -
        // bap - 1;
        if(CurTok == tok_assign) {
            return ParseAssignmentExpr(idName);
        }
        return std::make_unique<DeclRefAST>(idName);
    }  // Simple variable ref.

    // Call.
    consume(TokenType::tok_lparen);

    std::vector<std::unique_ptr<AST>> Args;
    if (CurTok != ')') {
        while (true) {
            if (auto Arg = ParseExpression())
                Args.push_back(std::move(Arg));
            else
                return nullptr;

            if (CurTok == ')')
                break;

            if (CurTok != ',')
                return nullptr;
            getNextToken();
        }
    }

    consume(tok_rparen);
    consume(tok_semicolon);

    return std::make_unique<CallExprAST>(idName, std::move(Args));
}

/**
 * @brief Simple consumer.
 *
 * Checks if the token we want to consume is equal to CurTok and consumes it.
 */
bool Parser::consume(int token) {

    std::clog << "Eat: ";
    PrintToken(CurTok);
    std::clog << std::endl;
    if(token != CurTok) {
        std::clog << "Expected token: " << ReturnTokenString(token) << " with current: "
        << ReturnTokenString(CurTok) << std::endl;
        return false;
    }
    getNextToken();

    std::clog << "Next: ";
    PrintToken(CurTok);
    std::clog << std::endl;

    return true;
};

// initialize lexer

int Parser::GetTokenPrecedence() {
    if (!isascii(CurTok))
        return -1;

    // Make sure it's a declared binop.
    int TokPrec = BinopPrecedence[CurTok];
    if (TokPrec <= 0) return -1;
    return TokPrec;
}

void Parser::PrintToken(int token) {
    if(tokenMap.find(token) == tokenMap.end()) {
        std::clog << "Not known token: " << token << std::endl;
        return;
    };
    if(CurTok == TokenType::tok_identifier) {
        std::clog << ">" << m_Lexer.identifierStr() << "<";
    }
    else
        std::clog << ">" << tokenMap[token]  << "<";
}
std::string Parser::ReturnTokenString(int token) {
    if(tokenMap.find(token) == tokenMap.end()) {
        std::clog << "Not known token: " << token << std::endl;
        return "";
    };
    if(CurTok == TokenType::tok_identifier) {
        return m_Lexer.identifierStr();
    }
    else
        return tokenMap[token];
}

std::unique_ptr<ExprAST> Parser::LogError(const char *string) {
    fprintf(stderr, "Error: %s\n", string);
    return nullptr;
}



const llvm::Module& Parser::Generate()
{

    // create writeln function

    {
        std::vector<llvm::Type*> Ints(1, llvm::Type::getInt32Ty(gen.ctx));
        llvm::FunctionType * FT = llvm::FunctionType::get(llvm::Type::getVoidTy(gen.ctx), Ints, false);
        llvm::Function * F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "writeln", gen.module);
        for (auto & Arg : F->args())
            Arg.setName("x");


    }
    {
        std::vector<llvm::Type*> Ints(1, llvm::Type::getInt32PtrTy(gen.ctx));
        llvm::FunctionType * FT = llvm::FunctionType::get(llvm::Type::getInt32Ty(gen.ctx), Ints, false);
        llvm::Function * F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "readln", gen.module);
        for (auto & Arg : F->args())
            Arg.setName("x");
    }

    // create main function
    {
        llvm::FunctionType * FT = llvm::FunctionType::get(llvm::Type::getInt32Ty(gen.ctx), false);
        llvm::Function * MainFunction = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "main", gen.module);

        // block
        llvm::BasicBlock * BB = llvm::BasicBlock::Create(gen.ctx, "entry", MainFunction);
        gen.builder.SetInsertPoint(BB);

        m_AstTree->codegen(gen);

        // call writeln with value from lexel
//        gen.builder.CreateCall(gen.module.getFunction("writeln"), {
//                llvm::ConstantInt::get(gen.ctx, llvm::APInt(32, m_Lexer.numVal()))
//        });

        // return 0
        gen.builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(gen.ctx), 0));
    }


    return this->gen.module;
}