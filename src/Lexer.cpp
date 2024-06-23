#include<string>

#include "Lexer.hpp"
#include "Token.hpp"
#include <iostream>
/**
 * @brief Function to return the next token from standard input
 *
 * the variable 'm_IdentifierStr' is set there in case of an identifier,
 * the variable 'm_NumVal' is set there in case of a number.
 */

int Lexer::gettok()
{
    while(isspace(lastChar)) {
        lastChar = std::cin.get();
    }

    //
    if (isalpha(lastChar ) || lastChar == '_' ) { // identifier: [a-zA-Z_-][a-zA-Z0-9_-]*
        std::string identifierStr;
        identifierStr += lastChar;

        while ( isalnum(lastChar = std::cin.get() ) || lastChar == '_' )
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

    if (isdigit(lastChar) || lastChar == '$' || lastChar =='&') {   // Number: [0-9.]+
        std::string numStr;
        char numberBase = ' ';
        if(lastChar == '$' || lastChar =='&') {
            numberBase = lastChar;
            lastChar = std::cin.get();
        }
        do {
            numStr += lastChar;
            lastChar = std::cin.get();
        } while (isdigit(lastChar));
        if(numberBase == ' ') {
            m_NumVal = strtod(numStr.c_str(), 0);
        }
        else if (numberBase == '$') {
            // Convert hexadecimal to base 10
            m_NumVal = std::stol(numStr, nullptr, 16);
        } else if (numberBase == '&') {
            // Convert octal to base 10
            m_NumVal = std::stol(numStr, nullptr, 8);
        }

        return tok_number;
    }
    // Handle comments
    if (lastChar == '#') {
        // Comment until end of line.
        do
            lastChar = std::cin.get();
        while (lastChar != EOF && lastChar != '\n' && lastChar != '\r');

        if (lastChar != EOF)
            return gettok();
    }

    // Handle strings
    if (lastChar == '\"') {
        std::string str;
        while ((lastChar = std::cin.get()) != '\"') {
            str += lastChar;
        }
        m_IdentifierStr = str;
        // Must be ended in "
        lastChar = std::cin.get();
        return TokenType::tok_identifier;
    }

    // all 2 char operators start with 1 char operator
    if(m_1char_operators.find( lastChar ) != m_1char_operators.end()) {
        // Need to look ahead for the next character
        std::string op;
        op += lastChar;
//        std::clog << std::endl <<"lastChar: >" << static_cast<char>(lastChar) << "<" << std::endl;
        lastChar = std::cin.get();
//        std::clog << "nextChar: >" << static_cast<char>(lastChar) << "<" << std::endl;
        // if matches m_2char_operators
        if(m_2char_operators.find( op + static_cast<char>(lastChar) ) != m_2char_operators.end()) {
            std::string returnValue(op + static_cast<char>(lastChar));
            lastChar = std::cin.get();
            return m_2char_operators[returnValue];
        }
        return m_1char_operators[op[0]];
    }
    

    // Check for end of file.  Don't eat the EOF.
    if (std::cin.peek() == EOF)
        return tok_eof;

    // Otherwise, just return the character as its ascii value.
    return static_cast<int>(std::cin.get());
}



