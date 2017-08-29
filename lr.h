#pragma once

#include <algorithm>
#include <iostream>
#include <optional>
#include <type_traits>
#include <vector>

#include "grammar.h"

enum class Token;
enum class Nonterminal;

std::ostream& operator<<(std::ostream& os, Token token);
std::ostream& operator<<(std::ostream& os, Nonterminal nonterminal);

class LR {
public:
    using Production = Grammar::Production;

    class Item {
    public:
        explicit Item(const Production* p)
            : production_(p), placeholder_(0), lookahead_({})
        {
        }

        Item(const Production* p, std::size_t placeholder)
            : production_(p), placeholder_(placeholder), lookahead_({})
        {
        }

        Item(const Production* p, std::optional<Token> lookahead)
            : production_(p), placeholder_(0), lookahead_(lookahead)
        {
        }

        Item(const Production*    p,
             std::size_t          placeholder,
             std::optional<Token> lookahead)
            : production_(p), placeholder_(placeholder), lookahead_(lookahead)
        {
        }

        const Production* production() const noexcept
        {
            return production_;
        }

        std::size_t placeholder() const noexcept
        {
            return placeholder_;
        }

        std::optional<Token> lookahead() const noexcept
        {
            return lookahead_;
        }

        Item advance_placeholder() const noexcept
        {
            return Item{production_, placeholder_ + 1, lookahead_};
        }

        bool is_final() const
        {
            return placeholder_ == production_->rhs_length();
        }

        bool next_is_terminal() const
        {
            return placeholder_ < production_->rhs_length()
                       ? production_->is_terminal_at(placeholder_)
                       : true;
        }

        template<typename Symbol>
        bool next_symbol_equals(Symbol symbol) const
        {
            if constexpr (std::is_same<Symbol, std::optional<Token>>::value)
                return next_is_terminal() && next_terminal() == symbol;
            if constexpr (std::is_same<Symbol, Nonterminal>::value)
                return !next_is_terminal() && next_nonterminal() == symbol;
            return false;
        }

        std::optional<Token> next_terminal() const
        {
            return placeholder_ < production_->rhs_length()
                       ? production_->token_at(placeholder_)
                       : std::optional<Token>{};
        }

        Nonterminal next_nonterminal() const
        {
            return production_->nonterminal_at(placeholder_);
        }

        bool operator==(const Item& rhs) const
        {
            return production_ == rhs.production_ &&
                   placeholder_ == rhs.placeholder_ &&
                   lookahead_ == rhs.lookahead_;
        }

        bool operator!=(const Item& rhs) const
        {
            return !operator==(rhs);
        }

        friend std::ostream& operator<<(std::ostream& os, const Item& item);

    private:
        const Production*    production_;
        std::size_t          placeholder_;
        std::optional<Token> lookahead_;
    };

    LR(Grammar&& grammar);

    void print_items() const;

private:
    template<typename Symbol>
    std::vector<Item> go_to(const std::vector<Item>& s, Symbol symbol) const
    {
        std::vector<Item> moved;
        for (const auto& item : s) {
            if (item.is_final() || !item.next_symbol_equals(symbol))
                continue;

            auto next_item = item.advance_placeholder();
            if (std::find(moved.begin(), moved.end(), next_item) == moved.end())
                moved.emplace_back(next_item);
        }
        return closure(moved);
    }

    std::vector<Item> closure(std::vector<Item> s) const;

    std::vector<std::optional<Token>> first_after_next(const Item& item) const;

    //void create_tables(std::vector<std::vector<Item>> cc);

    const Grammar                  grammar_;
    std::vector<std::vector<Item>> cc_;
};
