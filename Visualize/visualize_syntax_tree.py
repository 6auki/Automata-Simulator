#!/usr/bin/env python3
"""
Visualize the syntax tree generated from a regular expression.
Reads from syntax_tree.json and displays the tree graphically.
"""

import json
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
from matplotlib.patches import FancyBboxPatch
import sys


def load_syntax_tree(filename='syntax_tree.json'):
    """Load the syntax tree from JSON file."""
    try:
        with open(filename, 'r', encoding='utf-8') as f:
            return json.load(f)
    except FileNotFoundError:
        print(f"Error: {filename} not found.")
        sys.exit(1)
    except json.JSONDecodeError:
        print(f"Error: Invalid JSON in {filename}")
        sys.exit(1)


def calculate_tree_positions(node, x=0, y=0, level=0, x_spacing=1.5, y_spacing=1.2):
    """
    Recursively calculate positions for tree nodes.
    Returns a dictionary mapping node IDs to (x, y) positions and the tree's width.
    """
    if node is None:
        return {}, 0
    
    positions = {}
    
    # Calculate positions for left subtree
    left_positions = {}
    left_width = 0
    if 'left' in node and node['left'] is not None:
        left_positions, left_width = calculate_tree_positions(
            node['left'], x, y - y_spacing, level + 1, x_spacing, y_spacing
        )
    
    # Position current node
    current_x = x + left_width
    positions[node['id']] = (current_x, y)
    
    # Calculate positions for right subtree
    right_positions = {}
    right_width = 0
    if 'right' in node and node['right'] is not None:
        right_positions, right_width = calculate_tree_positions(
            node['right'], current_x + x_spacing, y - y_spacing, level + 1, x_spacing, y_spacing
        )
    
    # Merge all positions
    positions.update(left_positions)
    positions.update(right_positions)
    
    total_width = left_width + x_spacing + right_width
    
    return positions, total_width


def draw_tree(tree_data):
    """Draw the syntax tree using matplotlib."""
    root = tree_data.get('root')
    if root is None:
        print("Error: No root node found in syntax tree.")
        return
    
    # Calculate positions
    positions, _ = calculate_tree_positions(root)
    
    # Create figure
    fig, ax = plt.subplots(figsize=(12, 8))
    ax.set_aspect('equal')
    
    # Draw edges first (so they appear behind nodes)
    def draw_edges(node):
        if node is None:
            return
        
        node_id = node['id']
        node_pos = positions[node_id]
        
        # Draw edge to left child
        if 'left' in node and node['left'] is not None:
            left_id = node['left']['id']
            left_pos = positions[left_id]
            ax.plot([node_pos[0], left_pos[0]], [node_pos[1], left_pos[1]], 
                   'k-', linewidth=2, zorder=1)
            draw_edges(node['left'])
        
        # Draw edge to right child
        if 'right' in node and node['right'] is not None:
            right_id = node['right']['id']
            right_pos = positions[right_id]
            ax.plot([node_pos[0], right_pos[0]], [node_pos[1], right_pos[1]], 
                   'k-', linewidth=2, zorder=1)
            draw_edges(node['right'])
    
    draw_edges(root)
    
    # Draw nodes
    def draw_nodes(node):
        if node is None:
            return
        
        node_id = node['id']
        x, y = positions[node_id]
        value = node['value']
        
        # Determine node color based on type
        if value in ['*', '.', '|']:
            # Operator nodes
            color = '#FFD700'  # Gold
            edge_color = '#FF8C00'  # Dark orange
        else:
            # Operand nodes (letters or epsilon)
            color = '#87CEEB'  # Sky blue
            edge_color = '#4682B4'  # Steel blue
        
        # Draw node circle
        circle = plt.Circle((x, y), 0.3, color=color, ec=edge_color, 
                          linewidth=2.5, zorder=3)
        ax.add_patch(circle)
        
        # Draw node label
        ax.text(x, y, value, ha='center', va='center', 
               fontsize=14, fontweight='bold', zorder=4)
        
        # Recursively draw children
        if 'left' in node and node['left'] is not None:
            draw_nodes(node['left'])
        if 'right' in node and node['right'] is not None:
            draw_nodes(node['right'])
    
    draw_nodes(root)
    
    # Set title with regex information
    title = f"Syntax Tree\n"
    title += f"Original: {tree_data.get('original_regex', 'N/A')}\n"
    title += f"With Concat: {tree_data.get('regex_with_concat', 'N/A')}\n"
    title += f"Postfix: {tree_data.get('postfix', 'N/A')}"
    ax.set_title(title, fontsize=12, fontweight='bold', pad=20)
    
    # Create legend
    operator_patch = mpatches.Patch(color='#FFD700', label='Operators (*, ., |)', 
                                   edgecolor='#FF8C00', linewidth=2)
    operand_patch = mpatches.Patch(color='#87CEEB', label='Operands (a-z, ε)', 
                                  edgecolor='#4682B4', linewidth=2)
    ax.legend(handles=[operator_patch, operand_patch], loc='upper right', fontsize=10)
    
    # Remove axes
    ax.set_xlim(min(p[0] for p in positions.values()) - 1, 
                max(p[0] for p in positions.values()) + 1)
    ax.set_ylim(min(p[1] for p in positions.values()) - 1, 
                max(p[1] for p in positions.values()) + 1)
    ax.axis('off')
    
    plt.tight_layout()
    plt.savefig('syntax_tree.png', dpi=300, bbox_inches='tight')
    print("Syntax tree visualization saved as 'syntax_tree.png'")
    plt.show(block=False)  # Non-blocking - keeps window open but allows script to continue
    plt.pause(0.1)  # Small pause to ensure window renders


def main():
    """Main function to load and visualize the syntax tree."""
    tree_data = load_syntax_tree()
    
    # Print to terminal
    print("\n" + "="*70)
    print("SYNTAX TREE")
    print("="*70)
    print(f"Original Regex:      {tree_data.get('original_regex', 'N/A')}")
    print(f"With Concatenation:  {tree_data.get('regex_with_concat', 'N/A')}")
    print(f"Postfix Notation:    {tree_data.get('postfix', 'N/A')}")
    print("="*70)
    
    # Print tree structure
    root = tree_data.get('root')
    if root:
        print("\nTree Structure (preorder traversal):")
        print_tree(root, prefix="", is_tail=True)
        print()
    
    draw_tree(tree_data)


def print_tree(node, prefix="", is_tail=True):
    """Print tree structure to terminal in a visual format."""
    if node is None:
        return
    
    # Get node value, handle epsilon
    value = node['value']
    if value == 'ε':
        display_value = 'ε (epsilon)'
    else:
        display_value = value
    
    # Print current node
    connector = "└── " if is_tail else "├── "
    print(f"{prefix}{connector}{display_value}")
    
    # Prepare prefix for children
    extension = "    " if is_tail else "│   "
    new_prefix = prefix + extension
    
    # Print children
    has_left = 'left' in node and node['left'] is not None
    has_right = 'right' in node and node['right'] is not None
    
    if has_left:
        print_tree(node['left'], new_prefix, not has_right)
    
    if has_right:
        print_tree(node['right'], new_prefix, True)


if __name__ == '__main__':
    main()