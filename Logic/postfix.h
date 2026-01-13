#ifndef POSTFIX_H
#define POSTFIX_H

#include <string>
#include <memory>

using std::shared_ptr;

// Tree node for syntax tree
struct TreeNode {
    char value;
    std::shared_ptr<TreeNode> left;
    std::shared_ptr<TreeNode> right;
    float x = 0;   // for drawing
    float y = 0;   // for drawing

    TreeNode(char val) : value(val), left(nullptr), right(nullptr) {}
};

// Step 1 - Receive regex input
std::string receive_regex_input();

// Step 2 - Insert explicit concatenation operators
std::string insert_concatenation(const std::string& regex);

// Step 3 - Convert regex to postfix
std::string to_postfix(const std::string& regex);

// Step 4 - Build syntax tree from postfix with visualization
std::shared_ptr<TreeNode> build_syntax_tree(const std::string& postfix,
                                           const std::string& originalRegex,
                                           const std::string& regexWithConcat);

#endif