#include "lr.h"

#include <iterator>

LR::LR(Grammar&& grammar) : grammar_(std::move(grammar))
{
    std::vector<std::vector<Item>> cc{
        closure({Item{grammar_.main_production()}})};
    for (std::size_t i = 0; i < cc.size(); ++i) {
        for (const auto& item : cc[i]) {
            auto temp = go_to(cc[i], item.next_symbol());

            if (!temp.empty() &&
                std::find(cc.begin(), cc.end(), temp) == cc.end())
                cc.emplace_back(temp);
        }
    }

    build_tables(std::move(cc));
}

std::vector<LR::Item> LR::go_to(const std::vector<Item>& s, Symbol symbol) const
{
    std::vector<Item> moved;
    for (const auto& item : s) {
        if (item.is_final() || item.next_symbol() != symbol)
            continue;

        auto next_item = item.advance_placeholder();
        if (std::find(moved.begin(), moved.end(), next_item) == moved.end())
            moved.emplace_back(next_item);
    }
    return closure(moved);
}

std::vector<LR::Item> LR::closure(std::vector<Item> s) const
{
    for (std::size_t i = 0; i < s.size(); ++i) {
        auto item = s[i];

        if (item.next_is_terminal())
            continue;

        for (const auto& production : grammar_) {
            if (production->lhs() != item.next_nonterminal())
                continue;
            for (auto token : first_after_next(item)) {
                if (auto new_item = Item{production.get(), token};
                    std::find(s.begin(), s.end(), new_item) == s.end()) {
                    s.emplace_back(new_item);
                }
            }
        }
    }
    return s;
}

std::vector<LR::Terminal> LR::first_after_next(const Item& item) const
{
    auto ss = grammar_.first_from(item.production(), item.placeholder() + 1);

    if (ss.find(EmptyToken{}) != ss.end()) {
        ss.erase(EmptyToken{});
        ss.insert(item.lookahead());
    }
    return std::vector<Terminal>(ss.begin(), ss.end());
}

void LR::build_tables(std::vector<std::vector<Item>>&& cc)
{
    auto nonterminals = grammar_.nonterminals();
    for (std::size_t i = 0; i < cc.size(); ++i) {
        const auto& subset = cc[i];
        for (const auto& item : subset) {
            if (item.is_final()) {
                if (is_ending_item(item))
                    actions_.emplace(std::pair{i, EofToken{}}, Accept{});
                else
                    actions_.emplace(std::pair{i, item.lookahead()},
                                     Reduce{item.production()});
            }
            else if (item.next_is_terminal()) {
                auto goto_set = go_to(subset, item.next_symbol());
                auto it       = std::find(cc.begin(), cc.end(), goto_set);

                if (it == cc.end())
                    continue;

                std::size_t j = std::distance(cc.begin(), it);
                actions_.emplace(std::pair{i, item.next_terminal()}, Shift{j});
            }
        }
        for (auto nonterminal : nonterminals) {
            auto goto_set = go_to(subset, nonterminal);
            auto it       = std::find(cc.begin(), cc.end(), goto_set);

            if (it == cc.end())
                continue;

            std::size_t j = std::distance(cc.begin(), it);

            goto_[std::pair{i, nonterminal}] = j;
        }
    }
}

void LR::print_tables() const
{
    for (const auto& it : actions_) {
        auto state    = it.first.first;
        auto terminal = it.first.second;
        auto action   = it.second;

        std::cout << state << '\t';

        if (std::holds_alternative<EofToken>(terminal)) {
            std::cout << "eof";
        }
        else if (std::holds_alternative<Token>(terminal)) {
            std::cout << std::get<Token>(terminal);
        }
        else {
            std::cout << "Empty Token";
        }

        std::cout << '\t';

        if (std::holds_alternative<Accept>(action)) {
            std::cout << "acc";
        }
        else if (std::holds_alternative<Shift>(action)) {
            std::cout << "shift " << std::get<Shift>(action).state;
        }
        else {
            std::cout << "reduce " << std::get<Reduce>(action).production->lhs()
                      << '('
                      << std::get<Reduce>(action).production->rhs_length()
                      << ')';
        }

        std::cout << '\n';
    }

    std::cout << "---------------\n";

    for (const auto& it : goto_) {
        auto state       = it.first.first;
        auto nonterminal = it.first.second;
        auto to_state    = it.second;

        std::cout << state << '\t' << nonterminal << '\t' << to_state << '\n';
    }
}

std::ostream& operator<<(std::ostream& os, const LR::Item& item)
{
    os << '[' << item.production_->lhs() << " ->";

    auto symbols_count = item.production_->rhs_length();
    for (std::size_t i = 0; i < symbols_count; ++i) {
        if (item.placeholder_ == i)
            os << " .";

        if (item.production_->is_terminal_at(i))
            os << ' ' << std::get<Token>(item.production_->terminal_at(i));
        else
            os << ' ' << item.production_->nonterminal_at(i);
    }

    if (item.placeholder_ == symbols_count)
        os << " . ";

    os << ", ";
    if (std::holds_alternative<Token>(item.lookahead_))
        os << std::get<Token>(item.lookahead_);
    else
        os << "EOF";
    os << ']';

    return os;
}
