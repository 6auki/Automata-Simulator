#ifndef NFA2DFA_H
#define NFA2DFA_H

#include <memory>
#include <vector>
#include <set>
#include <map>
#include <string>

// Add these using declarations
using std::shared_ptr;
using std::set;
using std::map;
using std::make_shared;

struct NFAState;
struct NFA;

// DFA State
struct DFAState {
    int id;
    map<char, shared_ptr<DFAState>> transitions;  // Single transition per symbol
    bool is_accepting;

    DFAState(int state_id) : id(state_id), is_accepting(false) {}
};

// DFA structure
struct DFA {
    shared_ptr<DFAState> start_state;
    set<shared_ptr<DFAState>> all_states;

    DFA() : start_state(nullptr) {}
};

// Functions
set<shared_ptr<NFAState>> epsilon_closure(const set<shared_ptr<NFAState>>& states);
DFA nfa_to_dfa(const NFA& nfa, const set<char>& input_symbols);

#endif