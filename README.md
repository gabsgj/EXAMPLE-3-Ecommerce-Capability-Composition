# EXAMPLE 3: E-Commerce Capability Composition

**Formal Capability Embeddings, Compatibility Algebra, and Multi-Criteria Planning**  
*Enterprise Microservice & Service Pipeline Architecture*

---

## 1. Overview and Problem Definition

This project implements an enterprise e-commerce pipeline and provides formal evaluation of capability compatibility, algebraic composition, and multi-criteria planning.

The application models an **E-Commerce Purchase Order & Fulfillment Pipeline**. The system transitions through customer authentication, shopping cart reservation, inventory validation, multi-provider payment processing, digital invoicing, warehouse shipment dispatch, and customer notifications.

> **CRITICAL CONCEPT: CAPABILITIES ARE TRANSITIONS (NOT STATES)**
> - A **State** describes the system variables at a snapshot in time:  
>   $$S = \{(\code{Cart.exists}, \true), (\code{Order.exists}, \false), (\code{Payment.status}, \code{NOT\_STARTED}), \dots\}$$
> - A **Capability** is a **Transition / Service Operation** ($S \xrightarrow{C_i} S'$):  
>   $$\code{CreateOrder}: S \longrightarrow S' = \operatorname{Apply}(S, E_{\code{CreateOrder}})$$
>   It requires preconditions ($P_i$) and produces state effects ($E_i$).

---

## 2. Mathematical Formalization

### 2.1 Formal Application Model
$$\mathcal{A} = (\mathcal{S}, \mathcal{C}, S_I, G, \mathcal{R}, \mathcal{K})$$
- $\mathcal{S}$: State space of e-commerce entity variables.
- $\mathcal{C}$: Set of 10 available capabilities (API, Database, Event, Service, Function).
- $S_I$: Initial state: Customer authenticated, cart with 3 items, no order yet.
- $G$: Goal specification: Order exists, payment successful, invoice generated, shipment dispatched, notification sent.
- $\mathcal{R}$: Computational resources (Payment gateway, Database connection, EventBus).
- $\mathcal{K}$: Global safety constraints (e.g., payment failure hazard).

### 2.2 Formal Capability Representation
Each capability is modeled as an 11-tuple:
$$C_i = (T_i, I_i, O_i, P_i, E_i, K_i, R_i, Q_i, \operatorname{Rel}_i, A_i, M_i)$$
- $T_i \in \{\code{API}, \code{DATABASE}, \code{EVENT}, \code{FUNCTION}, \code{SERVICE}\}$
- $I_i, O_i$: Typed input and output ports (e.g. `order_id: UUID`, `token: string`)
- $P_i, E_i$: Preconditions and Effects
- $Q_i = (C_{\text{time}}, C_{\text{money}}, C_{\text{resource}}, C_{\text{risk}})$
- $\operatorname{Rel}_i \in [0, 1]$: Reliability
- $A_i \in \{0, 1\}$: Availability
- $M_i$: Execution mechanism details (REST endpoints, SQL queries)

### 2.3 Vector Embedding
Unified embedding vector $\phi_C(C_i) \in \mathbb{R}^{d_c}$:
$$\mathbf{v}(C_i) = \left[ \mathbf{v}_{\text{pre}}, \mathbf{v}_{\text{eff}}, \mathbf{v}_{\text{type}}, \mathbf{v}_{\text{qos}} \right]$$

---

## 3. The 5 Formal Capability Experiments

This project contains automated evaluators for all 5 formal experiments:

1. **Experiment 1: Capability Compatibility (Section 7.1)**  
   Demonstrates that $C_1$ (`CreateOrder`) is compatible with $C_2$ (`ValidateInventory`, $+1.00$), but incompatible with $C_3$ (`CancelCart`, $-1.00$).
2. **Experiment 2: Capability Composition (Section 7.2)**  
   Constructs the 3-step composite capability:
   $$\code{CompletePurchaseAndInvoice} = \code{GenerateInvoice} \circ \code{MakePayment} \circ \code{CreateOrder}$$
3. **Experiment 3: Alternative Implementations (Section 7.3)**  
   Compares `MakePayment_API` (API, latency 120ms, cost \$0.05) against `MakePayment_DB` (Database, latency 35ms, cost \$0.00). Shows cosine similarity $\approx 0.93$ (functional resemblance) while preserving distinct operational dimensions.
4. **Experiment 4: Irrelevant Capabilities (Section 7.4)**  
   Evaluates `BrowseCatalog` and `UpdateWishlist` against the goal vector, verifying they are pruned.
5. **Experiment 5: Operational Attributes (Section 7.5)**  
   Multi-criteria Pareto evaluation table over latency, monetary cost, reliability, and risk.

---

## 4. Modular Directory Architecture

```
EXAMPLE 3 - Ecommerce Capability Composition/
├── include/
│   ├── types.hpp            <- Formal State, Goal, and 11-tuple Capability definitions
│   ├── embedding.hpp        <- Vector embedding engine, compatibility, & composition
│   ├── experiments.hpp      <- Automated suite for all 5 empirical experiments
│   ├── planner.hpp          <- A* Goal-directed workflow planner
│   └── engine.hpp           <- JSON problem loader & execution artifact exporter
├── src/
│   └── main.cpp             <- CLI interface, experimental runner, & plan visualizer
├── third_party/
│   └── nlohmann/json.hpp    <- Vendored single-header JSON library
├── ecommerce_purchase_flow.json       <- Purchase order fulfillment pipeline
├── ecommerce_refund_return_flow.json  <- Reverse logistics RMA return pipeline
├── ecommerce_b2b_wholesale_order.json <- Enterprise B2B wholesale procurement pipeline
├── Makefile                           <- Self-contained C++17 build script
├── README.md                          <- This documentation
├── USAGE.md                           <- Step-by-step usage guide and commands
├── DELIVERABLE_1_FORMAL_EMBEDDING_DESIGN.md
├── DELIVERABLE_2_IMPLEMENTATION.md
├── DELIVERABLE_3_EXPERIMENTAL_DATASET.md
└── DELIVERABLE_4_TECHNICAL_REPORT.md  <- Complete technical report with 5 experiments
```

---

## 5. Quick Start

```bash
# Compile
make

# Run the complete experimental suite and synthesized workflow
make run
```
