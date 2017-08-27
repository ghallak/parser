#include "grammar.h"

Grammar::FirstSet Grammar::first(Nonterminal nonterminal) const
{
    if (auto it = first_sets_.find(nonterminal);
        it != first_sets_.end())
    {
        return it->second;
    }

    auto init_size = first_sets_[nonterminal].size();
    do
    {
        init_size = first_sets_[nonterminal].size();

        for (const auto& production : productions_)
        {
            if (production->lhs() == nonterminal)
            {
                first_sets_[nonterminal].insert(first_from(production.get(), 0));
            }
        }
    } while (init_size != first_sets_[nonterminal].size());

    return first_sets_[nonterminal];
}

Grammar::FirstSet Grammar::first_from(const Production* production,
                                      std::size_t from) const
{
    auto symbols_count = production->rhs_length();

    FirstSet first_set;

    if (symbols_count == 0)
        first_set.insert(EmptyToken());

    for (std::size_t i = from; i < symbols_count; ++i)
    {
        auto fs = first(production, i);

        if (i + 1 < symbols_count)
            first_set.insert_without_empty_token(fs);
        else
            first_set.insert(fs);

        if (!fs.has_empty_token())
            break;
    }

    return first_set;
}

