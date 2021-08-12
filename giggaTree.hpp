#pragma once

#include <vector>
#include <memory>
#include <iostream>
#include "tokens.hpp"
#include <string>

struct ASTNode
{
    // EvalResult virtual evaluate() = 0;
    std::vector<std::unique_ptr<ASTNode>> children;
    virtual void print() = 0;
};

struct OrNode : ASTNode
{
    void print() override
    {
        std::cout << "+";
    }
};

struct ManyNode : ASTNode
{
    void print() override
    {
        std::cout << "*";
    }
};

struct GroupNode : ASTNode
{
    int index;
    GroupNode(int index) : index(index) {}
    void print() override
    {
        std::cout << "()";
    }
};

struct WildcardNode : ASTNode
{
    void print() override
    {
        std::cout << ".";
    }
};

struct CounterNode : ASTNode
{
    int count;

    CounterNode(int count) : count(count) {}

    void print() override
    {
        std::cout << "{" << count << "}";
    }
};

struct IgnoreNode : ASTNode
{
    void print() override
    {
        std::cout << "\\I";
    }
};

struct GroupSelectorNode : ASTNode
{
    int selction;

    void print() override
    {
        std::cout << "\\O{" << selction << "}";
    }
};

struct StringNode : ASTNode
{
    std::string value;
    StringNode(std::string value) : value(value) {}
    void print() override
    {
        std::cout << "\"" << value << "\"";
    }
};

struct RootNode : ASTNode
{
    void print() override
    {
        std::cout << "Groot";
    }
};

class Parser
{
private:
    std::vector<Token> tokens;
    int currentToken = 0;

    std::unique_ptr<ASTNode> tryBuildString()
    {
        Token *t = getToken(Token::Type::String);

        if (t == nullptr)
        {
            return nullptr;
        }

        return std::make_unique<StringNode>(t->value);
    }

    std::unique_ptr<ASTNode> tryBuildWildcard()
    {
        Token *t = getToken(Token::Type::Wildcard);

        if (t == nullptr)
        {
            return nullptr;
        }

        return std::make_unique<WildcardNode>();
    }

    std::unique_ptr<ASTNode> tryBuildCounter()
    {
        int checkpoint = currentToken;

        auto operand = tryBuildOperand();

        if (operand == nullptr)
        {
            return nullptr;
        }

        Token *t = getToken(Token::Type::Counter);

        if (t == nullptr)
        {
            currentToken = checkpoint;
            return nullptr;
        }

        auto counter = std::make_unique<CounterNode>(std::stoi(t->value));
        counter->children.push_back(std::move(operand));
        return counter;
    }

    std::unique_ptr<ASTNode> tryBuildMany()
    {
        int checkpoint = currentToken;

        auto operand = tryBuildOperand();

        if (operand == nullptr)
        {
            return nullptr;
        }

        Token *t = getToken(Token::Type::Many);

        if (t == nullptr)
        {
            currentToken = checkpoint;
            return nullptr;
        }

        auto many = std::make_unique<ManyNode>();
        many->children.push_back(std::move(operand));

        return many;
    }

    std::unique_ptr<ASTNode> tryBuildOr()
    {
        int chekpoint = currentToken;

        auto lhs = tryBuildOperand();

        if (lhs == nullptr)
        {
            return nullptr;
        }

        Token *t = getToken(Token::Type::Or);

        if (t == nullptr)
        {
            currentToken = chekpoint;
            return nullptr;
        }

        auto rhs = tryBuildOperand();

        if (rhs == nullptr)
        {
            currentToken = chekpoint;
            return nullptr;
        }

        auto orNode = std::make_unique<OrNode>();

        orNode->children.push_back(std::move(lhs));
        orNode->children.push_back(std::move(rhs));
        return orNode;
    }

    std::unique_ptr<ASTNode> tryBuildOperand()
    {
        auto p = tryBuildString();
        if (p != nullptr)
        {
            return p;
        }
        else if ((p = tryBuildWildcard()); p != nullptr)
        {
            return p;
        }
        else
        {
            return nullptr;
        }
    }

    std::unique_ptr<ASTNode> tryBuildOperator()
    {
        auto p = tryBuildOr();
        if (p != nullptr)
        {
            return p;
        }
        else if ((p = tryBuildMany()); p != nullptr)
        {
            return p;
        }
        else if ((p = tryBuildCounter()); p != nullptr)
        {
            return p;
        }
        else
        {
            return nullptr;
        }
    }

    std::unique_ptr<ASTNode> tryBuildGroup()
    {
        static int groupIndex = 1;
        int checkpoint = currentToken;
        auto t = getToken(Token::Type::OpenParan);
        if (t == nullptr)
        {
            return nullptr;
        }
        auto group = std::make_unique<GroupNode>(groupIndex);
        while (tokens[currentToken].type != Token::Type::CloseParan)
        {
            std::cout << tokens[currentToken] << "\n";
            if (isEnd())
            {
                std::exit(EXIT_FAILURE);
            }
            auto p = tryBuildOperator();
            if (p != nullptr)
            {
                group->children.push_back(std::move(p));
            }
            else if ((p = tryBuildOperand()); p != nullptr)
            {
                group->children.push_back(std::move(p));
            }
            else
            {
                std::exit(EXIT_FAILURE);
            }
        }
        currentToken++;
        groupIndex++;
        return group;
    }

    bool isEnd() const
    {
        return currentToken >= tokens.size();
    }

    Token *getToken(Token::Type type)
    {
        if (isEnd())
        {
            return nullptr;
        }
        if (tokens[currentToken].type == type)
        {
            return &tokens[currentToken++];
        }
        return nullptr;
    }

public:
    Parser(std::vector<Token> tokens) : tokens(tokens)
    {
    }

    std::unique_ptr<ASTNode> parse()
    {
        std::unique_ptr<ASTNode> root = std::make_unique<RootNode>();
        while (!isEnd())
        {
            auto p = tryBuildGroup();
            if (p != nullptr)
            {
                root->children.push_back(std::move(p));
            }
            else if ((p = tryBuildOperator()); p != nullptr)
            {
                root->children.push_back(std::move(p));
            }
            else if ((p = tryBuildOperand()); p != nullptr)
            {
                root->children.push_back(std::move(p));
            }
            else
            {
                return nullptr;
            }
        }

        return root;
    }
};