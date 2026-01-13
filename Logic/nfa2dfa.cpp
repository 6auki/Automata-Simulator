#include "nfa2dfa.h"
#include "thompsons_construction.h"
#include "postfix.h"
#include <set>
#include <queue>
#include <functional>
#include <iostream>  // ADD THIS for cout
using namespace std;

// Convert NFA to DFA using subset construction algorithm
// Added debug as well

// Function to compute epsilon closure of a set of NFA states
set<shared_ptr<NFAState>> epsilon_closure(const set<shared_ptr<NFAState>>& states) {
    set<shared_ptr<NFAState>> e_closure = states;
    queue<shared_ptr<NFAState>> to_process;

    for (const auto& state : states) {
        to_process.push(state);
    }

    while (!to_process.empty()) {
        auto state = to_process.front();
        to_process.pop();

        auto it = state->transitions.find(EPSILON);
        if (it != state->transitions.end()) {
            for (const auto& next_state : it->second) {
                if (e_closure.find(next_state) == e_closure.end()) {
                    e_closure.insert(next_state);
                    to_process.push(next_state);
                }
            }
        }
    }

    return e_closure;
}

DFA nfa_to_dfa(const NFA& nfa, const set<char>& input_symbols) {
    DFA dfa;
    map<set<shared_ptr<NFAState>>, shared_ptr<DFAState>> state_mapping;
    queue<set<shared_ptr<NFAState>>> to_process;

    // Create the DFA's start state
    set<shared_ptr<NFAState>> start_set = epsilon_closure({nfa.start_state});

    // DEBUG: Print start state composition
    cout << "\n=== NFA to DFA Conversion Debug ===\n";
    cout << "Start DFA state (0) contains NFA states: {";
    for (const auto& s : start_set) {
        cout << s->id << " ";
    }
    cout << "}\n";
    cout << "Start state is accepting: ";
    bool start_accepting = false;
    for (const auto& s : start_set) {
        if (s->is_accepting) {
            start_accepting = true;
            cout << "YES (contains NFA accepting state " << s->id << ")\n";
            break;
        }
    }
    if (!start_accepting) cout << "NO\n";
    cout << "\n";

    auto dfa_start_state = make_shared<DFAState>(0);
    dfa.start_state = dfa_start_state;
    state_mapping[start_set] = dfa_start_state;
    to_process.push(start_set);

    int dfa_state_id_counter = 1;

    // Create the DFA's transition table
    while (!to_process.empty()) {
        auto current_set = to_process.front();
        to_process.pop();
        auto current_dfa_state = state_mapping[current_set];

        // Mark as accepting if any NFA state in set is accepting
        for (const auto& nfa_state : current_set) {
            if (nfa_state->is_accepting) {
                current_dfa_state->is_accepting = true;
                break;
            }
        }

        // For each input symbol, compute the next set of NFA states
        for (char symbol : input_symbols) {
            set<shared_ptr<NFAState>> next_set;

            // Collect all NFA states reachable via this symbol
            for (const auto& nfa_state : current_set) {
                auto it = nfa_state->transitions.find(symbol);
                if (it != nfa_state->transitions.end()) {
                    for (const auto& next_state : it->second) {
                        next_set.insert(next_state);
                    }
                }
            }
            
            // Compute epsilon closure of the next set
            next_set = epsilon_closure(next_set);
            
            if (next_set.empty()) {
                continue; // no transition for this symbol
            }

            // DEBUG: Print transition information
            int next_dfa_id = -1;
            if (state_mapping.find(next_set) != state_mapping.end()) {
                next_dfa_id = state_mapping[next_set]->id;
            } else {
                next_dfa_id = dfa_state_id_counter;
            }
            
            cout << "DFA state " << current_dfa_state->id 
                 << " --" << symbol << "--> DFA state " << next_dfa_id
                 << " (NFA states: {";
            for (const auto& s : next_set) {
                cout << s->id << " ";
            }
            cout << "}";
            
            // Check if accepting
            bool is_accepting = false;
            for (const auto& s : next_set) {
                if (s->is_accepting) {
                    is_accepting = true;
                    cout << " - ACCEPTING";
                    break;
                }
            }
            cout << ")\n";

            // Create new DFA state if needed
            if (state_mapping.find(next_set) == state_mapping.end()) {
                auto new_dfa_state = make_shared<DFAState>(dfa_state_id_counter++);
                state_mapping[next_set] = new_dfa_state;
                to_process.push(next_set);
            }
            
            // Add the transition to the DFA state
            current_dfa_state->transitions[symbol] = state_mapping[next_set];
        }
    }
    
    cout << "\n=== End Debug ===\n\n";
    
    // Populate all_states
    for (const auto& [nfa_set, dfa_state] : state_mapping) {
        dfa.all_states.insert(dfa_state);
    }
    
    return dfa;
}