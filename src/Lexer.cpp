#include<string>

#include "Lexer.hpp"
#include "Token.hpp"
/**
 * @brief Function to return the next token from standard input
 *
 * the variable 'm_IdentifierStr' is set there in case of an identifier,
 * the variable 'm_NumVal' is set there in case of a number.
 */

int Lexer::gettok()
{
    while(isspace(lastChar)) {
        lastChar = m_ifs->get();
    }
    //
    if (isalpha(lastChar ) ) { // identifier: [a-zA-Z][a-zA-Z0-9]*
        std::string identifierStr;
        identifierStr += lastChar;

        while ( isalnum(lastChar = m_ifs->get() ) )
            identifierStr += lastChar;

        // If identifier is a keyword
        if(m_keywords.find(identifierStr) != m_keywords.end()) {
            return m_keywords[identifierStr];
        }
        // Could be a 3 character operator
        if(m_3char_operators.find( identifierStr ) != m_3char_operators.end()) {
            return m_3char_operators[identifierStr];
        }

        m_IdentifierStr = identifierStr;
        return TokenType::tok_identifier;
    } 

    if (isdigit(lastChar)) {   // Number: [0-9.]+
        std::string numStr;
        do {
            numStr += lastChar;
            lastChar = m_ifs->get();
        } while (isdigit(lastChar));

        m_NumVal = strtod(numStr.c_str(), 0);
        return tok_number;
    }
    // Handle comments
    if (lastChar == '#') {
        // Comment until end of line.
        do
            lastChar = m_ifs->get();
        while (lastChar != EOF && lastChar != '\n' && lastChar != '\r');

        if (lastChar != EOF)
            return gettok();
    }

    // Handle strings
    if (lastChar == '\"') {
        std::string str;
        while ((lastChar = m_ifs->get()) != '\"') {
            str += lastChar;
        }
        m_IdentifierStr = str;
        // Must be ended in "
        lastChar = m_ifs->get();
        return TokenType::tok_identifier;
    }

    // all 2 char operators start with 1 char operator
    if(m_1char_operators.find( lastChar ) != m_1char_operators.end()) {
        // Need to look ahead for the next character
        std::string op;
        op += lastChar;
//        std::cout << std::endl <<"lastChar: >" << static_cast<char>(lastChar) << "<" << std::endl;
        lastChar = m_ifs->get();
//        std::cout << "nextChar: >" << static_cast<char>(lastChar) << "<" << std::endl;
        // if matches m_2char_operators
        if(m_2char_operators.find( op + static_cast<char>(lastChar) ) != m_2char_operators.end()) {
            std::string returnValue(op + static_cast<char>(lastChar));
            lastChar = m_ifs->get();
            return m_2char_operators[returnValue];
        }
        return m_1char_operators[op[0]];
    }
    

    // Check for end of file.  Don't eat the EOF.
    if (m_ifs->peek() == EOF)
        return tok_eof;

    // Otherwise, just return the character as its ascii value.
    return static_cast<int>(m_ifs->get());
}

void Lexer::initStream(std::istream& ifs)
{
    m_ifs = &ifs;
}

