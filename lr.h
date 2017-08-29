#pragma once

#include <iostream>
#include <optional>
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

        Item(const Production* p, Token lookahead)
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
            return Item{production_, placeholder_ + 1, lookahead_.value()};
        }

        bool next_is_terminal() const
        {
            return production_->is_terminal_at(placeholder_);
        }

        Token next_terminal() const
        {
            return production_->token_at(placeholder_);
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
    std::vector<Item> closure(std::vector<Item> s) const;

    std::vector<Item> go_to(std::vector<Item> s, Token token) const;

    std::vector<Token> first_after_next(const Item& item) const
    {
        auto ss =
            grammar_.first_from(item.production(), item.placeholder() + 1);

        // TODO This is not adding EOF token, should I add it?
        if (ss.has_empty_token() && item.lookahead().has_value())
            ss.insert(item.lookahead().value());
        return std::vector<Token>(ss.tokens().begin(), ss.tokens().end());
    }

    std::vector<std::vector<Item>> cc_;
    const Grammar                  grammar_;
};
