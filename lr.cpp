#include "lr.h"

#include <algorithm>

LR::LR(Grammar&& grammar) : grammar_(std::move(grammar))
{
    std::vector<std::vector<Item>> marked;
    cc_.emplace_back(closure({Item{grammar_.main_production()}}));
    auto init_size = cc_.size();
    do
    {
        init_size = cc_.size();
        for (const auto& it : cc_)
        {
            if (std::find(marked.begin(), marked.end(), it) != marked.end())
                continue;

            marked.emplace_back(it);

            for (const auto& item : it)
            {
                if (!item.next_is_terminal())
                    continue;

                auto temp = go_to(it, item.next_terminal());

                if (std::find(cc_.begin(), cc_.end(), temp) == cc_.end())
                    cc_.emplace_back(temp);
            }
        }
    } while (init_size != cc_.size());
}

std::vector<LR::Item> LR::closure(std::vector<Item> s) const
{
    auto init_size = s.size();
    do
    {
        init_size = s.size();

        for (const auto& item : s)
        {
            if (item.next_is_terminal())
                continue;

            for (const auto& production : grammar_)
            {
                for (auto token : first_after_next(item))
                {
                    if (auto new_item = Item{production.get(), token};
                        std::find(s.begin(), s.end(), new_item) == s.end())
                    {
                        s.emplace_back(new_item);
                    }
                }
            }
        }
    } while (init_size != s.size());

    return s;
}

std::vector<LR::Item> LR::go_to(std::vector<Item> s, Token token) const
{
    std::vector<Item> moved;
    for (const auto& item : s)
    {
        if (!item.next_is_terminal())
            continue;

        if (auto next_token = item.next_terminal();
            next_token == token &&
            std::find(moved.begin(), moved.end(), item.advance_placeholder()) == moved.end())
        {
            moved.emplace_back(item.advance_placeholder());
        }
    }
    return moved;
}
