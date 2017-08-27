#include "grammar.h"
#include "lr.h"

enum class Token
{ OPEN, CLOSE };

enum class Nonterminal
{ GOAL, LIST, PAIR };

Grammar create_grammar()
{
    Grammar grammar(Nonterminal::GOAL, Nonterminal::LIST);

    grammar.add_production(Nonterminal::LIST,
                            Nonterminal::LIST, Nonterminal::PAIR);
    grammar.add_production(Nonterminal::LIST,
                            Nonterminal::PAIR);

    grammar.add_production(Nonterminal::PAIR,
                            Token::OPEN, Nonterminal::PAIR, Token::CLOSE);
    grammar.add_production(Nonterminal::PAIR,
                            Token::OPEN, Token::CLOSE);

    return grammar;
}

int main()
{
    LR lr{create_grammar()};
}
