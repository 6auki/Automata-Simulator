#ifndef MINIMIZED_DFA_H
#define MINIMIZED_DFA_H

#include "nfa2dfa.h"
#include <memory>
#include <vector>
#include <set>
#include <map>
#include <string>

// Minimized DFA State
struct MinDFAState {
    size_t id;
    map<char, shared_ptr<MinDFAState>> transitions;  // Single transition per symbol
    bool is_accepting;

    MinDFAState(int state_id) : id(state_id), is_accepting(false) {}
};

// Minimized DFA structure
struct MinDFA {
    std::shared_ptr<MinDFAState> start_state;
    set<shared_ptr<MinDFAState>> all_states;

    MinDFA() : start_state(nullptr) {}
};

// Function declaration
MinDFA minimize_dfa(const DFA& dfa, const std::set<char>& input_symbols);

#endif