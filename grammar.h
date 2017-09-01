#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

enum class Token;
enum class Nonterminal;

struct EmptyToken {
    constexpr bool operator==(const EmptyToken&) const
    {
        return true;
    }
    constexpr bool operator!=(const EmptyToken&) const
    {
        return false;
    }
};

struct EofToken {
    constexpr bool operator==(const EofToken&) const
    {
        return true;
    }
    constexpr bool operator!=(const EofToken&) const
    {
        return false;
    }
};

namespace std {

template<>
struct hash<EmptyToken> {
    constexpr std::size_t operator()(const EmptyToken&) const noexcept
    {
        return 0u;
    }
};

template<>
struct hash<EofToken> {
    constexpr std::size_t operator()(const EofToken&) const noexcept
    {
        return 1u;
    }
};

}  // namespace std

class Grammar {
public:
    class Production;

private:
    using productions_vector = std::vector<std::unique_ptr<const Production>>;
    using const_iterator     = productions_vector::const_iterator;

public:
    using Terminal = std::variant<Token, EmptyToken, EofToken>;
    using Symbol   = std::variant<Nonterminal, Terminal>;
    using FirstSet = std::unordered_set<Terminal>;

    class Production {
    public:
        template<typename... Args>
        Production(Nonterminal lhs, Args&&... args) : lhs_(lhs)
        {
            (add_rhs(args), ...);
        }

        std::size_t rhs_length() const noexcept
        {
            return rhs_.size();
        }

        bool is_terminal_at(std::size_t id) const
        {
            return std::holds_alternative<Terminal>(rhs_[id]);
        }

        Symbol symbol_at(std::size_t id) const
        {
            return rhs_[id];
        }

        Terminal terminal_at(std::size_t id) const
        {
            return std::get<Terminal>(rhs_[id]);
        }

        Nonterminal nonterminal_at(std::size_t id) const
        {
            return std::get<Nonterminal>(rhs_[id]);
        }

        Nonterminal lhs() const noexcept
        {
            return lhs_;
        }

    private:
        void add_rhs(Symbol symbol)
        {
            rhs_.emplace_back(symbol);
        }

        std::vector<Symbol> rhs_;
        Nonterminal         lhs_;
    };

    Grammar(Nonterminal nt1, Nonterminal nt2)
    {
        productions_.emplace_back(std::make_unique<Production>(nt1, nt2));
    }

    template<typename... Args>
    void add_production(Args&&... args)
    {
        productions_.emplace_back(std::make_unique<Production>(args...));
    }

    const Production* main_production() const
    {
        return productions_.front().get();
    }

    FirstSet first(Symbol symbol) const;

    FirstSet first_from(const Production* p, std::size_t from) const;

    const_iterator begin() const noexcept
    {
        return productions_.cbegin();
    }

    const_iterator end() const noexcept
    {
        return productions_.cend();
    }

private:
    productions_vector                                productions_;
    mutable std::unordered_map<Nonterminal, FirstSet> first_sets_;
};
