#pragma once

#include "types.hpp"
#include "embedding.hpp"
#include "planner.hpp"
#include <fstream>

namespace ecommerce {

class EcommerceEngine {
public:
    static ApplicationProblem loadFromJson(const std::string& filePath) {
        std::ifstream f(filePath);
        if (!f.is_open()) {
            throw std::runtime_error("Cannot open problem specification: " + filePath);
        }
        json j;
        f >> j;

        ApplicationProblem app;
        app.problemName = j.value("problemName", "Ecommerce_Order_Fulfillment");
        app.domain = j.value("domain", "E-Commerce");

        if (j.contains("initialState") && j["initialState"].is_object()) {
            for (auto& [k, v] : j["initialState"].items()) {
                if (v.is_boolean()) app.initialState.set(k, Value(v.get<bool>()));
                else if (v.is_number_integer()) app.initialState.set(k, Value(v.get<int64_t>()));
                else if (v.is_number_float()) app.initialState.set(k, Value(v.get<double>()));
                else if (v.is_string()) app.initialState.set(k, Value(v.get<std::string>()));
            }
        }

        if (j.contains("goal") && j["goal"].is_array()) {
            for (const auto& gItem : j["goal"]) {
                Condition cond;
                cond.variable = gItem.value("variable", "");
                cond.op = gItem.value("op", "==");
                if (gItem.contains("expectedValue")) {
                    const auto& ev = gItem["expectedValue"];
                    if (ev.is_boolean()) cond.expectedValue = Value(ev.get<bool>());
                    else if (ev.is_number_integer()) cond.expectedValue = Value(ev.get<int64_t>());
                    else if (ev.is_number_float()) cond.expectedValue = Value(ev.get<double>());
                    else if (ev.is_string()) cond.expectedValue = Value(ev.get<std::string>());
                }
                app.goal.conditions.push_back(cond);
            }
        }

        if (j.contains("hazards") && j["hazards"].is_array()) {
            for (const auto& hItem : j["hazards"]) {
                Condition cond;
                cond.variable = hItem.value("variable", "");
                cond.op = hItem.value("op", "==");
                if (hItem.contains("expectedValue")) {
                    const auto& ev = hItem["expectedValue"];
                    if (ev.is_boolean()) cond.expectedValue = Value(ev.get<bool>());
                    else if (ev.is_number_integer()) cond.expectedValue = Value(ev.get<int64_t>());
                    else if (ev.is_string()) cond.expectedValue = Value(ev.get<std::string>());
                }
                app.globalConstraints.push_back(cond);
            }
        }

        if (j.contains("capabilities") && j["capabilities"].is_array()) {
            for (const auto& cItem : j["capabilities"]) {
                Capability cap;
                cap.id = cItem.value("id", "cap_" + std::to_string(app.capabilities.size()));
                cap.name = cItem.value("name", cap.id);
                cap.description = cItem.value("description", "");
                cap.type = cItem.value("type", "API");
                cap.mechanism = cItem.value("mechanism", "HTTP");
                cap.reliability = cItem.value("reliability", 0.99);
                cap.availability = cItem.value("availability", 1.0);

                if (cItem.contains("qos")) {
                    cap.qos.timeCostMs = cItem["qos"].value("timeCostMs", 100.0);
                    cap.qos.moneyCost = cItem["qos"].value("moneyCost", 0.01);
                    cap.qos.resourceCost = cItem["qos"].value("resourceCost", 1.0);
                    cap.qos.risk = cItem["qos"].value("risk", 0.05);
                }

                if (cItem.contains("inputs") && cItem["inputs"].is_array()) {
                    for (const auto& inItem : cItem["inputs"]) {
                        Port p;
                        p.name = inItem.value("name", "");
                        p.type = inItem.value("type", "string");
                        p.domain = inItem.value("domain", "any");
                        p.required = inItem.value("required", true);
                        cap.inputs.push_back(p);
                    }
                }

                if (cItem.contains("outputs") && cItem["outputs"].is_array()) {
                    for (const auto& outItem : cItem["outputs"]) {
                        Port p;
                        p.name = outItem.value("name", "");
                        p.type = outItem.value("type", "string");
                        p.domain = outItem.value("domain", "any");
                        cap.outputs.push_back(p);
                    }
                }

                if (cItem.contains("preconditions") && cItem["preconditions"].is_array()) {
                    for (const auto& pItem : cItem["preconditions"]) {
                        Condition cond;
                        cond.variable = pItem.value("variable", "");
                        cond.op = pItem.value("op", "==");
                        if (pItem.contains("expectedValue")) {
                            const auto& ev = pItem["expectedValue"];
                            if (ev.is_boolean()) cond.expectedValue = Value(ev.get<bool>());
                            else if (ev.is_number_integer()) cond.expectedValue = Value(ev.get<int64_t>());
                            else if (ev.is_number_float()) cond.expectedValue = Value(ev.get<double>());
                            else if (ev.is_string()) cond.expectedValue = Value(ev.get<std::string>());
                        }
                        cap.preconditions.push_back(cond);
                    }
                }

                if (cItem.contains("effects") && cItem["effects"].is_array()) {
                    for (const auto& eItem : cItem["effects"]) {
                        Effect eff;
                        eff.variable = eItem.value("variable", "");
                        eff.op = eItem.value("op", "SET");
                        if (eItem.contains("value")) {
                            const auto& val = eItem["value"];
                            if (val.is_boolean()) eff.value = Value(val.get<bool>());
                            else if (val.is_number_integer()) eff.value = Value(val.get<int64_t>());
                            else if (val.is_number_float()) eff.value = Value(val.get<double>());
                            else if (val.is_string()) eff.value = Value(val.get<std::string>());
                        }
                        cap.effects.push_back(eff);
                    }
                }

                app.capabilities.push_back(cap);
            }
        }

        return app;
    }

    static void exportExecutionText(const std::string& filePath, const ApplicationProblem& app, const PlanResult& plan) {
        std::ofstream out(filePath);
        if (!out.is_open()) return;

        out << "======================================================================\n";
        out << "         E-COMMERCE CAPABILITY WORKFLOW EXECUTION SUMMARY            \n";
        out << "         Problem: " << app.problemName << "\n";
        out << "======================================================================\n\n";

        out << "Initial Application State S_I:\n";
        for (const auto& [k, v] : app.initialState.vars) {
            out << "  - " << std::left << std::setw(24) << k << ": " << v.asString() << "\n";
        }
        out << "\n";

        out << "Target Goal Predicates G:\n";
        for (const auto& g : app.goal.conditions) {
            out << "  - " << g.variable << " " << g.op << " " << g.expectedValue.asString() << "\n";
        }
        out << "\n";

        out << "Synthesized Workflow Execution Metrics:\n";
        out << "  Status:                 " << (plan.success ? "SUCCESS [ORDER FULFILLED]" : "FAILED") << "\n";
        out << "  Steps Executed:         " << plan.plannedCapabilities.size() << " capabilities\n";
        out << "  Cumulative Latency:     " << plan.totalTimeMs << " ms\n";
        out << "  Total Monetary Cost:    $" << std::fixed << std::setprecision(3) << plan.totalMoneyCost << "\n";
        out << "  Combined Reliability:   " << std::setprecision(4) << plan.combinedReliability * 100.0 << "%\n";
        out << "  Planning Latency:       " << plan.planningLatencyMicroseconds << " microseconds\n\n";

        out << "Ordered Sequence of Capabilities (Transitions S_i -> S_{i+1}):\n";
        out << "----------------------------------------------------------------------\n";
        State curState = app.initialState;
        for (size_t i = 0; i < plan.plannedCapabilities.size(); ++i) {
            const auto& cap = plan.plannedCapabilities[i];
            out << "Step " << (i + 1) << ": [" << cap.type << "] " << cap.id << " (" << cap.name << ")\n";
            out << "  Preconditions: ";
            for (const auto& p : cap.preconditions) out << p.variable << p.op << p.expectedValue.asString() << " ";
            out << "\n  Effects:       ";
            for (const auto& e : cap.effects) out << e.variable << "=" << e.value.asString() << " ";
            out << "\n  Latency: " << cap.qos.timeCostMs << "ms | Cost: $" << cap.qos.moneyCost << " | Rel: " << cap.reliability << "\n\n";
            curState = cap.apply(curState);
        }
        out << "======================================================================\n";
    }

    static void exportVisualizerManifest(const std::string& filePath, const ApplicationProblem& app, const PlanResult& plan) {
        json j;
        j["manifestVersion"] = "2.0";
        j["domain"] = "EcommerceWorkflow";
        j["problemName"] = app.problemName;
        j["success"] = plan.success;
        j["planningLatencyMicroseconds"] = plan.planningLatencyMicroseconds;
        j["totalLatencyMs"] = plan.totalTimeMs;
        j["totalCost"] = plan.totalMoneyCost;
        j["combinedReliability"] = plan.combinedReliability;

        json capArr = json::array();
        for (const auto& c : plan.plannedCapabilities) {
            json co;
            co["id"] = c.id;
            co["name"] = c.name;
            co["type"] = c.type;
            co["timeMs"] = c.qos.timeCostMs;
            co["cost"] = c.qos.moneyCost;
            co["reliability"] = c.reliability;
            capArr.push_back(co);
        }
        j["executedPipeline"] = capArr;

        std::ofstream out(filePath);
        if (out.is_open()) {
            out << j.dump(2) << "\n";
        }
    }
};

} // namespace ecommerce
