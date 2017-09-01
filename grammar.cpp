#include "grammar.h"

Grammar::FirstSet Grammar::first(Symbol symbol) const
{
    if (std::holds_alternative<Terminal>(symbol))
        return {std::get<Terminal>(symbol)};

    auto nonterminal = std::get<Nonterminal>(symbol);

    if (auto it = first_sets_.find(nonterminal); it != first_sets_.end())
        return it->second;

    auto& first_set = first_sets_[nonterminal];
    for (const auto& production : productions_)
        if (production->lhs() == nonterminal) {
            auto fs = first_from(production.get(), 0);
            first_set.insert(fs.begin(), fs.end());
        }

    return first_set;
}

Grammar::FirstSet Grammar::first_from(const Production* production,
                                      std::size_t       from) const
{
    auto symbols_count = production->rhs_length();

    FirstSet first_set;

    if (symbols_count == 0 || from >= symbols_count)
        first_set.insert(EmptyToken{});

    for (std::size_t i = from; i < symbols_count; ++i) {
        const auto& fs = first(production->symbol_at(i));

        if (i + 1 < symbols_count) {
            if (first_set.find(EmptyToken{}) == first_set.end()) {
                first_set.insert(fs.begin(), fs.end());
                first_set.erase(EmptyToken{});
            }
            else {
                first_set.insert(fs.begin(), fs.end());
            }
        }
        else {
            first_set.insert(fs.begin(), fs.end());
        }

        if (fs.find(EmptyToken{}) == fs.end())
            break;
    }

    return first_set;
}

std::vector<Nonterminal> Grammar::nonterminals() const
{
    std::unordered_set<Nonterminal> result;
    for (const auto& production : productions_) {
        if (main_production()->lhs() == production->lhs())
            continue;
        result.emplace(production->lhs());
    }
    return std::vector<Nonterminal>(result.begin(), result.end());
}
