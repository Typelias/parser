#include <iostream>
#include "tokens.hpp"

int main()
{
    std::string input = "lo* could.{3} (hej + du) \\I \\O{2}";

    auto tokens = Tokenizer(input).getTokens();

    for (auto c : tokens)
    {
        std::cout << c << "\n";
    }

}