# Usage Guide: EXAMPLE 3 - E-Commerce Capability Composition

This guide details how to build, run, and execute experiments on the E-Commerce Capability Composer.

---

## 1. Building the Program

```bash
cd "EXAMPLE 3 - Ecommerce Capability Composition"
make
```

To clean generated binaries and logs:
```bash
make clean
```

---

## 2. Command Line Interface

```text
Usage:
  ./ecommerce_composer <problem_spec.json> [options]

Options:
  --experiments     Run all 5 formal empirical experiments
  --vectors         Display vector embeddings for states, goals, & capabilities
  --compatibility   Display precondition-effect compatibility matrix
  --compose         Demonstrate formal capability composition (C_2 o C_1)
  --help            Show this help message
```

---

## 3. Running Pre-Packaged Experiments

### Run All 5 Empirical Experiments and Full Workflow Plan
```bash
./ecommerce_composer ecommerce_purchase_flow.json --experiments
```

### Inspect Vectors and Compatibility Matrix
```bash
./ecommerce_composer ecommerce_purchase_flow.json --vectors --compatibility --compose
```

### Run RMA Customer Return & Refund Pipeline
```bash
./ecommerce_composer ecommerce_refund_return_flow.json --vectors
```

### Run Enterprise B2B Wholesale Procurement Order
```bash
./ecommerce_composer ecommerce_b2b_wholesale_order.json --compatibility
```

---

## 4. Generated Artifacts

1. `workflow_execution.txt`: Formatted log showing state snapshots and transition steps through order completion.

