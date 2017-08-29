#include "grammar.h"
#include "lr.h"

enum class Token { OPEN, CLOSE };

enum class Nonterminal { GOAL, LIST, PAIR };

std::ostream& operator<<(std::ostream& os, Token token)
{
    switch (token) {
    case Token::OPEN:
        os << '(';
        break;
    case Token::CLOSE:
        os << ')';
        break;
    }
    return os;
}
std::ostream& operator<<(std::ostream& os, Nonterminal nonterminal)
{
    switch (nonterminal) {
    case Nonterminal::GOAL:
        os << "GOAL";
        break;
    case Nonterminal::LIST:
        os << "LIST";
        break;
    case Nonterminal::PAIR:
        os << "PAIR";
        break;
    }
    return os;
}

Grammar create_grammar()
{
    Grammar grammar(Nonterminal::GOAL, Nonterminal::LIST);

    grammar.add_production(Nonterminal::LIST, Nonterminal::LIST,
                           Nonterminal::PAIR);
    grammar.add_production(Nonterminal::LIST, Nonterminal::PAIR);

    grammar.add_production(Nonterminal::PAIR, Token::OPEN, Nonterminal::PAIR,
                           Token::CLOSE);
    grammar.add_production(Nonterminal::PAIR, Token::OPEN, Token::CLOSE);

    return grammar;
}

int main()
{
    LR lr{create_grammar()};

    lr.print_items();
}
