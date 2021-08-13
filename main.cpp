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
    //std::string input = "hej.*hej+tummel (greger+banan)\\O{1}satan\\I.{3}";
    //std::string input = "(hej+tummel{3})";
    std::string input = "promise to (Love+Hate)\\I you\\O{1}";

    auto tokens = Tokenizer(input).getTokens();

    for (auto t: tokens)
    {
        std::cout << "[" << t << "]\t";
    }
    std::cout << "\n";
    

    auto p = Parser(tokens);

    auto root = p.parse();

    print(root.get());
    std::cout << "\n";

    return EXIT_SUCCESS;
}