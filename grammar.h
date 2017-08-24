#pragma once

#include <vector>
#include <variant>
#include <type_traits>
#include <unordered_set>
#include <unordered_map>

template<typename TokenType, typename NonterminalType>
class Grammar
{
private:
    struct EmptyToken {};

public:
    class Production
    {
    public:
        template<typename... Args>
        Production(NonterminalType lhs, Args&&... args)
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
        TokenType token_at(std::size_t id) const
        {
            return std::get<TokenType>(rhs_[id]);
        }
        NonterminalType nonterminal_at(std::size_t id) const
        {
            return std::get<NonterminalType>(rhs_[id]);
        }
        NonterminalType lhs() const noexcept
        {
            return lhs_;
        }

    private:
        template<typename SymbolType>
        void add_rhs(SymbolType symbol)
        {
            rhs_.emplace_back(symbol);
            is_terminal_.emplace_back(std::is_same<SymbolType, TokenType>::value);
        }

        std::vector<std::variant<TokenType, NonterminalType>> rhs_;
        std::vector<bool> is_terminal_;
        NonterminalType lhs_;
    };

    class FirstSet
    {
    public:
        FirstSet() : tokens_(), empty_token_(false)
        {}
        explicit FirstSet(TokenType token)
            : tokens_({token})
            , empty_token_(false)
        {}

        void insert(EmptyToken)
        {
            empty_token_ = true;
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
        bool has(TokenType token) const noexcept
        {
            return tokens_.find(token) != tokens_.end();
        }
        bool has_empty_token() const noexcept
        {
            return empty_token_;
        }

    private:
        std::unordered_set<TokenType> tokens_;
        bool empty_token_;
    };

    template<typename... Args>
    void add_production(Args&&... args)
    {
        productions_.emplace_back(args...);
    }

    FirstSet first(NonterminalType nonterminal) const;
    FirstSet first(TokenType token) const
    {
        return FirstSet(token);
    }

    using iterator = typename std::vector<Production>::iterator;
    using const_iterator = typename std::vector<Production>::const_iterator;

    iterator begin() noexcept
    {
        return productions_.begin();
    }
    iterator end() noexcept
    {
        return productions_.end();
    }
    const_iterator begin() const noexcept
    {
        return productions_.cbegin();
    }
    const_iterator end() const noexcept
    {
        return productions_.cend();
    }

private:
    FirstSet first(const Production& p, std::size_t id) const
    {
        return p.is_terminal_at(id) ? first(p.token_at(id))
                                    : first(p.nonterminal_at(id));
    }

    std::vector<Production> productions_;
    mutable std::unordered_map<NonterminalType, FirstSet> first_sets_;
};

template<typename TokenType, typename NonterminalType>
typename Grammar<TokenType, NonterminalType>::FirstSet
Grammar<TokenType, NonterminalType>::first(NonterminalType nonterminal) const
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
            if (production.lhs() == nonterminal)
            {
                auto symbols_count = production.rhs_length();

                FirstSet rhs;

                if (symbols_count == 0)
                    rhs.insert(EmptyToken());

                for (std::size_t i = 0; i < symbols_count; ++i)
                {
                    auto fs = first(production, i);

                    if (i + 1 < symbols_count)
                        rhs.insert_without_empty_token(fs);
                    else
                        rhs.insert(fs);

                    if (!fs.has_empty_token())
                        break;
                }

                first_sets_[nonterminal].insert(rhs);
            }
        }
    } while (init_size != first_sets_[nonterminal].size());

    return first_sets_[nonterminal];
}
