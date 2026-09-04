# Deliverable 2: Working Implementation
## EXAMPLE 3: E-Commerce Capability Composition

**Implementation Domain**: Enterprise Microservice & Service Pipeline Architecture  
**Directory**: `EXAMPLE 3 - Ecommerce Capability Composition/`

---

## 1. System Architecture and File Structure

The E-Commerce Capability Composition engine is engineered in modern C++17 with an object-oriented and functional hybrid architecture:

```
EXAMPLE 3 - Ecommerce Capability Composition/
├── include/
│   ├── types.hpp               # Universal Value, State, Goal, Port, Capability 11-tuple types
│   ├── embedding.hpp           # 5 Standard vector embedding methods & compatibility operators
│   ├── planner.hpp             # A* multi-criteria workflow planner with bestG closed set memoization
│   ├── experiments.hpp         # Automated harness executing the 5 formal empirical experiments
│   ├── engine.hpp              # JSON dataset loader, workflow execution simulator, & artifact exporter
│   └── json.hpp                # Single-header embedded JSON parser (nlohmann::json)
├── src/
│   └── main.cpp                # CLI entry point, argument parsing, experiment dispatcher
├── ecommerce_purchase_flow.json       # Benchmark 1: Canonical checkout & fulfillment pipeline
├── ecommerce_refund_return_flow.json  # Benchmark 2: Reverse logistics RMA return pipeline
├── ecommerce_b2b_wholesale_order.json # Benchmark 3: Enterprise B2B contract & bulk freight pipeline
├── workflow_execution.txt             # Auto-generated step-by-step workflow execution trace
├── Makefile                           # Standalone compilation script
├── README.md                          # Architecture guide & executive overview
├── USAGE.md                           # CLI syntax and experiment reproduction manual
├── DELIVERABLE_1_FORMAL_EMBEDDING_DESIGN.md
├── DELIVERABLE_2_IMPLEMENTATION.md
├── DELIVERABLE_3_EXPERIMENTAL_DATASET.md
└── DELIVERABLE_4_TECHNICAL_REPORT.md
```

### Dependencies
- **Standard**: C++17 (`-std=c++17`)
- **Compilers**: Clang++ (Apple Clang $\ge 12.0$ or LLVM Clang $\ge 10.0$), G++ ($\ge 9.0$)
- **Runtime Dependencies**: Zero external dependencies. Uses standard C++ `<vector>`, `<string>`, `<unordered_map>`, `<queue>`, `<cmath>`, `<iostream>`, `<fstream>`, `<iomanip>`.

---

## 2. The 5 Core Standard Vector Embedding Methods

All mathematical projections and vector operations are defined within [`include/embedding.hpp`](include/embedding.hpp):

### Method 1: `encode(const State& s)`
```cpp
std::vector<double> encode(const State& s);
```
- **Description**: Projects an enterprise business state into a bipolar vector $\phi_S(S) \in \{-1.0, +1.0\}^{d_s}$.
- **Implementation**:
  ```cpp
  std::vector<double> v(domain_vars.size(), -1.0);
  for (size_t i = 0; i < domain_vars.size(); ++i) {
      if (s.has(domain_vars[i]) && s.get_bool(domain_vars[i])) {
          v[i] = +1.0;
      }
  }
  return v;
  ```

### Method 2: `encode(const Goal& g)`
```cpp
std::vector<double> encode(const Goal& g);
```
- **Description**: Projects target business fulfillment goals into a ternary masked vector $\phi_G(G) \in \{-1.0, 0.0, +1.0\}^{d_s}$. Unspecified variables receive $0.0$, preventing non-goal variables from distorting the distance heuristic.

### Method 3: `encode(const Capability& c)`
```cpp
std::vector<double> encode(const Capability& c);
```
- **Description**: Maps an 11-tuple capability into a partitioned continuous vector space $\phi_C(C) \in \mathbb{R}^{2d_s + d_{\text{type}} + 5}$.
- **Subspace Layout**:
  - `[0, ds - 1]`: Precondition subspace $\mathbf{v}_{\text{pre}} \in \{-1, 0, +1\}^{d_s}$
  - `[ds, 2*ds - 1]`: Effect subspace $\mathbf{v}_{\text{eff}} \in \{-1, 0, +1\}^{d_s}$
  - `[2*ds, 2*ds + dtype - 1]`: Capability type one-hot encoding $\mathbf{v}_{\text{type}}$
  - `[2*ds + dtype]`: $\log_{10}(1 + C_{\text{time}} / 1000)$ (normalized response latency)
  - `[2*ds + dtype + 1]`: $C_{\text{money}}$ (financial cost in USD)
  - `[2*ds + dtype + 2]`: $C_{\text{resource}}$ (compute index)
  - `[2*ds + dtype + 3]`: $C_{\text{risk}}$ (operational risk factor)
  - `[2*ds + dtype + 4]`: $\operatorname{Rel}_i$ (reliability probability)

### Method 4: `compose(const Capability& c1, const Capability& c2)`
```cpp
Capability compose(const Capability& c1, const Capability& c2);
```
- **Description**: Implements sequential algebraic composition $C_{12} = C_2 \circ C_1$.
- **Semantics**:
  - Preconditions: $P(C_{12}) = P(C_1) \cup (P(C_2) \setminus E(C_1))$
  - Effects: $E(C_{12}) = E(C_1) \oplus E(C_2)$
  - Latency and financial costs are additive ($C_{\text{time}} = t_1 + t_2, C_{\text{money}} = m_1 + m_2$)
  - Reliability is multiplicative ($\operatorname{Rel} = r_1 \cdot r_2$)

### Method 5: `similarity(const std::vector<double>& v1, const std::vector<double>& v2)`
```cpp
double similarity(const std::vector<double>& v1, const std::vector<double>& v2);
```
- **Description**: Computes the normalized cosine similarity between two embedding vectors:
  $$\operatorname{Sim}(\mathbf{v}_1, \mathbf{v}_2) = \frac{\langle \mathbf{v}_1, \mathbf{v}_2 \rangle}{\|\mathbf{v}_1\|_2 \cdot \|\mathbf{v}_2\|_2 + \epsilon} \in [-1.0, +1.0]$$

---

## 3. Compatibility Operators (`include/embedding.hpp`)

In addition to vector similarity, the library provides two specialized composability operators:

1. **Precondition-Effect Compatibility**:
   ```cpp
   double compute_compatibility(const Capability& c1, const Capability& c2);
   ```
   Computes:
   $$\operatorname{Comp}(C_1, C_2) = \frac{\langle \mathbf{v}_{\text{eff}}(C_1), \; \mathbf{v}_{\text{pre}}(C_2) \rangle}{\|\mathbf{v}_{\text{pre}}(C_2)\|^2 + \epsilon}$$

2. **Port Compatibility**:
   ```cpp
   double compute_port_compatibility(const Capability& c1, const Capability& c2);
   ```
   Evaluates typed schema matching between output ports $O_1$ and input ports $I_2$.

---

## 4. Multi-Criteria Workflow Planner (`include/planner.hpp`)

The automated workflow engine employs an $A^*$ search algorithm enhanced with state memoization:
- **State Representation**: Bipolar state vector $\phi_S(S)$.
- **Cost Function $g(S)$**: Multi-attribute objective function:
  $$g(S) = w_t \cdot C_{\text{time}} + w_m \cdot C_{\text{money}} + w_r \cdot (1.0 - \operatorname{Rel})$$
- **Heuristic Function $h(S, G)$**: Masked Manhattan distance in vector space:
  $$h(S, G) = \sum_{k=1}^{d_s} |\phi_G(G)_k| \cdot \frac{1}{2} |\phi_G(G)_k - \phi_S(S)_k|$$
- **Closed Set Memoization**: Tracks optimal path costs in `bestG` hash map, pruning suboptimal cycles immediately.

---

## 5. Automated Experiment Harness (`include/experiments.hpp`)

The module [`include/experiments.hpp`](include/experiments.hpp) provides automated test runners executing the five formal empirical experiments:
1. `run_experiment_1()`: Precondition-Effect Compatibility matrix evaluation.
2. `run_experiment_2()`: Sequential 3-step pipeline composition and QoS aggregation.
3. `run_experiment_3()`: Alternative implementation similarity ($\operatorname{Sim}_{\text{func}}$ vs $\operatorname{Sim}_{\text{impl}}$) and similarity $\neq$ composability proof.
4. `run_experiment_4()`: Distractor and irrelevant capability directional cosine filtering.
5. `run_experiment_5()`: Pareto trade-off curve generation across latency, cost, and reliability.

---

## 6. Compilation & CLI Execution

### Build Instructions
```bash
# Clean previous builds
make clean

# Compile release binary with optimization
make
```

### CLI Execution
```bash
# Run the 5 formal empirical experiments
./ecommerce_composer ecommerce_purchase_flow.json --experiments

# Display vector space embeddings
./ecommerce_composer ecommerce_purchase_flow.json --vectors

# Display pairwise compatibility matrix
./ecommerce_composer ecommerce_purchase_flow.json --compatibility

# Plan reverse logistics workflow
./ecommerce_composer ecommerce_refund_return_flow.json

# Plan enterprise B2B wholesale workflow
./ecommerce_composer ecommerce_b2b_wholesale_order.json

# Review execution trace artifact
cat workflow_execution.txt
```
