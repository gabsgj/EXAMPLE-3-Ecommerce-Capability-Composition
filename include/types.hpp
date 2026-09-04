#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <variant>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include "third_party/nlohmann/json.hpp"

namespace ecommerce {

using json = nlohmann::json;

// =============================================================================
// FORMAL APPLICATION & CAPABILITY MODEL
// Section 3: Formal Application Model A = (S, C, S_I, G, R, K)
// Section 4: Formal Capability Model C_i = (T_i, I_i, O_i, P_i, E_i, K_i, R_i, Q_i, Rel_i, A_i, M_i)
//
// Running Example from Assignment Document:
// E-Commerce Order Fulfillment, Payment, Inventory, and Notification
// =============================================================================

// Universal Value representation for State Variables
struct Value {
    using VariantType = std::variant<std::monostate, bool, int64_t, double, std::string>;
    VariantType data;

    Value() : data(std::monostate{}) {}
    Value(bool b) : data(b) {}
    Value(int64_t i) : data(i) {}
    Value(int i) : data(static_cast<int64_t>(i)) {}
    Value(double d) : data(d) {}
    Value(const std::string& s) : data(s) {}
    Value(const char* s) : data(std::string(s)) {}

    [[nodiscard]] double toNumeric() const {
        if (std::holds_alternative<bool>(data)) return std::get<bool>(data) ? 1.0 : 0.0;
        if (std::holds_alternative<int64_t>(data)) return static_cast<double>(std::get<int64_t>(data));
        if (std::holds_alternative<double>(data)) return std::get<double>(data);
        if (std::holds_alternative<std::string>(data)) {
            const auto& s = std::get<std::string>(data);
            if (s == "true" || s == "TRUE") return 1.0;
            if (s == "false" || s == "FALSE") return 0.0;
            if (s == "CREATED") return 1.0;
            if (s == "VALIDATED") return 2.0;
            if (s == "PAID") return 3.0;
            if (s == "FULFILLED") return 4.0;
            if (s == "SUCCESS") return 1.0;
            if (s == "PENDING") return 0.5;
            if (s == "FAILED") return -1.0;
            try { return std::stod(s); } catch (...) { return 0.5; }
        }
        return 0.0;
    }

    [[nodiscard]] std::string asString() const {
        if (std::holds_alternative<bool>(data)) return std::get<bool>(data) ? "true" : "false";
        if (std::holds_alternative<int64_t>(data)) return std::to_string(std::get<int64_t>(data));
        if (std::holds_alternative<double>(data)) {
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(2) << std::get<double>(data);
            return ss.str();
        }
        if (std::holds_alternative<std::string>(data)) return std::get<std::string>(data);
        return "null";
    }

    bool operator==(const Value& other) const {
        if (std::holds_alternative<bool>(data) && std::holds_alternative<bool>(other.data)) {
            return std::get<bool>(data) == std::get<bool>(other.data);
        }
        if (std::holds_alternative<std::string>(data) && std::holds_alternative<std::string>(other.data)) {
            return std::get<std::string>(data) == std::get<std::string>(other.data);
        }
        return std::abs(toNumeric() - other.toNumeric()) < 1e-6;
    }
};

// Application State: S = {(x_1, v_1), ..., (x_n, v_n)}
struct State {
    std::unordered_map<std::string, Value> vars;

    [[nodiscard]] bool has(const std::string& key) const {
        return vars.find(key) != vars.end();
    }

    [[nodiscard]] Value getOr(const std::string& key, const Value& def) const {
        auto it = vars.find(key);
        return (it != vars.end()) ? it->second : def;
    }

    void set(const std::string& key, const Value& val) {
        vars[key] = val;
    }

    [[nodiscard]] std::string signature() const {
        std::map<std::string, std::string> sortedVars;
        for (const auto& [k, v] : vars) {
            sortedVars[k] = v.asString();
        }
        std::string sig;
        for (const auto& [k, v] : sortedVars) {
            sig += k + "=" + v + ";";
        }
        return sig;
    }
};

// Input / Output Parameter Specification (Section 4.2)
// i = (name, type, domain, required), o = (name, type, domain)
struct Port {
    std::string name;
    std::string type;   // e.g. "UUID", "double", "string"
    std::string domain; // e.g. "valid UUIDs", "non-negative currency"
    bool required = true;
};

// Condition / Predicate on a state variable (Section 4.3)
struct Condition {
    std::string variable;
    std::string op; // "==", "!=", ">", "<", ">=", "<="
    Value expectedValue;

    [[nodiscard]] bool evaluate(const State& s) const {
        if (!s.has(variable)) return false;
        Value cur = s.getOr(variable, Value{});
        if (op == "==") return cur == expectedValue;
        if (op == "!=") return !(cur == expectedValue);
        if (op == ">")  return cur.toNumeric() > expectedValue.toNumeric();
        if (op == "<")  return cur.toNumeric() < expectedValue.toNumeric();
        if (op == ">=") return cur.toNumeric() >= expectedValue.toNumeric();
        if (op == "<=") return cur.toNumeric() <= expectedValue.toNumeric();
        return false;
    }
};

// Effect of a Capability Transition: Apply(S, E_i) (Section 4.3)
struct Effect {
    std::string variable;
    std::string op; // "SET", "INCREMENT", "DECREMENT"
    Value value;

    void apply(State& s) const {
        if (op == "SET") {
            s.set(variable, value);
        } else if (op == "INCREMENT") {
            double cur = s.getOr(variable, Value(0.0)).toNumeric();
            s.set(variable, Value(cur + value.toNumeric()));
        } else if (op == "DECREMENT") {
            double cur = s.getOr(variable, Value(0.0)).toNumeric();
            s.set(variable, Value(cur - value.toNumeric()));
        }
    }
};

// Goal Specification: G = {g_1, g_2, ..., g_m} (Section 3.2)
struct Goal {
    std::vector<Condition> conditions;

    [[nodiscard]] bool isSatisfied(const State& s) const {
        for (const auto& cond : conditions) {
            if (!cond.evaluate(s)) return false;
        }
        return true;
    }
};

// Operational Quality Attributes Q_i = (C_time, C_resource, C_money, C_risk, C_energy) (Section 4.5)
struct QualityAttributes {
    double timeCostMs = 100.0;    // Latency in ms
    double moneyCost = 0.01;      // Transaction cost in USD
    double resourceCost = 1.0;    // CPU / Memory compute units
    double risk = 0.05;           // Risk factor in [0, 1]

    [[nodiscard]] double totalNormalizedCost() const {
        return (timeCostMs / 100.0) + (moneyCost * 10.0) + resourceCost + (risk * 5.0);
    }
};

// Formal Capability Model (11-tuple) (Section 4)
// C_i = (T_i, I_i, O_i, P_i, E_i, K_i, R_i, Q_i, Rel_i, A_i, M_i)
// NOTE: CAPABILITIES ARE TRANSITIONS (NOT STATES)!
struct Capability {
    std::string id;
    std::string name;
    std::string description;

    // 11-Tuple Components:
    std::string type = "API";                      // T_i in {API, DATABASE, GUI, EVENT, FUNCTION, SERVICE}
    std::vector<Port> inputs;                      // I_i
    std::vector<Port> outputs;                     // O_i
    std::vector<Condition> preconditions;          // P_i
    std::vector<Effect> effects;                   // E_i
    std::vector<Condition> constraints;            // K_i
    std::vector<std::string> resources;            // R_i
    QualityAttributes qos;                         // Q_i
    double reliability = 0.99;                     // Rel_i in [0, 1]
    double availability = 1.0;                     // A_i in {0, 1}
    std::string mechanism = "HTTP_POST";           // M_i (e.g. endpoint, SQL query)

    [[nodiscard]] bool isApplicable(const State& s) const {
        for (const auto& pre : preconditions) {
            if (!pre.evaluate(s)) return false;
        }
        for (const auto& cons : constraints) {
            if (!cons.evaluate(s)) return false;
        }
        return true;
    }

    [[nodiscard]] State apply(const State& s) const {
        State nextState = s;
        for (const auto& eff : effects) {
            eff.apply(nextState);
        }
        return nextState;
    }
};

// Formal Application Model: A = (S, C, S_I, G, R, K) (Section 3)
struct ApplicationProblem {
    std::string problemName;
    std::string domain;
    State initialState;                            // S_I
    Goal goal;                                     // G
    std::vector<Capability> capabilities;          // C
    std::vector<std::string> availableResources;   // R
    std::vector<Condition> globalConstraints;      // K
};

} // namespace ecommerce
