#pragma once

#include "types.hpp"
#include <cmath>
#include <numeric>
#include <iomanip>

namespace ecommerce {

// =============================================================================
// FORMAL VECTOR EMBEDDING ENGINE
// Section 6: Embedding Design Problem
//
// Key Formal Properties Investigated:
// 1. Capability Identity: Distinct vectors for distinct capabilities
// 2. Precondition-Effect Compatibility: E_i => P_j
// 3. Input-Output Compatibility: O_i -> I_j
// 4. Similarity vs Composability: High similarity != Composable!
// 5. Capability Composition: C_12 = C_2 o C_1
// =============================================================================

class VectorEmbeddingEngine {
public:
    std::unordered_map<std::string, size_t> varIndex;
    std::vector<std::string> indexVar;

    std::unordered_map<std::string, size_t> typeIndex;
    std::vector<std::string> indexType;

    size_t registerVariable(const std::string& varName) {
        auto it = varIndex.find(varName);
        if (it != varIndex.end()) return it->second;
        size_t idx = indexVar.size();
        varIndex[varName] = idx;
        indexVar.push_back(varName);
        return idx;
    }

    size_t registerType(const std::string& tName) {
        auto it = typeIndex.find(tName);
        if (it != typeIndex.end()) return it->second;
        size_t idx = indexType.size();
        typeIndex[tName] = idx;
        indexType.push_back(tName);
        return idx;
    }

    [[nodiscard]] size_t stateDimension() const {
        return indexVar.size();
    }

    void initialize(const ApplicationProblem& app) {
        for (const auto& kv : app.initialState.vars) registerVariable(kv.first);
        for (const auto& cond : app.goal.conditions) registerVariable(cond.variable);
        for (const auto& cap : app.capabilities) {
            registerType(cap.type);
            for (const auto& pre : cap.preconditions) registerVariable(pre.variable);
            for (const auto& eff : cap.effects) registerVariable(eff.variable);
        }
    }

    // -------------------------------------------------------------------------
    // Deliverable 2: encode(state) -> phi_S(S) in R^{d_s}
    // -------------------------------------------------------------------------
    [[nodiscard]] std::vector<double> encode(const State& s) const {
        return encodeState(s);
    }

    [[nodiscard]] std::vector<double> encodeState(const State& s) const {
        std::vector<double> vec(stateDimension(), 0.0);
        for (size_t i = 0; i < stateDimension(); ++i) {
            vec[i] = s.getOr(indexVar[i], Value(0.0)).toNumeric();
        }
        return vec;
    }

    // -------------------------------------------------------------------------
    // Deliverable 2: encode(goal) -> phi_G(G) in R^{d_g}
    // -------------------------------------------------------------------------
    [[nodiscard]] std::vector<double> encode(const Goal& g) const {
        return encodeGoal(g);
    }

    [[nodiscard]] std::vector<double> encodeGoal(const Goal& g) const {
        std::vector<double> vec(stateDimension(), 0.0);
        for (const auto& cond : g.conditions) {
            auto it = varIndex.find(cond.variable);
            if (it != varIndex.end()) {
                vec[it->second] = cond.expectedValue.toNumeric();
            }
        }
        return vec;
    }

    // Encodes Preconditions into R^d
    [[nodiscard]] std::vector<double> encodePreconditions(const Capability& c) const {
        std::vector<double> vec(stateDimension(), 0.0);
        for (const auto& pre : c.preconditions) {
            auto it = varIndex.find(pre.variable);
            if (it != varIndex.end()) {
                vec[it->second] = pre.expectedValue.toNumeric();
            }
        }
        return vec;
    }

    // Encodes Effects into R^d
    [[nodiscard]] std::vector<double> encodeEffects(const Capability& c) const {
        std::vector<double> vec(stateDimension(), 0.0);
        for (const auto& eff : c.effects) {
            auto it = varIndex.find(eff.variable);
            if (it != varIndex.end()) {
                vec[it->second] = eff.value.toNumeric();
            }
        }
        return vec;
    }

    // -------------------------------------------------------------------------
    // Deliverable 2: encode(capability) -> phi_C(C) in R^{d_c}
    // -------------------------------------------------------------------------
    [[nodiscard]] std::vector<double> encode(const Capability& c) const {
        return encodeCapability(c);
    }

    // phi_C: C -> R^{d_c}
    // Encodes [Preconditions, Effects, One-Hot Type, QoS (latency, cost, risk, reliability)]
    [[nodiscard]] std::vector<double> encodeCapability(const Capability& c) const {
        std::vector<double> fullVec;
        auto pre = encodePreconditions(c);
        auto eff = encodeEffects(c);
        fullVec.insert(fullVec.end(), pre.begin(), pre.end());
        fullVec.insert(fullVec.end(), eff.begin(), eff.end());

        // Type encoding
        std::vector<double> typeVec(indexType.size(), 0.0);
        auto tit = typeIndex.find(c.type);
        if (tit != typeIndex.end()) typeVec[tit->second] = 1.0;
        fullVec.insert(fullVec.end(), typeVec.begin(), typeVec.end());

        // Operational attributes (Q_i, Rel_i, A_i)
        fullVec.push_back(c.qos.timeCostMs / 200.0);
        fullVec.push_back(c.qos.moneyCost * 10.0);
        fullVec.push_back(c.qos.risk);
        fullVec.push_back(c.reliability);
        fullVec.push_back(c.availability);

        return fullVec;
    }

    // -------------------------------------------------------------------------
    // Deliverable 2: similarity(x, y) -> Cosine similarity in R^d
    // -------------------------------------------------------------------------
    static double similarity(const std::vector<double>& v1, const std::vector<double>& v2) {
        return cosineSimilarity(v1, v2);
    }

    // Cosine similarity between two vectors
    static double cosineSimilarity(const std::vector<double>& v1, const std::vector<double>& v2) {
        if (v1.size() != v2.size() || v1.empty()) return 0.0;
        double dot = 0.0, n1 = 0.0, n2 = 0.0;
        for (size_t i = 0; i < v1.size(); ++i) {
            dot += v1[i] * v2[i];
            n1 += v1[i] * v1[i];
            n2 += v2[i] * v2[i];
        }
        if (n1 < 1e-9 || n2 < 1e-9) return 0.0;
        return dot / (std::sqrt(n1) * std::sqrt(n2));
    }

    // Precondition-Effect Compatibility: Does C_1 enable C_2? (E_1 => P_2)
    [[nodiscard]] double calculateCompatibility(const Capability& c1, const Capability& c2) const {
        if (c2.preconditions.empty()) return 1.0;

        double score = 0.0;
        for (const auto& p : c2.preconditions) {
            bool foundInEffect = false;
            for (const auto& e : c1.effects) {
                if (e.variable == p.variable) {
                    foundInEffect = true;
                    if (e.value == p.expectedValue) {
                        score += 1.0; // Directly satisfies precondition!
                    } else {
                        score -= 1.0; // Directly conflicts with precondition!
                    }
                    break;
                }
            }
            if (!foundInEffect) {
                // Neutral: C1 does not alter this precondition variable
            }
        }
        return score / static_cast<double>(c2.preconditions.size());
    }

    // Input-Output Compatibility: Does C_1 provide outputs required by C_2's inputs?
    [[nodiscard]] double calculateInputOutputCompatibility(const Capability& c1, const Capability& c2) const {
        if (c2.inputs.empty()) return 1.0;
        double matched = 0.0;
        for (const auto& in : c2.inputs) {
            for (const auto& out : c1.outputs) {
                if (in.name == out.name && in.type == out.type) {
                    matched += 1.0;
                    break;
                }
            }
        }
        return matched / static_cast<double>(c2.inputs.size());
    }

    // -------------------------------------------------------------------------
    // Deliverable 2: compose(c1, c2) -> C_12 = C_2 o C_1 (Section 5)
    // -------------------------------------------------------------------------
    [[nodiscard]] Capability compose(const Capability& c1, const Capability& c2) const {
        return composeCapabilities(c1, c2);
    }

    // Capability Composition: C_12 = C_2 o C_1 (Section 5)
    // Constructs composite capability CompletePurchase = MakePayment o CreateOrder
    [[nodiscard]] Capability composeCapabilities(const Capability& c1, const Capability& c2) const {
        Capability comp;
        comp.id = c1.id + "_THEN_" + c2.id;
        comp.name = "Composite: (" + c2.name + " o " + c1.name + ")";
        comp.type = "COMPOSITE_SERVICE";
        comp.description = "Composed workflow: " + c1.id + " followed by " + c2.id;

        // Composed Preconditions
        comp.preconditions = c1.preconditions;
        for (const auto& p2 : c2.preconditions) {
            bool satisfied = false;
            for (const auto& e1 : c1.effects) {
                if (e1.variable == p2.variable && e1.value == p2.expectedValue) {
                    satisfied = true;
                    break;
                }
            }
            if (!satisfied) comp.preconditions.push_back(p2);
        }

        // Composed Effects
        comp.effects = c1.effects;
        for (const auto& e2 : c2.effects) {
            bool overwritten = false;
            for (auto& ce : comp.effects) {
                if (ce.variable == e2.variable) {
                    ce = e2;
                    overwritten = true;
                    break;
                }
            }
            if (!overwritten) comp.effects.push_back(e2);
        }

        // Composed Inputs/Outputs
        comp.inputs = c1.inputs;
        comp.outputs = c2.outputs;

        // Composed Quality Attributes
        comp.qos.timeCostMs = c1.qos.timeCostMs + c2.qos.timeCostMs;
        comp.qos.moneyCost = c1.qos.moneyCost + c2.qos.moneyCost;
        comp.qos.resourceCost = std::max(c1.qos.resourceCost, c2.qos.resourceCost);
        comp.qos.risk = 1.0 - ((1.0 - c1.qos.risk) * (1.0 - c2.qos.risk));
        comp.reliability = c1.reliability * c2.reliability;
        comp.availability = c1.availability * c2.availability;

        return comp;
    }
};

} // namespace ecommerce
