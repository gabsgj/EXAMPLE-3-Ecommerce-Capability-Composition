#include "include/types.hpp"
#include "include/embedding.hpp"
#include "include/planner.hpp"
#include "include/engine.hpp"
#include "include/experiments.hpp"
#include <iostream>
#include <iomanip>

using namespace ecommerce;

void printHeader() {
    std::cout << "======================================================================\n";
    std::cout << "  EXAMPLE 3: E-COMMERCE CAPABILITY COMPOSITION                       \n";
    std::cout << "  Formal Capability Embeddings, Compatibility, & Workflow Planning   \n";
    std::cout << "======================================================================\n\n";
}

void printUsage(const char* prog) {
    std::cout << "Usage:\n";
    std::cout << "  " << prog << " <problem_spec.json> [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --experiments     Run all 5 formal empirical experiments\n";
    std::cout << "  --vectors         Display vector embeddings for states, goals, & capabilities\n";
    std::cout << "  --compatibility   Display precondition-effect compatibility matrix\n";
    std::cout << "  --compose         Demonstrate formal capability composition (C_2 o C_1)\n";
    std::cout << "  --help            Show this help message\n\n";
}

int main(int argc, char* argv[]) {
    printHeader();

    if (argc < 2) {
        printUsage(argv[0]);
        std::cout << "Defaulting to: ecommerce_purchase_flow.json\n\n";
    }

    std::string jsonPath = (argc >= 2 && argv[1][0] != '-') ? argv[1] : "ecommerce_purchase_flow.json";

    bool runExp = false;
    bool showVectors = false;
    bool showCompat = false;
    bool showCompose = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--experiments") runExp = true;
        if (arg == "--vectors") showVectors = true;
        if (arg == "--compatibility") showCompat = true;
        if (arg == "--compose") showCompose = true;
        if (arg == "--help") {
            printUsage(argv[0]);
            return 0;
        }
    }

    // By default, planning and execution are performed.
    // Experiments are run when --experiments is specified.

    try {
        std::cout << "Loading formal application specification: " << jsonPath << " ...\n";
        ApplicationProblem app = EcommerceEngine::loadFromJson(jsonPath);

        std::cout << "  -> Problem Name:    " << app.problemName << "\n";
        std::cout << "  -> Domain:          " << app.domain << "\n";
        std::cout << "  -> Initial State:   " << app.initialState.vars.size() << " variables in S_I\n";
        std::cout << "  -> Goal Predicates: " << app.goal.conditions.size() << " target goals in G\n";
        std::cout << "  -> Capabilities:    " << app.capabilities.size() << " available transitions in C\n";
        std::cout << "  -> Hazards:         " << app.globalConstraints.size() << " global invariants in K\n\n";

        // Initialize Vector Embedding Engine
        VectorEmbeddingEngine embedding;
        embedding.initialize(app);
        size_t d_s = embedding.stateDimension();
        size_t d_c = d_s * 2 + embedding.indexType.size() + 5;

        std::cout << "[EMBEDDING SPACE DEFINITION]\n";
        std::cout << "  State Space Dimension   d_s = " << d_s << "\n";
        std::cout << "  Goal Space Dimension    d_g = " << d_s << "\n";
        std::cout << "  Capability Dimension    d_c = " << d_c << " [P in R^" << d_s 
                  << ", E in R^" << d_s << ", Type in R^" << embedding.indexType.size() 
                  << ", Q/Rel/Avail in R^5]\n\n";

        // Display Vector Embeddings
        if (showVectors) {
            std::cout << "----------------------------------------------------------------------\n";
            std::cout << "1. FORMAL VECTOR REPRESENTATIONS (phi_S, phi_G, phi_C)\n";
            std::cout << "----------------------------------------------------------------------\n";

            auto s0Vec = embedding.encodeState(app.initialState);
            std::cout << "Initial State Vector phi_S(S_I) in R^" << d_s << ":\n  [";
            for (size_t i = 0; i < s0Vec.size(); ++i) {
                std::cout << std::fixed << std::setprecision(1) << s0Vec[i] << (i + 1 < s0Vec.size() ? ", " : "");
            }
            std::cout << "]\n\n";

            auto gVec = embedding.encodeGoal(app.goal);
            std::cout << "Goal Target Vector phi_G(G) in R^" << d_s << ":\n  [";
            for (size_t i = 0; i < gVec.size(); ++i) {
                std::cout << std::fixed << std::setprecision(1) << gVec[i] << (i + 1 < gVec.size() ? ", " : "");
            }
            std::cout << "]\n\n";

            std::cout << "Sample Capability Vectors phi_C(C_i) in R^" << d_c << ":\n";
            for (size_t k = 0; k < std::min(app.capabilities.size(), size_t(4)); ++k) {
                const auto& c = app.capabilities[k];
                auto cVec = embedding.encodeCapability(c);
                std::cout << "  " << std::left << std::setw(22) << c.id << ": [";
                for (size_t i = 0; i < std::min(cVec.size(), size_t(8)); ++i) {
                    std::cout << std::fixed << std::setprecision(1) << cVec[i] << ", ";
                }
                std::cout << "... (dim=" << cVec.size() << ")]\n";
            }
            std::cout << "\n";
        }

        // Display Compatibility Matrix
        if (showCompat) {
            std::cout << "----------------------------------------------------------------------\n";
            std::cout << "2. PRECONDITION-EFFECT COMPATIBILITY MATRIX (Section 6.1 Property 3)\n";
            std::cout << "----------------------------------------------------------------------\n";
            size_t nShow = std::min(app.capabilities.size(), size_t(6));
            std::cout << std::left << std::setw(20) << "From \\ To";
            for (size_t j = 0; j < nShow; ++j) {
                std::cout << std::setw(14) << app.capabilities[j].id.substr(0, 12);
            }
            std::cout << "\n";
            for (size_t i = 0; i < nShow; ++i) {
                std::cout << std::left << std::setw(20) << app.capabilities[i].id.substr(0, 18);
                for (size_t j = 0; j < nShow; ++j) {
                    double comp = embedding.calculateCompatibility(app.capabilities[i], app.capabilities[j]);
                    std::cout << std::fixed << std::setprecision(2) << std::setw(14) << comp;
                }
                std::cout << "\n";
            }
            std::cout << "\n";
        }

        // Display Capability Composition
        if (showCompose) {
            std::cout << "----------------------------------------------------------------------\n";
            std::cout << "3. CAPABILITY COMPOSITION DEMONSTRATION (Section 5)\n";
            std::cout << "----------------------------------------------------------------------\n";
            const Capability* c1 = nullptr;
            const Capability* c2 = nullptr;
            for (const auto& c : app.capabilities) {
                if (c.id == "CreateOrder") c1 = &c;
                if (c.id == "MakePayment_API" || c.id == "MakePayment") c2 = &c;
            }
            if (c1 && c2) {
                Capability c12 = embedding.composeCapabilities(*c1, *c2);
                std::cout << "Composite: CompletePurchase = MakePayment o CreateOrder\n";
                std::cout << "  ID:              " << c12.id << "\n";
                std::cout << "  Name:            " << c12.name << "\n";
                std::cout << "  Combined Latency:" << c12.qos.timeCostMs << " ms\n";
                std::cout << "  Combined Cost:   $" << c12.qos.moneyCost << "\n";
                std::cout << "  Reliability:     " << c12.reliability << "\n\n";
            }
        }

        // Run Formal Empirical Experimental Suite
        if (runExp) {
            BenchmarkExperiments::runAll(app, embedding);
        }

        // Planning: Synthesizing the complete end-to-end purchase workflow
        std::cout << "----------------------------------------------------------------------\n";
        std::cout << "WORKFLOW PLANNING: SEARCHING FOR GOAL-SATISFYING CAPABILITY SEQUENCE\n";
        std::cout << "----------------------------------------------------------------------\n";
        WorkflowPlanner planner(app, embedding);
        PlanResult plan = planner.findPlan();

        if (!plan.success) {
            std::cerr << "Error: No capability pipeline could satisfy the fulfillment goal.\n";
            return 1;
        }

        std::cout << "Workflow Status:      SUCCESS [PURCHASE FULFILLED]\n";
        std::cout << "Planning Latency:     " << plan.planningLatencyMicroseconds << " microseconds\n";
        std::cout << "Nodes Evaluated:      " << plan.nodesExpanded << " states\n";
        std::cout << "Total Steps:          " << plan.plannedCapabilities.size() << " capabilities\n";
        std::cout << "Cumulative Latency:   " << plan.totalTimeMs << " ms\n";
        std::cout << "Total Monetary Cost:  $" << std::fixed << std::setprecision(2) << plan.totalMoneyCost << "\n";
        std::cout << "Combined Reliability: " << std::setprecision(2) << plan.combinedReliability * 100.0 << "%\n\n";

        std::cout << "======================================================================\n";
        std::cout << "              SYNTHESIZED E-COMMERCE CAPABILITY PIPELINE              \n";
        std::cout << "======================================================================\n";
        for (size_t i = 0; i < plan.plannedCapabilities.size(); ++i) {
            const auto& cap = plan.plannedCapabilities[i];
            std::cout << "Step " << (i + 1) << ": [" << std::left << std::setw(10) << cap.type << "] "
                      << std::setw(22) << cap.id << " -> " << cap.name
                      << " (" << cap.qos.timeCostMs << " ms, $" << cap.qos.moneyCost << ")\n";
        }
        std::cout << "======================================================================\n\n";

        // Export Artifacts
        EcommerceEngine::exportExecutionText("workflow_execution.txt", app, plan);

        std::cout << "Artifacts Successfully Exported:\n";
        std::cout << "  - Execution Log:      workflow_execution.txt\n\n";

    } catch (const std::exception& ex) {
        std::cerr << "Fatal Error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
