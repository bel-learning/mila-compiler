#include "Parser.hpp"

Parser::Parser()
    : MilaContext()
    , MilaBuilder(MilaContext)
    , MilaModule("mila", MilaContext)
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
            {':', ":"}
    };

    if(tokenMap.find(CurTok) == tokenMap.end()) {
        std::cout << "Not known token: " << CurTok << std::endl;
        return;
    };
    if(CurTok == TokenType::tok_identifier) {
        std::cout << ">" << m_Lexer.identifierStr() << "<" << ", ";
    }
    else
        std::cout << ">" << tokenMap[CurTok]  << "<" << ", ";
}

bool Parser::Parse()
{

    while (CurTok != tok_eof) {
        printCurrentToken();
        getNextToken();
    }
    return true;
}

void Parser::initLexer(std::istream& ifs) {
    m_Lexer.initialize();
    m_Lexer.initStream(ifs);
};  // initialize lexer


const llvm::Module& Parser::Generate()
{

    // create writeln function
    {
      std::vector<llvm::Type*> Ints(1, llvm::Type::getInt32Ty(MilaContext));
      llvm::FunctionType * FT = llvm::FunctionType::get(llvm::Type::getInt32Ty(MilaContext), Ints, false);
      llvm::Function * F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "writeln", MilaModule);
      for (auto & Arg : F->args())
          Arg.setName("x");
    }

    // create main function
    {
        llvm::FunctionType * FT = llvm::FunctionType::get(llvm::Type::getInt32Ty(MilaContext), false);
        llvm::Function * MainFunction = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "main", MilaModule);

        // block
        llvm::BasicBlock * BB = llvm::BasicBlock::Create(MilaContext, "entry", MainFunction);
        MilaBuilder.SetInsertPoint(BB);

      // call writeln with value from lexel
      MilaBuilder.CreateCall(MilaModule.getFunction("writeln"), {
              llvm::ConstantInt::get(MilaContext, llvm::APInt(32, m_Lexer.numVal()))
      });

      // return 0
      MilaBuilder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(MilaContext), 0));
    }

    return this->MilaModule;
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


std::unique_ptr<ExprAST> Parser::Lmax() {
    auto result = L4();

    return result;
}
std::unique_ptr<ExprAST> Parser::L4() {

}
std::unique_ptr<ExprAST> Parser::L3() {

}
std::unique_ptr<ExprAST> Parser::L2() {

}
std::unique_ptr<ExprAST> Parser::L1() {

}
std::unique_ptr<ExprAST> Parser::L0() {

}

/// numberexpr ::= number
std::unique_ptr<ExprAST> Parser::ParseNumberExpr() {
    auto result = std::make_unique<NumberExprAST>(CurTok);
    getNextToken(); // consume the number
    return std::move(result);
}

std::unique_ptr<ExprAST> Parser::ParseParenExpr() {
    getNextToken(); // consume '('
    auto result = ParseExpression();
    if (!result)
        return nullptr;
    consume(TokenType::tok_rparen);

    return result;
}
/**
 * @brief Simple consumer.
 *
 * Checks if the token we want to consume is equal to CurTok and consumes it.
 */
bool Parser::consume(int token) {
    if(token != CurTok) {
        return false;
    }
    getNextToken();
    return true;
};