#ifndef PJPPROJECT_LEXER_HPP
#define PJPPROJECT_LEXER_HPP

#include <iostream>
#include <istream>
#include <vector>
#include <optional>
#include <map>

#include "Token.hpp"


enum TokenType {
    tok_eof =           -1,

    // numbers and identifiers
    tok_identifier =    -2,
    tok_number =        -3,

    // keywords
    tok_begin =         -4,
    tok_end =           -5,
    tok_const =         -6,
    tok_procedure =     -7,
    tok_forward =       -8,
    tok_function =      -9,
    tok_if =            -10,
    tok_then =          -11,
    tok_else =          -12,
    tok_program =       -13,
    tok_while =         -14,
    tok_exit =          -15,
    tok_var =           -16,
    tok_integer =       -17,
    tok_for =           -18,
    tok_do =            -19,

    // 2-character operators
    tok_notequal =      -20,
    tok_lessequal =     -21,
    tok_greaterequal =  -22,
    tok_assign =        -23,
    tok_or =            -24,
    tok_range =         -33,

    // 3-character operators (keywords)
    tok_mod =           -25,
    tok_div =           -26,
    tok_not =           -27,
    tok_and =           -28,
    tok_xor =           -29,

    // keywords in for loop
    tok_to =            -30,
    tok_downto =        -31,

    // keywords for array
    tok_array =         -32,


    // 1-character operators
    tok_plus =          '+',
    tok_minus =         '-',
    tok_star =          '*',
    tok_slash =         '/',
    tok_lparen =        '(',
    tok_rparen =        ')',
    tok_lbrace =        '{',
    tok_rbrace =        '}',
    tok_lbracket =      '[',
    tok_rbracket =      ']',
    tok_comma =         ',',
    tok_semicolon =     ';',
    tok_greater =       '>',
    tok_less =          '<',
    tok_equal =         '=',
    tok_exclamation =   '!',
    tok_bar =           '|',
    tok_ampersand =     '&',
    tok_caret =         '^',
    tok_colon =         ':',
    tok_dquote =        '"',
    tok_squote =        '\'',
    tok_dot =           '.',
};



class Lexer {
public:
    Lexer() {
        lastChar = ' ';
        initialize_keywords();
        initialize_2char_operators();
        initialize_3char_operators();
        initialize_1char_operators();
    };
    ~Lexer() = default;
    
    void tokenize(); // tokenize
    int gettok();
    const std::string& identifierStr() const { return this->m_IdentifierStr; }

    int numVal() { return this->m_NumVal; }

private:
    int lastChar;
    std::string m_IdentifierStr;
    int m_NumVal;

    // Position m_currentPos;
    std::vector<TokenType> m_Tokens;
    
    std::map<std::string, TokenType> m_keywords;

    void initialize_keywords() {
        m_keywords["begin"] = tok_begin;
        m_keywords["end"] = tok_end;
        m_keywords["const"] = tok_const;
        m_keywords["procedure"] = tok_procedure;
        m_keywords["forward"] = tok_forward;
        m_keywords["function"] = tok_function;
        m_keywords["if"] = tok_if;
        m_keywords["then"] = tok_then;
        m_keywords["else"] = tok_else;
        m_keywords["program"] = tok_program;
        m_keywords["while"] = tok_while;
        m_keywords["exit"] = tok_exit;
        m_keywords["var"] = tok_var;
        m_keywords["integer"] = tok_integer;
        m_keywords["for"] = tok_for;
        m_keywords["to"] = tok_to;
        m_keywords["downto"] = tok_downto;
        m_keywords["do"] = tok_do;

    }

    std::map<std::string, TokenType> m_2char_operators;

    void initialize_2char_operators() {
        m_2char_operators["!="] = tok_notequal;
        m_2char_operators["<="] = tok_lessequal;
        m_2char_operators[">="] = tok_greaterequal;
        m_2char_operators[":="] = tok_assign;
        m_2char_operators["||"] = tok_or;
        m_2char_operators[".."] = tok_range;
    }

    std::map<std::string, TokenType> m_3char_operators;

    void initialize_3char_operators() {
        m_3char_operators["mod"] = tok_mod;
        m_3char_operators["div"] = tok_div;
        m_3char_operators["not"] = tok_not;
        m_3char_operators["and"] = tok_and;
        m_3char_operators["xor"] = tok_xor;
    }

    std::map<char, TokenType> m_1char_operators;

    void initialize_1char_operators() {
        m_1char_operators['+'] = tok_plus;
        m_1char_operators['-'] = tok_minus;
        m_1char_operators['*'] = tok_star;
        m_1char_operators['/'] = tok_slash;
        m_1char_operators['('] = tok_lparen;
        m_1char_operators[')'] = tok_rparen;
        m_1char_operators['{'] = tok_lbrace;
        m_1char_operators['}'] = tok_rbrace;
        m_1char_operators['['] = tok_lbracket;
        m_1char_operators[']'] = tok_rbracket;
        m_1char_operators[','] = tok_comma;
        m_1char_operators[';'] = tok_semicolon;
        m_1char_operators['>'] = tok_greater;
        m_1char_operators['<'] = tok_less;
        m_1char_operators['='] = tok_equal;
        m_1char_operators['!'] = tok_exclamation;
        m_1char_operators['|'] = tok_bar;
        m_1char_operators['&'] = tok_ampersand;
        m_1char_operators['^'] = tok_caret;
        m_1char_operators[':'] = tok_colon;
        m_1char_operators['"'] = tok_dquote;
        m_1char_operators['\''] = tok_squote;
        m_1char_operators['.'] = tok_dot;

    }

    
};


/*
 * Lexer returns tokens [0-255] if it is an unknown character, otherwise one of these for known things.
 * Here are all valid tokens:
 */






#endif //PJPPROJECT_LEXER_HPP
