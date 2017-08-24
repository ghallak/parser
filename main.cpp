#include "grammar.h"

enum class Token
{ OPEN, CLOSE };

enum class Nonterminal
{ GOAL, LIST, PAIR };

int main()
{
    const auto grammar = []
    {
        Grammar<Token, Nonterminal> grammar;

        grammar.add_production(Nonterminal::GOAL,
                               Nonterminal::LIST);

        grammar.add_production(Nonterminal::LIST,
                               Nonterminal::LIST, Nonterminal::PAIR);
        grammar.add_production(Nonterminal::LIST,
                               Nonterminal::PAIR);

        grammar.add_production(Nonterminal::PAIR,
                               Token::OPEN, Nonterminal::PAIR, Token::CLOSE);
        grammar.add_production(Nonterminal::PAIR,
                               Token::OPEN, Token::CLOSE);

        return grammar;
    }();
}
