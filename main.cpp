#include <iostream>
#include "tokens.hpp"
#include "giggaTree.hpp"

void print(ASTNode *root)
{
    static int i = 0;
    root->print();
    i++;
    for (auto &n : root->children)
    {
        std::cout << "\n";
        for (int q = 0; q < i; q++)
        {
            std::cout << "\t";
        }
        print(n.get());
    }
    i--;
}

int main()
{
    // std::string input = "lo* could.{3} (hej + du) \\I \\O{2}";
    std::string input = "hej.*hej+tummel";

    auto tokens = Tokenizer(input).getTokens();

    auto p = Parser(tokens);

    auto root = p.parse();

    print(root.get());
    std::cout << "\n";
}