#!/usr/bin/env python3
"""
Visualize the DFA (Deterministic Finite Automaton) converted from NFA.
Reads from dfa.json and displays the automaton graphically.
"""

import json
import matplotlib.pyplot as plt
import networkx as nx
from matplotlib.patches import Circle, FancyArrowPatch
import sys


def load_dfa(filename='dfa.json'):
    """Load the DFA from JSON file."""
    try:
        with open(filename, 'r', encoding='utf-8') as f:
            return json.load(f)
    except FileNotFoundError:
        print(f"Error: {filename} not found.")
        sys.exit(1)
    except json.JSONDecodeError:
        print(f"Error: Invalid JSON in {filename}")
        sys.exit(1)


def draw_dfa(dfa_data):
    """Draw the DFA using matplotlib and networkx."""
    states = dfa_data.get('states', [])
    start_state_id = dfa_data.get('start_state')
    transitions = dfa_data.get('transitions', [])
    
    if not states:
        print("Error: No states found in DFA.")
        return
    
    # Create directed graph
    G = nx.DiGraph()
    
    # Add nodes
    for state in states:
        G.add_node(state['id'], accepting=state['accepting'])
    
    # Add edges with labels
    edge_labels = {}
    for trans in transitions:
        from_state = trans['from']
        to_state = trans['to']
        symbol = trans['symbol']
        
        # Handle multiple transitions between same states
        if G.has_edge(from_state, to_state):
            # Append to existing label
            current_label = edge_labels[(from_state, to_state)]
            if symbol not in current_label.split(', '):
                edge_labels[(from_state, to_state)] = current_label + ', ' + symbol
        else:
            G.add_edge(from_state, to_state)
            edge_labels[(from_state, to_state)] = symbol
    
    # Create layout
    try:
        pos = nx.spring_layout(G, k=2.5, iterations=50, seed=42)
    except:
        pos = nx.spring_layout(G, seed=42)
    
    # Create figure
    fig, ax = plt.subplots(figsize=(14, 10))
    
    # Draw edges
    for (u, v) in G.edges():
        # Handle self-loops differently
        if u == v:
            # Draw self-loop
            x, y = pos[u]
            circle = Circle((x, y + 0.15), 0.1, fill=False, edgecolor='black', 
                          linewidth=1.5, linestyle='--')
            ax.add_patch(circle)
            # Add label
            label = edge_labels[(u, v)]
            ax.text(x, y + 0.28, label, ha='center', va='bottom', 
                   fontsize=9, bbox=dict(boxstyle='round,pad=0.3', 
                   facecolor='white', edgecolor='gray', alpha=0.8))
        else:
            # Regular edge
            x1, y1 = pos[u]
            x2, y2 = pos[v]
            
            arrow = FancyArrowPatch((x1, y1), (x2, y2),
                                   arrowstyle='->', mutation_scale=20,
                                   linewidth=1.5, color='black',
                                   connectionstyle="arc3,rad=0.1",
                                   zorder=1)
            ax.add_patch(arrow)
            
            # Add edge label
            label = edge_labels[(u, v)]
            mid_x, mid_y = (x1 + x2) / 2, (y1 + y2) / 2
            ax.text(mid_x, mid_y, label, ha='center', va='center',
                   fontsize=9, bbox=dict(boxstyle='round,pad=0.3',
                   facecolor='white', edgecolor='gray', alpha=0.8),
                   zorder=2)
    
    # Draw nodes
    for node_id in G.nodes():
        x, y = pos[node_id]
        is_accepting = G.nodes[node_id]['accepting']
        is_start = (node_id == start_state_id)
        
        # Determine colors
        if is_start and is_accepting:
            node_color = '#90EE90'  # Light green
            edge_color = '#228B22'  # Forest green
        elif is_start:
            node_color = '#87CEEB'  # Sky blue
            edge_color = '#4169E1'  # Royal blue
        elif is_accepting:
            node_color = '#FFB6C1'  # Light pink
            edge_color = '#DC143C'  # Crimson
        else:
            node_color = '#F0F0F0'  # Light gray
            edge_color = '#696969'  # Dim gray
        
        # Draw outer circle for accepting states
        if is_accepting:
            outer_circle = Circle((x, y), 0.22, fill=False, edgecolor=edge_color,
                                linewidth=2.5, zorder=3)
            ax.add_patch(outer_circle)
        
        # Draw main circle
        circle = Circle((x, y), 0.18, facecolor=node_color, edgecolor=edge_color,
                       linewidth=2, zorder=4)
        ax.add_patch(circle)
        
        # Draw node label
        ax.text(x, y, str(node_id), ha='center', va='center',
               fontsize=11, fontweight='bold', zorder=5)
        
        # Draw arrow pointing to start state
        if is_start:
            arrow_start_x = x - 0.4
            arrow_start_y = y
            arrow = FancyArrowPatch((arrow_start_x, arrow_start_y), (x - 0.2, y),
                                   arrowstyle='->', mutation_scale=20,
                                   linewidth=2.5, color=edge_color, zorder=2)
            ax.add_patch(arrow)
            ax.text(arrow_start_x - 0.1, arrow_start_y, 'Start',
                   ha='right', va='center', fontsize=10, fontweight='bold')
    
    # Set title
    ax.set_title('DFA (Deterministic Finite Automaton)', 
                fontsize=16, fontweight='bold', pad=20)
    
    # Add state count info
    info_text = f"Total States: {len(states)}\n"
    info_text += f"Accepting States: {sum(1 for s in states if s['accepting'])}\n"
    info_text += f"Transitions: {len(transitions)}"
    ax.text(0.02, 0.98, info_text, transform=ax.transAxes,
           fontsize=10, verticalalignment='top',
           bbox=dict(boxstyle='round', facecolor='wheat', alpha=0.5))
    
    # Create legend
    legend_elements = [
        plt.Line2D([0], [0], marker='o', color='w', markerfacecolor='#87CEEB',
                  markeredgecolor='#4169E1', markersize=12, markeredgewidth=2,
                  label='Start State', linestyle='None'),
        plt.Line2D([0], [0], marker='o', color='w', markerfacecolor='#FFB6C1',
                  markeredgecolor='#DC143C', markersize=12, markeredgewidth=2,
                  label='Accept State', linestyle='None'),
        plt.Line2D([0], [0], marker='o', color='w', markerfacecolor='#90EE90',
                  markeredgecolor='#228B22', markersize=12, markeredgewidth=2,
                  label='Start & Accept', linestyle='None'),
        plt.Line2D([0], [0], marker='o', color='w', markerfacecolor='#F0F0F0',
                  markeredgecolor='#696969', markersize=12, markeredgewidth=2,
                  label='Regular State', linestyle='None'),
    ]
    ax.legend(handles=legend_elements, loc='upper right', fontsize=10)
    
    # Set axis properties
    ax.set_xlim(min(p[0] for p in pos.values()) - 0.5,
                max(p[0] for p in pos.values()) + 0.5)
    ax.set_ylim(min(p[1] for p in pos.values()) - 0.5,
                max(p[1] for p in pos.values()) + 0.5)
    ax.axis('off')
    ax.set_aspect('equal')
    
    plt.tight_layout()
    plt.savefig('dfa.png', dpi=300, bbox_inches='tight')
    print("DFA visualization saved as 'dfa.png'")
    plt.show(block=False)  # Non-blocking
    plt.pause(0.1)


def main():
    """Main function to load and visualize the DFA."""
    dfa_data = load_dfa()
    
    # Print to terminal
    print("\n" + "="*70)
    print("DFA (DETERMINISTIC FINITE AUTOMATON)")
    print("="*70)
    
    states = dfa_data.get('states', [])
    start_state_id = dfa_data.get('start_state')
    transitions = dfa_data.get('transitions', [])
    
    print(f"Total States:      {len(states)}")
    print(f"Start State:       {start_state_id}")
    
    accepting_states = [s['id'] for s in states if s['accepting']]
    print(f"Accepting States:  {{{', '.join(map(str, accepting_states))}}}")
    print(f"Total Transitions: {len(transitions)}")
    print()
    
    # Print transition table
    print("Transition Table:")
    print("-" * 70)
    print(f"{'From':<8} {'Symbol':<10} {'To':<8}")
    print("-" * 70)
    
    for trans in sorted(transitions, key=lambda t: (t['from'], t['symbol'], t['to'])):
        print(f"{trans['from']:<8} {trans['symbol']:<10} {trans['to']:<8}")
    
    print("="*70)
    print()
    
    draw_dfa(dfa_data)


if __name__ == '__main__':
    main()