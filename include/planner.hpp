#pragma once

#include "types.hpp"
#include "embedding.hpp"
#include <queue>
#include <chrono>

namespace ecommerce {

struct PlanStep {
    Capability capability;
    State stateBefore;
    State stateAfter;
};

struct PlanNode {
    State state;
    std::vector<std::string> path;
    double gCost = 0.0;
    double hCost = 0.0;

    [[nodiscard]] double fCost() const { return gCost + hCost; }

    bool operator>(const PlanNode& other) const {
        return fCost() > other.fCost();
    }
};

struct PlanResult {
    bool success = false;
    std::vector<Capability> plannedCapabilities;
    State finalState;
    double totalTimeMs = 0.0;
    double totalMoneyCost = 0.0;
    double combinedReliability = 1.0;
    double planningLatencyMicroseconds = 0.0;
    size_t nodesExpanded = 0;
};

class WorkflowPlanner {
private:
    const ApplicationProblem& app;
    const VectorEmbeddingEngine& embedding;

    [[nodiscard]] double heuristic(const State& s) const {
        auto sVec = embedding.encodeState(s);
        auto gVec = embedding.encodeGoal(app.goal);
        double dist = 0.0;
        for (size_t i = 0; i < sVec.size(); ++i) {
            if (gVec[i] != 0.0 && sVec[i] != gVec[i]) {
                dist += 1.0;
            }
        }
        return dist;
    }

    [[nodiscard]] bool violatesHazards(const State& s) const {
        for (const auto& hazard : app.globalConstraints) {
            if (hazard.evaluate(s)) return true;
        }
        return false;
    }

public:
    WorkflowPlanner(const ApplicationProblem& problem, const VectorEmbeddingEngine& emb)
        : app(problem), embedding(emb) {}

    PlanResult findPlan() {
        auto startTime = std::chrono::high_resolution_clock::now();
        PlanResult result;

        if (violatesHazards(app.initialState)) return result;
        if (app.goal.isSatisfied(app.initialState)) {
            result.success = true;
            result.finalState = app.initialState;
            return result;
        }

        std::priority_queue<PlanNode, std::vector<PlanNode>, std::greater<PlanNode>> openSet;
        openSet.push({app.initialState, {}, 0.0, heuristic(app.initialState)});

        std::unordered_map<std::string, double> bestG;
        bestG[app.initialState.signature()] = 0.0;

        std::unordered_map<std::string, Capability> capMap;
        for (const auto& c : app.capabilities) capMap[c.id] = c;

        while (!openSet.empty()) {
            PlanNode cur = openSet.top();
            openSet.pop();
            result.nodesExpanded++;

            if (app.goal.isSatisfied(cur.state)) {
                auto endTime = std::chrono::high_resolution_clock::now();
                result.success = true;
                result.finalState = cur.state;
                result.planningLatencyMicroseconds = std::chrono::duration<double, std::micro>(endTime - startTime).count();

                for (const auto& cid : cur.path) {
                    const auto& c = capMap[cid];
                    result.plannedCapabilities.push_back(c);
                    result.totalTimeMs += c.qos.timeCostMs;
                    result.totalMoneyCost += c.qos.moneyCost;
                    result.combinedReliability *= c.reliability;
                }
                return result;
            }

            for (const auto& cap : app.capabilities) {
                if (!cap.isApplicable(cur.state)) continue;

                State nextState = cap.apply(cur.state);
                if (violatesHazards(nextState)) continue;

                double nextG = cur.gCost + cap.qos.totalNormalizedCost();
                std::string nextSig = nextState.signature();
                auto it = bestG.find(nextSig);
                if (it != bestG.end() && it->second <= nextG + 1e-9) {
                    continue;
                }
                bestG[nextSig] = nextG;

                PlanNode nextNode;
                nextNode.state = nextState;
                nextNode.path = cur.path;
                nextNode.path.push_back(cap.id);
                nextNode.gCost = nextG;
                nextNode.hCost = heuristic(nextState);

                openSet.push(nextNode);
            }

            if (result.nodesExpanded > 5000) break;
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        result.planningLatencyMicroseconds = std::chrono::duration<double, std::micro>(endTime - startTime).count();
        return result;
    }
};

} // namespace ecommerce
