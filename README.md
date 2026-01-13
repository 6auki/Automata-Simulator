# Automata Simulator C++
## Regex → ε-NFA → DFA → Minimized DFA

This project is an automata simulator that takes a regular expression as input and constructs a corresponding ε-NFA (Nondeterministic Finite Automaton with epsilon transitions) using Thompson’s construction, and a minimized DFA that recognizes the same language. The simulator demonstrates the full pipeline from regex to NFA, then to DFA, and finally to a minimized DFA, allowing users to simulate strings and observe the automaton in action.

Some visualization files are omitted.

---


## Workflow

### [Input](#1-input)
- Receives a **regular expression** from the user.

### [Preprocessing](#2-preprocessing)
- Adding **explicit concatenation operators** to clarify operator precedence.

### [Postfix Conversion](#3-postfix-conversion)
- Transforming the regex into **postfix notation** using the **shunting-yard algorithm**.

### [Syntax Tree Construction Using Stack](#4-syntax-tree-construction-using-stack)
- Building a **syntax tree** from the postfix expression using a **stack**.
- Additionally simulating **tree construction**.
- Each node represents a regex expression type:
  - Symbol  
  - Epsilon (`ε`)  
  - Concatenation (`·`)  
  - Alternation (`|`)  
  - Kleene Star (`*`)

### [NFA Construction (Thompson’s Construction)](#5-nfa-construction-thompsons-construction)
- Recursively traversing the syntax tree.
- Constructing an **ε-NFA fragment** for each node and combining child fragments.
- Each fragment contains:
  - **Start state**  
  - **Accepting state(s)**  
  - **Transitions**, including **ε-transitions**

### [NFA → DFA Conversion (Subset Construction)](#6-ε-nfa-→-dfa-conversion-subset-construction)
- Converting the **ε-NFA** into an equivalent **DFA**:
  - Each DFA state corresponds to a **set of NFA states** (power set construction).

### [DFA Minimization](#7-dfa-minimization)
- Minimizing the DFA using **partitioning algorithms**:
  - Removing redundant states  
  - Producing a **canonical, minimal DFA**

### [Automata Visualization](#8-automata-visualization)
- Simulating the **Syntax Tree for RegEx**, **ε-NFA**, **DFA**, and **minimized DFA** on input strings for efficient acceptance checking.

### [How to Run](#9-how-to-run)


---

## Outcome
- Converts any valid regex into an **Syntax Tree & ε-NFA & DFA & Equivalent Minimized DFA**.  
- Supports simulation at both **NFA** and **DFA** levels.  
- Provides a **visualizable automaton pipeline**:
  - **regex → syntax tree → ε-NFA → DFA → minimized DFA.**

---



## 1. Input
- Receive a **regular expression** from the user.



## 2. Preprocessing
Add **explicit concatenation operator** `.` to clarify operator precedence.

---

We need to add an explicit concatenation operator `.` before converting a regex to an NFA because:
- Regexes often imply concatenation (e.g., `ab`) without a symbol.
- NFA construction algorithms (like Thompson’s) require unambiguous operator precedence.
- Adding `.` clarifies which parts are concatenated vs. alternated `(a.b|c*)` so postfix conversion and syntax tree construction work correctly.

**`e.g. a(b|c)*d → a·(b|c)*·d`**
loop through the whole regex and check conditions for inserting concatenation operator
concatenate if:
  - `a b   →  a.b`
  - `a (   →  a.(`
  - `) a   →  ).a`
  - `* a   →  *.a`



## 3. Postfix Conversion
Convert infix regex to postfix using the shunting-yard algorithm.

---

### **Why we need this:**
1. Regex is infix and ambiguous
   Example: `a.b|c*`
   Without clear precedence, you can’t tell if it’s `(a.b) | (c*)` or `a.(b|c*)`.
2. Trees require clear parent-child relationships
   When building a syntax tree, each operator becomes a node with its operands as children.
   If the regex is infix, figuring out which operands go with which operator is tricky.
3. Shunting-Yard → postfix
   Converts infix → postfix (RPN), making the order of operations explicit.
   Postfix is perfect for stack-based tree construction: you push operands, then pop them when an operator is encountered to form a tree node.

**In short:** use Shunting-Yard so the syntax tree correctly reflects regex precedence, which is essential for building the NFA properly.


**This algorithm needs a little modification for RegEx.**
Operator precedence is defined as: `* > . > |`
- `case '*': return 3; // highest precedence`
- `case '.': return 2;`
- `case '|': return 1; // lowest precedence`

---

### **Step-by-step process**

1. Initialize:
   - An empty **operator stack**  
   - An empty **output string**  

2. Loop through the regex **left-to-right**.  

3. Handle **five types of tokens**:

   - **Alphabet letter or ε (epsilon):**  
     - Add directly to output.

   - **`(` (left parenthesis):**  
     - Push onto the operator stack.

   - **`)` (right parenthesis):**  
     - Pop operators from the stack to output **until `(` is popped**.

   - **Kleene star `*`:**  
     - Since it is postfix, add directly to output.

   - **Binary operators `.` or `|`:**  
     - While:
       - The stack is not empty, and  
       - The top of the stack is an operator, and  
       - `precedence(top) ≥ precedence(current)`  
     - Pop from stack to output.  
     - Then push the current operator onto the stack.

4. **End of regex:**  
   - Pop all remaining operators from the stack to the output.

---

This results in a **postfix regex** ready for **stack-based syntax tree construction** and subsequent **NFA building**.



## 4. Syntax Tree Construction using Stack

After converting a regex to **postfix** using the Shunting-Yard algorithm:  

**Example:**  
Regex: `a.b|c*` → Postfix: `ab.c*|`  

- Postfix notation makes **operator precedence explicit**, so the evaluation order is clear.  
- To construct an NFA using **Thompson’s construction**, we first build a **syntax tree** representing the regex.  
- Each **operator** (`*`, `.`, `|`) becomes a **tree node**.  
- Each **operand** (symbol) becomes a **leaf node**.  
- A **stack-based algorithm** is ideal for building the tree from postfix because postfix expressions are evaluated **left-to-right** using a stack.

---

### **Step-by-step process:**

1. **Initialize an empty stack.**

2. **Scan the postfix regex from left to right:**

   - **Operand (symbol):**
     - Create a tree node for the symbol and **push it onto the stack**.

   - **Unary operator (`*`):**
     - Pop **one node** from the stack → it becomes the **child** of the operator node.  
     - Push the new operator node back onto the stack.

   - **Binary operator (`.` or `|`):**
     - Pop **two nodes** from the stack → they become the **children** (left and right) of the operator node.  
     - Push the new operator node back onto the stack.

3. **At the end:**
   - The stack contains **one node**, which is the **root of the syntax tree**.

---

**Node types in the syntax tree:**

- **Symbol:** a literal character from the regex  
- **Epsilon (`ε`):** represents an empty transition  
- **Concatenation (`·`)**  
- **Alternation (`|`)**  
- **Kleene Star (`*`)**



## 5. NFA Construction (Thompson’s Construction)
### **Why we need it:**

Thompson's construction is used to convert a **regular expression** into an **ε-NFA**.  
- NFAs are easier to construct **directly from regex patterns** than DFAs.  
- Each regex operator (`*`, `.`, `|`) has a **corresponding NFA fragment**.  
- Once the ε-NFA is built, it can later be **converted to a DFA** (via subset/powerset construction) for efficient string matching.  

This step is essential because it provides a **structured automaton** representing all possible ways the regex can match input strings.

---

### **How it works (step-by-step):**

1. Traverse the syntax tree recursively. Each node is either an operand (symbol) or an operator (*, ., |).

2. Operand (symbol) node:
   - Create an NFA fragment with a start state, an accept state, and a transition labeled with the symbol.

3. Concatenation (.) node:
   - Recursively build NFAs for left and right children.
   - Connect the accept state of the left NFA to the start state of the right NFA using an ε-transition.
   - The combined NFA's start state is the left's start, accept state is the right's accept.

4. Alternation (|) node:
   - Recursively build NFAs for left and right children.
   - Create a new start state and new accept state.
   - Add ε-transitions from the new start to each child’s start, and from each child’s accept to the new accept state.

5. Kleene star (*) node:
   - Recursively build the NFA for the child.
   - Create a new start and accept state.
   - Add ε-transitions:
     - From new start → child start
     - From new start → new accept (for zero repetitions)
     - From child accept → child start (loop for repetition)
     - From child accept → new accept

6. The root node of the tree produces the final ε-NFA, which can then be converted to a DFA.



## 6. ε-NFA → DFA Conversion (Subset Construction)
After building the ε-NFA, the next step is to convert it into an **equivalent DFA**.  
This allows for **deterministic and efficient string matching**.

---

### **Key Idea:**  
- Each DFA state corresponds to a **set of NFA states**.  
- This is why it’s called **subset construction** (or powerset construction).  

---

### **Step-by-step:**

1. **Compute the start state of the DFA:**  
   - Take the **ε-closure** of the NFA's start state.  
   - This set of NFA states becomes the **DFA's start state**.  
   - *(Epsilon closure: all states reachable via ε-transitions from a given set of NFA states, including the states themselves.)*

2. **Process DFA states iteratively:**  
   - Maintain a **queue of unprocessed DFA states**.  
   - For each DFA state (a set of NFA states) and each input symbol:
     1. Find all NFA states reachable from **any state in the set** via that symbol.  
     2. Compute the **ε-closure** of the resulting set.  
     3. If this set has not been seen before, create a **new DFA state** and add it to the queue.  
     4. Add a transition in the DFA for this symbol to the resulting DFA state.

3. **Mark accepting states:**  
   - If **any NFA state in a DFA state set is accepting**, mark the DFA state as accepting.

---

**Optimization:**  
- Instead of computing all DFA states upfront, only **compute states as needed**, while processing input symbols.  
- This avoids creating unreachable states and saves memory.

---

**Result:**  
- The final DFA recognizes the **same language** as the original regex, but is **deterministic**, so it can match strings efficiently.



## 7. DFA Minimization
After constructing a DFA, it can often be **simplified** without changing the language it recognizes.  
This project uses **Hopcroft’s algorithm** to produce a **canonical, minimal DFA**.

---

### **Why minimize a DFA?**  
- Remove **redundant states** to reduce memory usage and improve efficiency.  
- Produce a **minimal, canonical DFA** that recognizes the same language.  

---

### **Types of states that can be removed or merged:**

1. **Unreachable states:**  
   - States that cannot be reached from the **initial DFA state** for any input string.  
   - These states can be safely removed.

2. **Dead states:**  
   - States from which **no accepting state is reachable**.  
   - These can be removed unless a **complete DFA** is required (in which case a single “sink” state may be used).

3. **Nondistinguishable states:**  
   - States that **cannot be distinguished** by any input string.  
   - These states can be **merged** into one state.

---

### **Minimization process (common approach):**

1. **Remove dead and unreachable states**  
   - Speeds up the following steps.

2. **Merge nondistinguishable states**  
   - Typically done using **partitioning or equivalence classes** (Hopcroft’s algorithm is the most efficient).

3. **Optional:**  
   - If a **complete DFA** is required, re-create a **single dead “sink” state** to absorb undefined transitions.

---

**Input:** DFA  
**Output:** Minimized DFA



## 8. Automata Visualization
### **Visualizations done by Python are below these.**
### Since Graphs didn't work out on large scale, the terminal also gives outputs in the following way:


Checking on check on a(b|c)* and (1*01*01*)*(0*10*10*)*


<img width="1001" height="144" alt="image" src="https://github.com/user-attachments/assets/231fa3ec-cd70-431c-864f-7af77429a63e" />

<img width="858" height="807" alt="image" src="https://github.com/user-attachments/assets/a0f26d34-5bf9-4394-b32d-96b45cbb6940" />

<img width="1014" height="820" alt="image" src="https://github.com/user-attachments/assets/eb08e60f-87cf-4e80-9bac-0ae9d93f8ec4" />
etc.

<img width="998" height="846" alt="image" src="https://github.com/user-attachments/assets/dbdc473c-1bc5-4892-b011-ee87aaf6b3fc" />
etc.

<img width="1026" height="842" alt="image" src="https://github.com/user-attachments/assets/147259ef-9894-44c1-a8ab-e7b984aa06df" />

<img width="523" height="425" alt="image" src="https://github.com/user-attachments/assets/eea9ea0f-04ce-4c77-90b1-6b76961c0dc2" />


### Syntax Tree for RegEx
<img width="3570" height="1977" alt="syntax_tree" src="https://github.com/user-attachments/assets/4615ad7b-5ef0-48d4-8d47-43a11fbea572" />


<img width="3570" height="1238" alt="syntax_tree" src="https://github.com/user-attachments/assets/ab8d49ba-9984-4762-a734-1b4f7497c679" />


### ε-NFA
<img width="2646" height="2973" alt="nfa" src="https://github.com/user-attachments/assets/1fa6cf0d-262d-4ebf-9080-6570e73b6942" />

<img width="2832" height="2973" alt="nfa" src="https://github.com/user-attachments/assets/e10ffb26-5a30-4ec4-87bf-7ec87af8d6b6" />


### DFA
<img width="2864" height="2973" alt="dfa" src="https://github.com/user-attachments/assets/81d9e51b-e9b3-435c-a9cc-660c0e7736cb" />

<img width="2768" height="2973" alt="dfa" src="https://github.com/user-attachments/assets/ef7e47ec-bbce-42c7-881c-0bcc6aaf8e01" />


### Minimized DFA
<img width="2957" height="2973" alt="min_dfa" src="https://github.com/user-attachments/assets/362f4e85-0dd5-4de3-80a1-182f542f214b" />


<img width="2747" height="2973" alt="min_dfa" src="https://github.com/user-attachments/assets/03ee4fa1-b456-45c5-aaec-c0bfa60dff46" />



---

## 9. How to Run
First you need to have cmake installed.
In Command Prompt:
- Go to your project directory
  - `cd "your_project_directory"`
- Create a `build` folder and enter it
  - `mkdir build`
  - `cd build`
- Run CMake to generate build files
  - `cmake ..`
- Build the project
  - `cmake --build . --config Release`
- Run the executable
  - For Windows (Visual Studio multi-config):
  - `.\Release\MyApp.exe`
  - For Linux/Mac or single-config generators:
  - `./MyApp`

Run the Python Visualizer
- Go to the visualization folder
  - `cd "visualization_folder"`
- Run the Python script
  - `py visualize_all.py` or `python visualize_all.py`

