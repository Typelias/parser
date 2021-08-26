#pragma once

#include <vector>
#include <memory>
#include <iostream>
#include "tokens.hpp"
#include <string>
#include <algorithm>
#include <map>

int currentChar = 0;
int startingChar = 0;
bool parentIsIgnore = false;
bool visitedWhildcard = false;
std::string text;

struct GroupIndexe
{
    int start;
    int end;
};

std::map<int, GroupIndexe> indexes;

struct ASTNode
{
    virtual bool evaluate() = 0;
    std::vector<std::unique_ptr<ASTNode>> children;
    virtual void print() = 0;
};

struct OrNode : ASTNode
{
    void print() override
    {
        std::cout << "+";
    }
    bool evaluate() override
    {
        int checkpoint = currentChar;
        bool lhsSuccsess = children.front()->evaluate();
        int lhsEnd = currentChar;
        currentChar = checkpoint;
        bool rhsSuccess = children.back()->evaluate();
        int rhsEnd = currentChar;
        if (!lhsSuccsess && !rhsSuccess)
        {
            currentChar = lhsEnd > rhsEnd ? lhsEnd : rhsEnd;
            return false;
        }
        if (lhsSuccsess && rhsSuccess)
        {
            currentChar = lhsEnd > rhsEnd ? lhsEnd : rhsEnd;
            return true;
        }
        if (lhsSuccsess)
        {

            currentChar = lhsEnd;
            return true;
        }
        currentChar = rhsEnd;
        return true;
    }
};

struct ManyNode : ASTNode
{
    void print() override
    {
        std::cout << "*";
    }
    bool evaluate() override
    {
        for (auto &c : children)
        {
            if (!c->evaluate())
            {
                return false;
            }
        }
        if (visitedWhildcard)
        {
            currentChar = text.size();
            visitedWhildcard = false;
            return true;
        }
        char c = text[currentChar - 1];
        if (currentChar >= text.size() || text[currentChar] != c)
        {
            return false;
        }
        while (currentChar != text.size())
        {
            if (text[currentChar] != c)
            {
                break;
            }
            currentChar++;
        }

        // std::cout << "\"" << text[currentChar] << "\"\n";
        return true;
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
    bool evaluate() override
    {
        int start = currentChar;
        for (auto &c : children)
        {
            if (!c->evaluate())
            {
                return false;
            }
        }
        GroupIndexe i;
        i.start = start;
        i.end = currentChar;
        indexes[index] = i;

        return true;
    }
};

struct WildcardNode : ASTNode
{
    void print() override
    {
        std::cout << ".";
    }
    bool evaluate() override
    {
        if (currentChar >= text.size())
        {
            return false;
        }
        visitedWhildcard = true;
        currentChar++;
        return true;
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
    bool evaluate() override
    {
        if (!children.front()->evaluate())
        {
            return false;
        }
        if (visitedWhildcard)
        {
            if (currentChar + count - 1 >= text.size())
            {
                return false;
            }
            currentChar += count - 1;
            return true;
        }
        char c = text[currentChar - 1];
        for (int i = 0; i < count; i++)
        {
            if (currentChar >= text.size())
                return false;
            if (text[currentChar] != c)
            {
                return false;
            }
            currentChar++;
        }
        return true;
    }
};

struct IgnoreNode : ASTNode
{

    void print() override
    {
        std::cout << "\\I";
    }
    bool evaluate() override
    {
        //std::cout <<"test\n";
        parentIsIgnore = true;
        for (auto &c : children)
        {
            if (!c->evaluate())
            {
                parentIsIgnore = false;
                return false;
            }
        }
        parentIsIgnore = false;
        return true;
    }
};

struct GroupSelectorNode : ASTNode
{
    int selction;

    //GroupSelectorNode(int selction) : selction(selction) {}

    void print() override
    {
        std::cout << "\\O{" << selction << "}";
    }

    bool evaluate() override
    {
        for (auto &c : children)
        {
            if (!c->evaluate())
            {
                return false;
            }
        }
        if(selction == 0) {
            return true;
        }
        if(indexes.find(selction) == indexes.end()) {
            std::exit(EXIT_FAILURE);
        }
        startingChar = indexes[selction].start;
        currentChar = indexes[selction].end;
        return true;
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

    bool evaluate() override
    {
        if (currentChar >= text.size())
        {
            return false;
        }
        for (int i = 0; i < value.size(); i++)
        {
            if (value[i] == text[currentChar])
            {
                currentChar++;
            }
            else if (parentIsIgnore)
            {
                if (tolower(value[i]) == tolower(text[currentChar]))
                {
                    currentChar++;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return false;
            }
        }
        return true;
    }
};

struct RootNode : ASTNode
{
    void print() override
    {
        std::cout << "Root";
    }
    bool evaluate() override
    {
    RETRY:
        for (auto &c : children)
        {
            if (!c->evaluate())
            {
                currentChar++;
                startingChar = currentChar;
                if (startingChar >= text.size())
                {
                    return false;
                }
                goto RETRY;
            }
            // std::cout << "\"" << text[currentChar] << "\"\n";
        }
        return true;
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

    std::vector<std::unique_ptr<ASTNode>> tryBuildExpression()
    {
        std::vector<std::unique_ptr<ASTNode>> children;
        while (!isEnd())
        {
            if (tokens[currentToken].type == Token::Type::Ignore || tokens[currentToken].type == Token::Type::GroupSelector)
            {
                break;
            }
            auto p = tryBuildGroup();
            if (p != nullptr)
            {
                children.push_back(std::move(p));
            }
            else if ((p = tryBuildOperator()); p != nullptr)
            {
                children.push_back(std::move(p));
            }
            else if ((p = tryBuildOperand()); p != nullptr)
            {
                children.push_back(std::move(p));
            }
            else
            {
                std::exit(EXIT_FAILURE);
            }
        }

        return children;
    }

    std::unique_ptr<ASTNode> tryBuildGroupSelector()
    {
        int checkpoint = currentToken;
        auto isGroupeEnd = [](Token t)
        { return t.type == Token::Type::CloseParan; };

        auto stop = std::find_if(tokens.begin(), tokens.end(), isGroupeEnd);
        if (stop == tokens.end())
        {
            currentToken = checkpoint;
            return nullptr;
        }

        std::vector<std::unique_ptr<ASTNode>> c;
        auto selector = std::make_unique<GroupSelectorNode>();
        while (!isEnd())
        {
            if (tokens[currentToken].type == Token::Type::GroupSelector)
                break;
            auto p = tryBuildIgnore();
            if (p != nullptr)
            {
                selector->children.push_back(std::move(p));
            }
            else if ((c = tryBuildExpression()); !c.empty())
            {
                selector->children.insert(selector->children.end(),
                                          std::make_move_iterator(c.begin()),
                                          std::make_move_iterator(c.end()));
            }
        }
        auto t = getToken(Token::Type::GroupSelector);
        if (t == nullptr)
        {
            currentToken = checkpoint;
            return nullptr;
        }

        selector->selction = std::stoi(t->value);

        return selector;
    }

    std::unique_ptr<ASTNode> tryBuildIgnore()
    {
        int checkpoint = currentToken;
        auto c = tryBuildExpression();
        if (c.empty())
        {
            currentToken = checkpoint;
            return nullptr;
        }
        auto p = getToken(Token::Type::Ignore);

        if (p == nullptr)
        {
            currentToken = checkpoint;
            return nullptr;
        }
        auto ignore = std::make_unique<IgnoreNode>();
        ignore->children.insert(ignore->children.end(),
                                std::make_move_iterator(c.begin()),
                                std::make_move_iterator(c.end()));
        return ignore;
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
        if (isEnd()) {
            return nullptr;
        }
        std::unique_ptr<ASTNode> root = std::make_unique<RootNode>();

        while (!isEnd())
        {
            std::vector<std::unique_ptr<ASTNode>> c;
            auto p = tryBuildGroupSelector();
            if (p != nullptr)
            {
                root->children.push_back(std::move(p));
            }
            else if ((p = tryBuildIgnore()); p != nullptr)
            {
                root->children.push_back(std::move(p));
            }
            else if ((c = tryBuildExpression()); !c.empty())
            {
                root->children.insert(root->children.end(),
                                      std::make_move_iterator(c.begin()),
                                      std::make_move_iterator(c.end()));
            }
            else
            {
                return nullptr;
            }
        }

        return root;
    }
};