//
// Created by bilguudeibaljinnyam on 6/13/24.
//

#ifndef PJPPROJECT_PARSER_HPP
#define PJPPROJECT_PARSER_HPP

#include<Lexer.hpp>
#include<Position.hpp>


class Token {
    int m_tokenType;
    std::optional<int> m_intValue;
    Position m_position;
public:
    Token(int type, const Position& position, const std::optional<int>& intValue = std::nullopt);
    int type() const;
    std::optional<int> value() const;
    const Position& position() const;
};

#endif // PJPPROJECT_PARSER_HPP
