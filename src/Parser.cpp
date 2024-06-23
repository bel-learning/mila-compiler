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
            case tok_procedure:
            case tok_function:
                modules.push_back(ParseFunction());
                break;
            case tok_semicolon:
                getNextToken();
                break;
            default:
                modules.push_back(ParseMainModule());
        }
    }
    return nullptr;
}

std::unique_ptr<AST> Parser::ParseMainModule() {
    std::unique_ptr<PrototypeAST> prototype = std::make_unique<PrototypeAST>("main", std::vector<std::string>(), nullptr);
    std::vector<std::unique_ptr<VarDeclAST>> vars;
    ParseFunctionVarDeclaration(vars);

    std::unique_ptr<AST> body = ParseBlock();
    return std::make_unique<FunctionAST>(std::move(prototype), std::move(vars), std::move(body));
}

// Begin
// End

std::unique_ptr<PrototypeAST> Parser::ParsePrototype()
{
    int tokenType = CurTok;

    consume(tok_function);
    std::string idName = m_Lexer.identifierStr();
    std::unique_ptr<VarDeclAST> returnValue = nullptr;
    consume(tok_identifier);
//    #TODO take in more than integer
    consume('(');
    std::vector<std::string> parameters;
    while (CurTok != ')')
    {
        parameters.push_back(m_Lexer.identifierStr());
        consume(tok_identifier);
        consume(':');
        consume(tok_integer);
        if(CurTok == ')') break;
        consume(',');

    }
    consume(')');

    if (tokenType == tok_function)
    {
        consume(':');
        consume(tok_integer);
        consume(';');
        returnValue = std::make_unique<VarDeclAST>(idName, std::make_unique<TypeAST>(TypeAST::Type::INT), nullptr, false);
        return std::make_unique<PrototypeAST>(idName, std::move(parameters), std::move(returnValue));
    }
    else if (tokenType == tok_procedure)
    {
        getNextToken(); // eat ;
        return std::make_unique<PrototypeAST>(idName, std::move(parameters), nullptr);
    }
    return nullptr;
}

std::unique_ptr<AST> Parser::ParseFunction()
{
    std::unique_ptr<PrototypeAST> prototype =  ParsePrototype();
    std::vector<std::unique_ptr<VarDeclAST>> variables;
    if (CurTok == tok_forward)
    {
        consume(tok_forward);
        consume(tok_semicolon);
        return std::make_unique<FunctionAST>(std::move(prototype), std::move(variables), nullptr);
    }
    ParseFunctionVarDeclaration(variables);

    std::unique_ptr<AST> mainBlock = ParseBlock();
    consume(';');
    return std::make_unique<FunctionAST>(std::move(prototype), std::move(variables),  std::move(mainBlock));
}


std::unique_ptr<AST> Parser::ParseBlock() {

    consume(TokenType::tok_begin);
    std::vector<std::unique_ptr<AST>> body;

    while(CurTok != TokenType::tok_end) {
//        auto res = Parse
        switch(CurTok) {
            case TokenType::tok_semicolon:
                consume(tok_semicolon);
                continue;
                // L5 -> L4 = L5 <- right->left
                // A = B  A + B  C * D  W / R
            case TokenType::tok_number:
            case TokenType::tok_identifier:
                body.push_back(ParseExpression());
                consume(tok_semicolon);
                break;
            case TokenType::tok_begin:
                body.push_back(ParseBlock());
                break;
            case TokenType::tok_while:
                body.push_back(ParseWhileStmt());
                break;
            case TokenType::tok_if:
                body.push_back(ParseIfStmt());
                break;
            case TokenType::tok_for:
                body.push_back(ParseForStmt());
                break;
            case TokenType::tok_exit:
                consume(tok_exit);
                body.push_back(std::make_unique<FunctionExitAST>());
                break;
        }
    }
    consume(tok_end);

    return std::make_unique<BlockAST>( std::move(body) );
};

std::unique_ptr<AST> Parser::ParseOneLineBlock() {
    std::unique_ptr<ExprAST> res = nullptr;
    switch(CurTok) {
        case TokenType::tok_number:
            return ParseExpression();
        case TokenType::tok_identifier:
            res = ParseExpression();
            return res;
        case TokenType::tok_begin:
            return ParseBlock();
        case TokenType::tok_while:
            return ParseWhileStmt();
        case TokenType::tok_if:
            return ParseIfStmt();
        case TokenType::tok_for:
            return ParseForStmt();
        case TokenType::tok_exit:
            consume(tok_exit);
            return std::make_unique<FunctionExitAST>();
            break;
        default:
            throw std::runtime_error("ParseOneLineBlock with Token: " + ReturnTokenString(CurTok));
    }
    return nullptr;
};

void Parser::ParseFunctionVarDeclaration(std::vector<std::unique_ptr<VarDeclAST>> & vars) {
    while (CurTok == tok_const || CurTok == tok_var) {
        if (CurTok == tok_const) {
            consume(tok_const);
            // Const variable name
            while (CurTok == tok_identifier) {
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
        if (CurTok == tok_var) {
            consume(tok_var);
            // Const variable name
            while (CurTok == tok_identifier) {
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
    }

    return;
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
    auto res = ParseBinOpRHS(0, std::move(LHS));
    return res;
}

std::unique_ptr<ExprAST> Parser::ParsePrimary() {
    switch (CurTok) {
        default:
            std::clog << "tok: " << ReturnTokenString(CurTok) << std::endl;
            return LogError("unknown token when expecting an expression from ParsePrimary -  ");
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

        // It's not binary expression anymore
        if (TokPrec == -1)
            return LHS;
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
        return std::make_unique<DeclRefAST>(idName);
    }  // Simple variable ref.

    // Call.
    consume(TokenType::tok_lparen);

    std::vector<std::unique_ptr<ExprAST>> Args;
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

    return std::make_unique<CallExprAST>(idName, std::move(Args));
}

std::unique_ptr<AST> Parser::ParseIfStmt() {
    consume(tok_if);

    auto Cond = ParseExpression();
    if(!Cond) return nullptr;
    // It can be with begin or without begin
    std::unique_ptr<AST> Then = nullptr;
    consume(tok_then);
    if(CurTok == tok_begin)
        Then = ParseBlock();
    else
        Then = ParseOneLineBlock();
    if(!Then) return nullptr;

    while(CurTok == ';') consume(';');


    std::unique_ptr<AST> Else = nullptr;
    if(CurTok == tok_else) {
        consume(tok_else);
        if(CurTok == tok_begin)
            Else = ParseBlock();
        else
            Else = ParseOneLineBlock();
        if(!Else) return nullptr;
    }

    return std::make_unique<IfStmtAST>(std::move(Cond), std::move(Then), std::move(Else));

}

std::unique_ptr<AST> Parser::ParseForStmt() {
    consume(tok_for);

    if(CurTok != tok_identifier) {
        throw std::runtime_error("For loop, expected identifier");
    }
    std::string idName = m_Lexer.identifierStr();
    consume(tok_identifier);
    consume(tok_assign);

    auto Start = ParseExpression();
    std::clog << "START::::" << std::endl;
    Start->print(std::clog);
    std::unique_ptr<NumberExprAST> Step = nullptr;
    std::clog << "Next expected token: " << ReturnTokenString(CurTok) << std::endl;
    if(CurTok == tok_to) {
        Step = std::make_unique<NumberExprAST>(1);
        consume(tok_to);
    }
    else if(CurTok == tok_downto)  {
        Step = std::make_unique<NumberExprAST>(-1);
        consume(tok_downto);
    }
    else throw std::runtime_error("Excepted tok_down or tok_to");

    std::unique_ptr<ExprAST> End = ParseExpression();

    std::unique_ptr<AST> Body = nullptr;

    consume(tok_do);
    if(CurTok == tok_begin) {
        Body = ParseBlock();
        consume(tok_begin);
    } else {
        Body = ParseOneLineBlock();
    }
    return std::make_unique<ForStmtAST>(idName,std::move(Start), std::move(End), std::move(Step), std::move(Body));
}

std::unique_ptr<AST> Parser::ParseWhileStmt() {
    consume(tok_while);

    auto Expr =ParseExpression();

    consume(tok_do);
    auto Body = ParseBlock();

    return std::make_unique<WhileStmtAST>(std::move(Expr), std::move(Body));
};



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

    if (!isascii(CurTok) && CurTok != tok_mod && CurTok != tok_div && CurTok != tok_not
        && CurTok != tok_and && CurTok != tok_xor && CurTok != tok_assign && CurTok != tok_equal
        && CurTok != tok_lessequal && CurTok != tok_greaterequal && CurTok != tok_notequal && CurTok != tok_or
    )
        return -1;

    // Make sure it's a declared binop.
//    std::clog << "Getting token precedence: " << ReturnTokenString(CurTok) << std::endl;
    int TokPrec = BinopPrecedence[CurTok];
//    std::clog << "Precedence: " << TokPrec << std::endl;
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
//        llvm::FunctionType * FT = llvm::FunctionType::get(llvm::Type::getInt32Ty(gen.ctx), false);
//        llvm::Function * MainFunction = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "main", gen.module);
//
//        // block
//        llvm::BasicBlock * BB = llvm::BasicBlock::Create(gen.ctx, "entry", MainFunction);
//        gen.builder.SetInsertPoint(BB);

        m_AstTree->codegen(gen);

        // call writeln with value from lexel
//        gen.builder.CreateCall(gen.module.getFunction("writeln"), {
//                llvm::ConstantInt::get(gen.ctx, llvm::APInt(32, m_Lexer.numVal()))
//        });

        // return 0
//        gen.builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(gen.ctx), 0));
    }


    return this->gen.module;
}

