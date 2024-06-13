//
// Created by bilguudeibaljinnyam on 6/13/24.
//

#ifndef PJPPROJECT_PARSER_HPP
#define PJPPROJECT_PARSER_HPP

#include<Lexer.hpp>

class Position {
    unsigned m_line;
    unsigned m_col;

public:
    Position();
    Position(const Position& other);

    friend std::ostream& operator<<(std::ostream& os, const Position& tk) noexcept;
    void advance();
    void line();
};


class Token {
    TokenType m_tokenType;
    std::optional<int> m_intValue;
    Position m_position;
public:
    Token(TokenType type, const Position& position, const std::optional<int>& intValue);
    TokenType type() const;
    std::optional<int> value() const;
    const Position& position() const;
};

#endif // PJPPROJECT_PARSER_HPP
