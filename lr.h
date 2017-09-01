#pragma once

#include <algorithm>
#include <iostream>
#include <variant>
#include <vector>

#include "grammar.h"

enum class Token;
enum class Nonterminal;

std::ostream& operator<<(std::ostream& os, Token token);
std::ostream& operator<<(std::ostream& os, Nonterminal nonterminal);

class LR {
public:
    using Production = Grammar::Production;
    using Symbol     = Grammar::Symbol;
    using Terminal   = Grammar::Terminal;

    class Item {
    public:
        explicit Item(const Production* p)
            : production_(p), placeholder_(0), lookahead_(EofToken{})
        {
        }

        Item(const Production* p, std::size_t placeholder)
            : production_(p), placeholder_(placeholder), lookahead_(EofToken{})
        {
        }

        Item(const Production* p, Terminal lookahead)
            : production_(p), placeholder_(0), lookahead_(lookahead)
        {
        }

        Item(const Production* p, std::size_t placeholder, Terminal lookahead)
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

        Terminal lookahead() const noexcept
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

        Symbol next_symbol() const
        {
            return placeholder_ < production_->rhs_length()
                       ? production_->symbol_at(placeholder_)
                       : EofToken{};
        }

        Terminal next_terminal() const
        {
            return std::get<Terminal>(next_symbol());
        }

        Nonterminal next_nonterminal() const
        {
            return std::get<Nonterminal>(next_symbol());
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
        const Production* production_;
        std::size_t       placeholder_;
        Terminal          lookahead_;
    };

    LR(Grammar&& grammar);

    void print_items() const;

private:
    std::vector<Item> go_to(const std::vector<Item>& s, Symbol symbol) const;

    std::vector<Item> closure(std::vector<Item> s) const;

    std::vector<Terminal> first_after_next(const Item& item) const;

    // void create_tables(std::vector<std::vector<Item>> cc);

    const Grammar                  grammar_;
    std::vector<std::vector<Item>> cc_;
};
