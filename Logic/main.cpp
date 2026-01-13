#include "postfix.h"
#include "thompsons_construction.h"
#include "nfa2dfa.h"
#include "minimized_dfa.h"
#include <iostream>
#include <fstream>
#include <set>
#include <functional>
#include <algorithm>

using namespace std;

// Function to export to JSON
void export_nfa_to_json(const NFA& nfa, const string& filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Failed to open file: " << filename << endl;
        return;
    }

    file << "{\n";
    file << "  \"states\": [\n";
    
    // Collect all states
    set<shared_ptr<NFAState>> visited;
    vector<shared_ptr<NFAState>> all_states;
    function<void(shared_ptr<NFAState>)> collect = [&](shared_ptr<NFAState> state) {
        if (visited.find(state) != visited.end()) return;
        visited.insert(state);
        all_states.push_back(state);
        for (const auto& [symbol, next_states] : state->transitions) {
            for (const auto& next : next_states) {
                collect(next);
            }
        }
    };
    collect(nfa.start_state);
    
    // Write states
    for (size_t i = 0; i < all_states.size(); ++i) {
        auto state = all_states[i];
        file << "    {\"id\": " << state->id 
             << ", \"accepting\": " << (state->is_accepting ? "true" : "false") << "}";
        if (i < all_states.size() - 1) file << ",";
        file << "\n";
    }
    
    file << "  ],\n";
    file << "  \"start_state\": " << nfa.start_state->id << ",\n";
    file << "  \"transitions\": [\n";
    
    // Write transitions
    bool first_transition = true;
    for (const auto& state : all_states) {
        for (const auto& [symbol, next_states] : state->transitions) {
            for (const auto& next : next_states) {
                if (!first_transition) file << ",\n";
                first_transition = false;
                
                file << "    {\"from\": " << state->id 
                     << ", \"to\": " << next->id 
                     << ", \"symbol\": \"";
                
                if (symbol == EPSILON) {
                    file << "ε";
                } else {
                    file << symbol;
                }
                file << "\"}";
            }
        }
    }
    
    file << "\n  ]\n";
    file << "}\n";
    
    file.close();
}

void export_syntax_tree_to_json(const shared_ptr<TreeNode>& root, 
                                const string& original_regex,
                                const string& regex_with_concat,
                                const string& postfix,
                                const string& filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Failed to open file: " << filename << endl;
        return;
    }

    file << "{\n";
    file << "  \"original_regex\": \"" << original_regex << "\",\n";
    file << "  \"regex_with_concat\": \"" << regex_with_concat << "\",\n";
    file << "  \"postfix\": \"" << postfix << "\",\n";
    file << "  \"root\": ";
    
    // Recursive function to write tree nodes
    int node_id_counter = 0;
    function<void(const shared_ptr<TreeNode>&)> write_node = [&](const shared_ptr<TreeNode>& node) {
        if (!node) {
            file << "null";
            return;
        }
        
        file << "{\n";
        file << "    \"id\": " << node_id_counter++ << ",\n";
        file << "    \"value\": \"";
        if (node->value == EPSILON) {
            file << "ε";
        } else {
            file << node->value;
        }
        file << "\"";
        
        if (node->left) {
            file << ",\n    \"left\": ";
            write_node(node->left);
        }
        
        if (node->right) {
            file << ",\n    \"right\": ";
            write_node(node->right);
        }
        
        file << "\n  }";
    };
    
    write_node(root);
    
    file << "\n}\n";
    file.close();
}

void export_dfa_to_json(const DFA& dfa, const string& filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Failed to open file: " << filename << endl;
        return;
    }

    file << "{\n";
    file << "  \"states\": [\n";
    
    // Use all_states from DFA struct
    vector<shared_ptr<DFAState>> all_states(dfa.all_states.begin(), dfa.all_states.end());
    
    // Sort states by ID for consistent output
    sort(all_states.begin(), all_states.end(), 
         [](const shared_ptr<DFAState>& a, const shared_ptr<DFAState>& b) {
             return a->id < b->id;
         });
    
    // Write states
    for (size_t i = 0; i < all_states.size(); ++i) {
        auto state = all_states[i];
        file << "    {\"id\": " << state->id 
             << ", \"accepting\": " << (state->is_accepting ? "true" : "false") << "}";
        if (i < all_states.size() - 1) file << ",";
        file << "\n";
    }
    
    file << "  ],\n";
    file << "  \"start_state\": " << dfa.start_state->id << ",\n";
    file << "  \"transitions\": [\n";
    
    // Write transitions - DFA has single transition per symbol (NOT vector!)
    bool first_transition = true;
    for (const auto& state : all_states) {
        for (const auto& [symbol, next_state] : state->transitions) {  // FIX: single state, not vector
            if (!first_transition) file << ",\n";
            first_transition = false;
            
            file << "    {\"from\": " << state->id 
                 << ", \"to\": " << next_state->id 
                 << ", \"symbol\": \"" << symbol << "\"}";
        }
    }
    
    file << "\n  ]\n";
    file << "}\n";
    
    file.close();
}

void export_min_dfa_to_json(const MinDFA& min_dfa, const string& filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Failed to open file: " << filename << endl;
        return;
    }

    file << "{\n";
    file << "  \"states\": [\n";
    
    // Collect all states
    vector<shared_ptr<MinDFAState>> all_states(min_dfa.all_states.begin(), min_dfa.all_states.end());
    
    // Sort states by ID
    sort(all_states.begin(), all_states.end(), 
         [](const shared_ptr<MinDFAState>& a, const shared_ptr<MinDFAState>& b) {
             return a->id < b->id;
         });
    
    // Write states
    for (size_t i = 0; i < all_states.size(); ++i) {
        auto state = all_states[i];
        file << "    {\"id\": " << state->id 
             << ", \"accepting\": " << (state->is_accepting ? "true" : "false") << "}";
        if (i < all_states.size() - 1) file << ",";
        file << "\n";
    }
    
    file << "  ],\n";
    file << "  \"start_state\": " << min_dfa.start_state->id << ",\n";
    file << "  \"transitions\": [\n";
    
    // Write transitions - MinDFA has single transition per symbol
    bool first_transition = true;
    for (const auto& state : all_states) {
        for (const auto& [symbol, next_state] : state->transitions) {
            if (!first_transition) file << ",\n";
            first_transition = false;
            
            file << "    {\"from\": " << state->id 
                 << ", \"to\": " << next_state->id 
                 << ", \"symbol\": \"" << symbol << "\"}";
        }
    }
    
    file << "\n  ]\n";
    file << "}\n";
    
    file.close();
}

int main() {
    // Output directory for JSON files (can be changed to "../../../Visualize/" for CMake builds)
    string output_dir = "../../../Visualize/";  // Write to Visualize directory
    
    string regex = receive_regex_input();
    string regex_with_concat = insert_concatenation(regex);
    cout << "Regex with explicit concatenation: " << regex_with_concat << endl;

    string postfix = to_postfix(regex_with_concat);
    cout << "Postfix expression: " << postfix << endl;

    // pass all the information to build_syntax_tree for display
    auto syntax_tree_root = build_syntax_tree(postfix, regex, regex_with_concat);
    cout << "Syntax tree built successfully." << endl;

    if (!syntax_tree_root) {
        cout << "Error: Failed to build syntax tree." << endl;
        return 1;
    }

    export_syntax_tree_to_json(syntax_tree_root, regex, regex_with_concat, postfix, output_dir + "syntax_tree.json");

    NFA nfa = build_nfa_from_syntax_tree(syntax_tree_root);
    if (!nfa.start_state) {
        cout << "Error: Failed to build NFA." << endl;
        return 1;
    }
    cout << "NFA built successfully." << endl;

    // Export NFA to JSON for visualization
    export_nfa_to_json(nfa, output_dir + "nfa.json");

    // Collect input symbols from the NFA transitions
    set<char> input_symbols;
    function<void(shared_ptr<NFAState>)> collect_symbols = [&](shared_ptr<NFAState> state) {
        if (state == nullptr) return;
        static set<shared_ptr<NFAState>> visited;
        if (visited.find(state) != visited.end()) return;
        visited.insert(state);
        
        for (const auto& [symbol, states] : state->transitions) {
            if (symbol != EPSILON) {
                input_symbols.insert(symbol);
            }
            for (const auto& next_state : states) {
                collect_symbols(next_state);
            }
        }
    };
    collect_symbols(nfa.start_state);

    // Convert NFA to DFA
    DFA dfa = nfa_to_dfa(nfa, input_symbols);
    if (!dfa.start_state) {
        cout << "Error: Failed to convert NFA to DFA." << endl;
        return 1;
    }
    cout << "DFA built successfully." << endl;

    // Export DFA to JSON for visualization
    export_dfa_to_json(dfa, output_dir + "dfa.json");

    // Minimize DFA
    MinDFA min_dfa = minimize_dfa(dfa, input_symbols);
    if (!min_dfa.start_state) {
        cout << "Error: Failed to minimize DFA." << endl;
        return 1;
    }
    cout << "Minimized DFA built successfully." << endl;

    // Export minimized DFA
    export_min_dfa_to_json(min_dfa, output_dir + "min_dfa.json");

    return 0;
}