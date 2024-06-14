
class Position {
    unsigned m_line;
    unsigned m_col;

public:
    // Position() = default;
    // Position(const Position& other) = default;

    friend std::ostream& operator<<(std::ostream& os, const Position& tk) noexcept;
    void advance();
    void line();
};