#include "postfix.h"

#include <iostream>
#include <string>
#include <stack>
#include <memory>
#include <vector>
#include <map>
using namespace std;

// structs and function declarations are in the header (postfix.h)
// function implementations are in this .cpp file
// This file does:
// 1. receive regex as input
// 2. add explicit concatenation operators .
// 3. convert the regex into postfix using the shunting-yard algorithm
// 4. build syntax tree from postfix using a stack

// Next steps are in simulator.cpp and nfa_visualizer.cpp
// 5. construct NFA from syntax tree using Thompson's construction
//  - recursively traverse the syntax tree of the regex and build an NFA for each node
// 6. simulate NFA on input string


/*
Step 1 - Receive Regex as Input
*/
string receive_regex_input() {
    string regex;
    cout << "Enter a regular expression: ";
    getline(cin, regex);
    return regex;
}


/*
Step 2 - Preprocess
Insert explicit concatenation operator .
e.g. "a(b|c)*d" -> "a·(b|c)*·d"
*/
string insert_concatenation(const string& regex) {
    string result;
    for (size_t i = 0; i < regex.length(); ++i) {
        char curr = regex[i];
        result += curr;

        if (i + 1 < regex.length()) {
            char next = regex[i + 1];

            // Check conditions for inserting concatenation operator
            // a b     → a.b
            // a (     → a.(
            // ) a     → ).a
            // * a     → *.a
            if ((isalnum(curr) || curr == '*' || curr == ')') &&
                (isalnum(next) || next == '(')) {
                result += '.';
            }
        }
    }
    return result;
}


/*
Step 3 - Shunting-Yard Algorithm
Operator precedence is defined as: * > . > |
*/
int get_precedence(char op) {
    switch (op) {
        case '.': return 2;
        case '|': return 1; // lowest precedence
        default: return 0;
    }
}

// Helper function to check if character is an operator
bool is_operator(char c) {
    return c == '*' || c == '.' || c == '|';
}

// Convert infix regex to postfix using the shunting-yard algorithm
string to_postfix(const string& regex) {
    string output;
    string operators;

    for (char token : regex) {
        if (isalnum(token)) {
            output += token;
        } else if (token == '(') { // push '(' to operator stack
            operators += token;
        } else if (token == ')') { // pop operators to output until '(' is popped
            while (!operators.empty() && operators.back() != '(') {
                output += operators.back();
                operators.pop_back();
            }
            operators.pop_back(); // pop the '('
        } else if (token == '*') {  // Kleene star is postfix, add '*' directly to output
            output += token;
        } else if (is_operator(token)) {
            /* for token . or |
            While:
            - stack is not empty, and
            - top of stack is an operator, and
            - precedence(top) ≥ precedence(t)
            pop from stack to output
            Then: push t onto stack
            */
            while (!operators.empty() && get_precedence(operators.back()) >= get_precedence(token)) {
                output += operators.back();
                operators.pop_back();
            }
            operators += token;
        }
    }

    // pop all remsaining operators from the stack to output
    while (!operators.empty()) {
        output += operators.back();
        operators.pop_back();
    }

    return output;
}



/*
Step 4 - Build Syntax Tree from Postfix using a Stack
*/
/*
It returns a shared_ptr<TreeNode>:
That is, a smart pointer to the root node of the syntax tree.
shared_ptr automatically manages memory: when no one references this node, it will be deleted.
*/
std::shared_ptr<TreeNode> build_syntax_tree(const std::string& postfix,
                                           const std::string& originalRegex,
                                           const std::string& regexWithConcat) {
    std::stack<std::shared_ptr<TreeNode>> stk;
    constexpr char EPSILON = '\0';
    
    // Process each token in postfix
    for (char token : postfix) {
        std::cout << "Processing token: " << token << "\n";
        
        if (isalnum(token) || token == EPSILON) { // if token is a symbol or ε: push Node(token)
            // create a leaf node and push onto stack
            stk.push(std::make_shared<TreeNode>(token));
        } else if (token == '*') { // if token is '*': child = pop stk; push Node('*', child)
            if (!stk.empty()) {
                auto operand = stk.top(); stk.pop();
                auto node = std::make_shared<TreeNode>(token);
                node->left = operand; // sets the left child of the * node to operand. only left child for unary operator
                stk.push(node);
            }
        } else if (is_operator(token)) { // if token is '.' or '|': right = pop stk; left = pop stk; push Node(token, left, right)
            // Binary operator: pop two operands
            if (stk.size() >= 2) {
                auto right = stk.top(); stk.pop(); // right = second operand
                auto left = stk.top(); stk.pop(); // left = first operand
                auto node = std::make_shared<TreeNode>(token); // new node for operator
                node->left = left;
                node->right = right;
                stk.push(node); // push the new subtree back onto stack
            }
        }
        
    }
    
    return stk.empty() ? nullptr : stk.top();
}