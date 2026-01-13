#include "minimized_dfa.h"
#include "nfa2dfa.h"
#include <map>
#include <queue>
#include <functional>
using namespace std;

/*
There are three classes of states that can be removed or merged from
the original DFA without affecting the language it accepts.

- Unreachable states are the states that are not reachable
  from the initial state of the DFA, for any input string. These states can be removed.
- Dead states are the states from which no final state is reachable.
  These states can be removed unless the automaton is required to be complete.
- Nondistinguishable states are those that cannot be distinguished from one another
  for any input string. These states can be merged.

DFA minimization is usually done in three steps:
- remove dead and unreachable states (this will accelerate the following step),
- merge nondistinguishable states,
- optionally, re-create a single dead state ("sink" state)
  if the resulting DFA is required to be complete.
*/

// This file minimizes DFA using Hopcroft's algorithm
// Input: DFA, Output: Minimized DFA

// Helper function to find which partition a state belongs to
int find_partition(const shared_ptr<DFAState>& state, 
                   const vector<set<shared_ptr<DFAState>>>& partitions) {
    for (size_t i = 0; i < partitions.size(); ++i) {
        if (partitions[i].find(state) != partitions[i].end()) {
            return i;
        }
    }
    return -1;
}

MinDFA minimize_dfa(const DFA& dfa, const set<char>& input_symbols) {
    MinDFA min_dfa;
    
    // Step 1: Collect all reachable states
    set<shared_ptr<DFAState>> reachable_states;
    map<int, shared_ptr<DFAState>> all_states; // id -> state mapping
    
    function<void(shared_ptr<DFAState>)> collect_reachable = [&](shared_ptr<DFAState> state) {
        if (reachable_states.find(state) != reachable_states.end()) return; // already visited
        reachable_states.insert(state);
        all_states[state->id] = state;
        
        for (const auto& [symbol, next_state] : state->transitions) {
            collect_reachable(next_state);
        }
    };
    collect_reachable(dfa.start_state);
    
    // Step 2: Initial partitioning into accepting and non-accepting states
    set<shared_ptr<DFAState>> accepting_states;
    set<shared_ptr<DFAState>> non_accepting_states;
    
    for (const auto& state : reachable_states) {
        if (state->is_accepting) {
            accepting_states.insert(state);
        } else {
            non_accepting_states.insert(state);
        }
    }
    
    vector<set<shared_ptr<DFAState>>> partitions;
    if (!non_accepting_states.empty()) {
        partitions.push_back(non_accepting_states);
    }
    if (!accepting_states.empty()) {
        partitions.push_back(accepting_states);
    }
    
    // Step 3: Refine partitions until no more refinement is possible
    bool changed = true;
    while (changed) {
        changed = false;
        vector<set<shared_ptr<DFAState>>> new_partitions;
        
        for (const auto& partition : partitions) {
            // Try to split this partition
            map<vector<int>, set<shared_ptr<DFAState>>> split_map;
            
            for (const auto& state : partition) {
                // Create a signature for this state based on where it transitions to
                vector<int> signature;
                
                for (char symbol : input_symbols) {
                    auto it = state->transitions.find(symbol);
                    if (it != state->transitions.end()) {
                        // Direct access to the target state
                        int target_partition = find_partition(it->second, partitions);
                        signature.push_back(target_partition);
                    } else {
                        signature.push_back(-1); // no transition for this symbol
                    }
                }
                
                split_map[signature].insert(state);
            }
            
            // If this partition was split into multiple groups
            if (split_map.size() > 1) {
                changed = true;
                for (const auto& [sig, states] : split_map) {
                    new_partitions.push_back(states);
                }
            } else {
                new_partitions.push_back(partition);
            }
        }
        
        partitions = new_partitions;
    }
    
    // Step 4: Build the minimized DFA
    map<int, shared_ptr<MinDFAState>> partition_to_state;
    
    // Create a MinDFAState for each partition
    for (size_t i = 0; i < partitions.size(); ++i) {
        auto min_state = make_shared<MinDFAState>(i);
        
        for (const auto& state : partitions[i]) {
            if (state->is_accepting) {
                min_state->is_accepting = true;
                break;
            }
        }
        
        partition_to_state[i] = min_state;
    }
    
    // Create transitions for the minimized DFA
    for (size_t i = 0; i < partitions.size(); ++i) {
        auto representative = *partitions[i].begin();
        auto min_state = partition_to_state[i];
        
        for (char symbol : input_symbols) {
            auto it = representative->transitions.find(symbol);
            if (it != representative->transitions.end()) {
                int target_partition = find_partition(it->second, partitions);
                if (target_partition != -1) {
                    min_state->transitions[symbol] = partition_to_state[target_partition];
                }
            }
        }
        
        // Check if this partition contains the original start state
        if (partitions[i].find(dfa.start_state) != partitions[i].end()) {
            min_dfa.start_state = min_state;
        }
    }

    for (const auto& [partition_idx, state] : partition_to_state) {
        min_dfa.all_states.insert(state);
    }
    
    return min_dfa;
}