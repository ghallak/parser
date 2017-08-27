#pragma once

#include <vector>
#include <variant>
#include <memory>
#include <type_traits>
#include <unordered_set>
#include <unordered_map>

enum class Token;
enum class Nonterminal;

class Grammar
{
public:
    class Production;

private:
    struct EmptyToken {};

    using productions_vector = std::vector<std::unique_ptr<const Production>>;
    using const_iterator = productions_vector::const_iterator;

public:
    class Production
    {
    public:
        template<typename... Args>
        Production(Nonterminal lhs, Args&&... args)
            : lhs_(lhs)
        {
            (add_rhs(args), ...);
        }

        std::size_t rhs_length() const noexcept
        {
            return rhs_.size();
        }

        bool is_terminal_at(std::size_t id) const
        {
            return is_terminal_[id];
        }

        Token token_at(std::size_t id) const
        {
            return std::get<Token>(rhs_[id]);
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
        template<typename SymbolType>
        void add_rhs(SymbolType symbol)
        {
            rhs_.emplace_back(symbol);
            is_terminal_.emplace_back(std::is_same<SymbolType, Token>::value);
        }

        std::vector<std::variant<Token, Nonterminal>> rhs_;
        std::vector<bool> is_terminal_;
        Nonterminal lhs_;
    };

    class FirstSet
    {
    public:
        FirstSet() : tokens_(), empty_token_(false)
        {}

        explicit FirstSet(Token token)
            : tokens_({token})
            , empty_token_(false)
        {}

        void insert(EmptyToken)
        {
            empty_token_ = true;
        }

        void insert(Token token)
        {
            tokens_.insert(token);
        }

        void insert(const FirstSet& first_set)
        {
            tokens_.insert(first_set.tokens_.begin(), first_set.tokens_.end());
            empty_token_ |= first_set.empty_token_;
        }

        void insert_without_empty_token(const FirstSet& first_set)
        {
            tokens_.insert(first_set.tokens_.begin(), first_set.tokens_.end());
        }

        std::size_t size() const noexcept
        {
            return tokens_.size() + empty_token_;
        }

        bool has(Token token) const noexcept
        {
            return tokens_.find(token) != tokens_.end();
        }

        bool has_empty_token() const noexcept
        {
            return empty_token_;
        }

        // TODO this function should be removed later and first functions
        // that are used outside the grammar class should be public
        // and others should be private
        std::unordered_set<Token> tokens() const noexcept
        {
            return tokens_;
        }

    private:
        std::unordered_set<Token> tokens_;
        bool empty_token_;
    };

    Grammar(Nonterminal nt1, Nonterminal nt2)
    {
        productions_.emplace_back(std::make_unique<Production>(nt1, nt2));
    }

    const Production* main_production() const
    {
        return productions_.front().get();
    }

    template<typename... Args>
    void add_production(Args&&... args)
    {
        productions_.emplace_back(std::make_unique<Production>(args...));
    }

    FirstSet first(Nonterminal nonterminal) const;

    FirstSet first(Token token) const
    {
        return FirstSet(token);
    }

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
    FirstSet first(const Production* p, std::size_t id) const
    {
        return p->is_terminal_at(id) ? first(p->token_at(id))
                                    : first(p->nonterminal_at(id));
    }

    productions_vector productions_;
    mutable std::unordered_map<Nonterminal, FirstSet> first_sets_;
};
