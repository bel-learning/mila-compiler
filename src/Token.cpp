//
// Created by bilguudeibaljinnyam on 6/13/24.
//
#include "Token.hpp"
#include "Lexer.hpp"

Token::Token(int type, const Position& position, const std::optional<int>& intValue ){
    m_tokenType = type;
    m_position = position;
    m_intValue = intValue;
}
