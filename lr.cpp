#include "lr.h"

#include <iterator>

LR::LR(Grammar&& grammar)
    : grammar_(std::move(grammar))
    , cc_({closure({Item{grammar_.main_production()}})})
{
    for (std::size_t i = 0; i < cc_.size(); ++i) {
        for (const auto& item : cc_[i]) {
            auto temp = item.next_is_terminal()
                            ? go_to(cc_[i], item.next_terminal())
                            : go_to(cc_[i], item.next_nonterminal());

            if (!temp.empty() &&
                std::find(cc_.begin(), cc_.end(), temp) == cc_.end())
                cc_.emplace_back(temp);
        }
    }

    // TODO: which one should I use
    // create_tables(cc_);
    // create_tables(std::move(cc_));
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

std::vector<std::optional<Token>> LR::first_after_next(const Item& item) const
{
    auto ss = grammar_.first_from(item.production(), item.placeholder() + 1);

    if (ss.has_empty_token())
        ss.insert(item.lookahead());
    return std::vector<std::optional<Token>>(ss.tokens().begin(),
                                             ss.tokens().end());
}

//void LR::create_tables(std::vector<std::vector<Item>> cc)
//{
    //    for (std::size_t i = 0; i < cc.size(); ++i) {
    //        const auto& subset = cc[i];
    //        for (const auto& item : subset) {
    //            if (item.is_final()) {
    //                if (item.is_ending_item()) {
    //                    // Action[i , eof ] ← ‘‘accept’’
    //                }
    //                else {
    //                    // Action[i ,a] ← ‘‘reduce A→β’’
    //                }
    //            }
    //            else {
    //                auto goto_set = item.next_is_terminal()
    //                    ? go_to(subset, item.next_terminal())
    //                    : go_to(subset, item.next_nonterminal());
    //                auto it = std::find(cc.begin(), cc.end(), goto_set);
    //
    //                // TODO: assert(it != cc.end());
    //
    //                auto j = std::distance(cc.begin(), it);
    //                // Action[i ,c] ← ‘‘shift j’’
    //            }
    //        }
    //        for (auto nonterminal : nonterminals_set) {
    //            auto goto_set = item.next_is_terminal()
    //                ? go_to(subset, item.next_terminal())
    //                : go_to(subset, item.next_nonterminal());
    //            auto it = std::find(cc.begin(), cc.end(), goto_set);
    //
    //            // TODO assert(it != cc.end());
    //
    //            auto j = std::distance(cc.begin(), it);
    //            // Goto[i ,nonterminal] ← j
    //    }
//}

void LR::print_items() const
{
    std::cout << "Size: " << cc_.size() << '\n'
              << "-----\n"
              << "Items:\n"
              << "------\n";
    for (const auto& subset : cc_) {
        for (const auto& item : subset) {
            std::cout << item << '\n';
        }
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
            os << ' ' << item.production_->token_at(i);
        else
            os << ' ' << item.production_->nonterminal_at(i);
    }

    if (item.placeholder_ == symbols_count)
        os << " . ";

    os << ", ";
    if (item.lookahead_.has_value())
        os << item.lookahead_.value();
    else
        os << "EOF";
    os << ']';

    return os;
}
