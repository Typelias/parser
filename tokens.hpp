#pragma once

#include <string>
#include <iostream>
#include <vector>

struct Token
{
    enum struct Type
    {
        Or,
        Many,
        OpenParan,
        CloseParan,
        Wildcard,
        Counter,
        Ignore,
        GroupSelector,
        String
    };

    std::string value;
    Type type;
};

std::ostream &operator<<(std::ostream &os, Token token)
{
    switch (token.type)
    {
    case Token::Type::Or:
        os << "+";
        break;
    case Token::Type::Many:
        os << "*";
        break;
    case Token::Type::OpenParan:
        os << "(";
        break;
    case Token::Type::CloseParan:
        os << ")";
        break;
    case Token::Type::Wildcard:
        os << ".";
        break;
    case Token::Type::Counter:
        os << "{" << token.value << "}";
        break;
    case Token::Type::Ignore:
        os << "\\I";
        break;
    case Token::Type::GroupSelector:
        os << "\\O{" << token.value << "}";
        break;
    case Token::Type::String:
        os << "\"" << token.value << "\"";
        break;
    default:
        std::exit(EXIT_FAILURE);
        break;
    }
    return os;
}

class Tokenizer
{
private:
    std::vector<Token> _token;
    std::string createBasicToken(std::vector<Token> &vec, std::string t, Token::Type type)
    {
        if (t != "")
            vec.push_back({.value = t, .type = Token::Type::String});
        vec.push_back({.value = "", .type = type});
        return "";
    }

    std::vector<Token> tokenize(std::string input)
    {
        std::vector<Token> tokens;
        std::string t = "";
        for (int i = 0; i < input.length(); i++)
        {
            switch (input[i])
            {
            case '*':
                t = createBasicToken(tokens, t, Token::Type::Many);
                break;
            case '.':
                t = createBasicToken(tokens, t, Token::Type::Wildcard);
                break;
            case '+':
                t = createBasicToken(tokens, t, Token::Type::Or);
                break;
            case '(':
                t = createBasicToken(tokens, t, Token::Type::OpenParan);
                break;
            case ')':
                t = createBasicToken(tokens, t, Token::Type::CloseParan);
                break;
            case '{':
                if (t != "")
                    tokens.push_back({.value = t, .type = Token::Type::String});
                t = "";
                i++;
                while (input[i] != '}')
                {
                    if (i == 1 - input.length())
                    {
                        std::exit(EXIT_FAILURE);
                    }
                    t += input[i];
                    i++;
                }
                tokens.push_back({.value = t, .type = Token::Type::Counter});
                t = "";
                break;

            case '\\':
                if (t != "")
                    tokens.push_back({.value = t, .type = Token::Type::String});
                t = "";

                i++;
                if (input[i] == 'O')
                {
                    i++;
                    if (input[i] != '{')
                        std::exit(EXIT_FAILURE);
                    i++;
                    while (input[i] != '}')
                    {
                        if (i == 1 - input.length())
                        {
                            std::exit(EXIT_FAILURE);
                        }
                        t += input[i];
                        i++;
                    }
                    tokens.push_back({.value = t, .type = Token::Type::GroupSelector});
                    t = "";
                }
                else if (input[i] == 'I')
                {
                    tokens.push_back({.value = t, .type = Token::Type::Ignore});
                    t = "";
                }
                else
                {
                    std::exit(EXIT_FAILURE);
                }

                break;

            default:
                t += input[i];
                break;
            }
        }

        if (t != "")
            tokens.push_back({.value = t, .type = Token::Type::String});

        return tokens;
    }

public:
    Tokenizer(std::string input)
    {
        _token = tokenize(input);
    }

    std::vector<Token> getTokens()
    {
        return _token;
    }
};
