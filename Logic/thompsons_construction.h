#ifndef THOMPSONS_CONSTRUCTION_H
#define THOMPSONS_CONSTRUCTION_H

#include <memory>
#include <vector>
#include <map>
#include <string>

// Forward declaration of TreeNode (so we don't need to include postfix_stack_method.h)
struct TreeNode;

// Use '\0' as epsilon
constexpr char EPSILON = '\0';

// NFA State
struct NFAState {
    int id;
    std::map<char, std::vector<std::shared_ptr<NFAState>>> transitions;
    bool is_accepting;

    NFAState(int state_id) : id(state_id), is_accepting(false) {}
};

// NFA structure
struct NFA {
    std::shared_ptr<NFAState> start_state;
    std::shared_ptr<NFAState> accept_state;

    NFA() : start_state(nullptr), accept_state(nullptr) {}
};

// Functions
std::shared_ptr<NFAState> create_state();
NFA build_nfa_from_syntax_tree(const std::shared_ptr<TreeNode>& node);

#endif