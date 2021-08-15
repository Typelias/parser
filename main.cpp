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

void printColor(const std::string &s)
{
    static int i = 0;
    std::cout << (i++ % 2 == 0 ? "\033[1;47;32m" : "\033[1;47;34m") << s << "\033[0m";
}

int main(int argc, char** argv)
{
    if(argc== 1) {
        std::cerr << "No arguments\n";
        return EXIT_FAILURE;
    }
    std::string input = argv[1];

    if(!std::getline(std::cin, text))
    {
        std::cerr << "No input file\n";
        return EXIT_FAILURE;
    }
    

    auto tokens = Tokenizer(input).getTokens();

    /* for (auto t : tokens)
    {
        std::cout << "[" << t << "]\t";
    }
    std::cout << "\n"; */

    auto p = Parser(tokens);

    auto root = p.parse();
    //text = "Waterloo I was defeated, you won the war Waterloo promise to love you for ever more Waterloo couldn't escape if I wanted to Waterloo knowing my fate is to be with you Waterloo finally facing my Waterloo";
    print(root.get());
    std::cout << "\n";

    if (!root)
    {
        std::cerr << "Could not parse tree\n";
        return EXIT_FAILURE;
    }

    auto success = root->evaluate();

    if (!success)
    {
        std::cerr << "No match\n";
        return EXIT_FAILURE;
    }

    int i = 0;

    while (success)
    {

        std::cout << std::string(text.begin() + i, text.begin() + startingChar);
        printColor(std::string(text.begin() + startingChar, text.begin() + currentChar));
        i = startingChar = currentChar;
        success = root->evaluate();
    }

    std::cout << std::string(text.begin() + i, text.end()) << "\n";

    return EXIT_SUCCESS;
}