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

        bool next_is_terminal() const
        {
            return std::holds_alternative<Terminal>(next_symbol());
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

    bool valid_parse(std::vector<Token> words) const;

    void print_tables() const;

private:
    std::vector<Item> go_to(const std::vector<Item>& s, Symbol symbol) const;

    std::vector<Item> closure(std::vector<Item> s) const;

    std::vector<Terminal> first_after_next(const Item& item) const;

    void build_tables(std::vector<std::vector<Item>>&& cc);

    bool is_ending_item(const Item& item) const
    {
        return item.is_final() &&
               item.production() == grammar_.main_production();
    }

    struct PairHash {
        template<typename T, typename U>
        std::size_t operator()(const std::pair<T, U>& p) const
        {
            auto h1 = std::hash<T>{}(p.first);
            auto h2 = std::hash<U>{}(p.second);

            return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
        }
    };

    struct Shift {
        Shift(std::size_t s) : state(s) {}
        std::size_t state;
    };

    struct Reduce {
        Reduce(const Production* p) : production(p) {}
        const Production* production;
    };

    struct Accept {
    };

    using Action            = std::variant<Shift, Reduce, Accept>;
    using index_nonterminal = std::pair<std::size_t, Nonterminal>;
    using index_terminal    = std::pair<std::size_t, Terminal>;

    const Grammar                                                grammar_;
    std::unordered_map<index_terminal, Action, PairHash>         actions_;
    std::unordered_map<index_nonterminal, std::size_t, PairHash> goto_;
};
