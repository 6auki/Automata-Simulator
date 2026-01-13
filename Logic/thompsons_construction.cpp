#include "thompsons_construction.h"
#include "postfix.h"

#include <iostream>
#include <map>
#include <string>
#include <stack>
#include <memory>
#include <vector>
using namespace std;

// This file does
// 5. construct NFA from syntax tree using Thompson's construction
//  - recursively traverse the syntax tree of the regex and build an NFA for each node
// next step in nfa_visualizer.cpp file:
// 6. simulate NFA

/*
structs and function declarations are in the header (thompsons_construction.h)
function implementations (like create_state() and build_nfa_from_syntax_tree()) are in this .cpp file
*/

// NFA structure
// (Q,Σ,δ,q0​,F)
// Q: set of states
// Σ: input alphabet
// δ: transition function
// q0: initial state
// F: set of accepting states

// already done:
// 1. receive regex as input
// 2. add explicit concatenation operators .
// 3. convert the regex into postfix using the shunting-yard algorithm
// 4. build syntax tree from postfix using a stack

// now I use trees and stack to build NFA from regexp


/*
Step 5 - Construct NFA from Syntax Tree Using Thompson's Construction
*/
// Global state ID counter
int state_id_counter = 0;

// Function to create a new NFA state with a unique ID
shared_ptr<NFAState> create_state() {
    return make_shared<NFAState>(state_id_counter++);
}

// Thompson's construction to build NFA from syntax tree
NFA build_nfa_from_syntax_tree(const shared_ptr<TreeNode>& node) {
    if (!node) {
        return NFA();
    }
    if (isalnum(node->value) || node->value == EPSILON) { // Base case: single symbol or ε
        NFA nfa;
        nfa.start_state = create_state();
        nfa.accept_state = create_state();
        
        nfa.accept_state->is_accepting = true;

        char symbol = node->value;
        nfa.start_state->transitions[symbol].push_back(nfa.accept_state);
        return nfa;
    } else if (node->value == '*') { // Kleene star
        NFA sub_nfa = build_nfa_from_syntax_tree(node->left);
        NFA nfa;
        nfa.start_state = create_state();
        nfa.accept_state = create_state();
        nfa.accept_state->is_accepting = true;

        // old sub-NFA accept should no longer be final
        sub_nfa.accept_state->is_accepting = false;

        // ε-transitions for star operation
        nfa.start_state->transitions[EPSILON].push_back(sub_nfa.start_state);
        nfa.start_state->transitions[EPSILON].push_back(nfa.accept_state);
        sub_nfa.accept_state->transitions[EPSILON].push_back(sub_nfa.start_state);
        sub_nfa.accept_state->transitions[EPSILON].push_back(nfa.accept_state);
        return nfa;
    } else if (node->value == '.') { // Concatenation
        NFA left_nfa = build_nfa_from_syntax_tree(node->left);
        NFA right_nfa = build_nfa_from_syntax_tree(node->right);

        // old left NFA accept should no longer be final
        left_nfa.accept_state->is_accepting = false;

        // connect left NFA's accept state to right NFA's start state via ε-transition
        left_nfa.accept_state->transitions[EPSILON].push_back(right_nfa.start_state);

        NFA nfa;
        nfa.start_state = left_nfa.start_state;
        nfa.accept_state = right_nfa.accept_state; //final state is right NFA's accept state, only final state
        return nfa;
    } else if (node->value == '|') { // Alternation
        NFA left_nfa = build_nfa_from_syntax_tree(node->left);
        NFA right_nfa = build_nfa_from_syntax_tree(node->right);

        // old accepts are no longer final
        left_nfa.accept_state->is_accepting = false;
        right_nfa.accept_state->is_accepting = false;

        NFA nfa;
        nfa.start_state = create_state();
        nfa.accept_state = create_state();
        nfa.accept_state->is_accepting = true;

        // ε-transitions from new start state to both sub-NFAs
        nfa.start_state->transitions[EPSILON].push_back(left_nfa.start_state);
        nfa.start_state->transitions[EPSILON].push_back(right_nfa.start_state);
        // ε-transitions from both sub-NFAs' accept states to new accept state
        left_nfa.accept_state->transitions[EPSILON].push_back(nfa.accept_state);
        right_nfa.accept_state->transitions[EPSILON].push_back(nfa.accept_state);
        return nfa;
    }
    return NFA(); // should not reach here
}