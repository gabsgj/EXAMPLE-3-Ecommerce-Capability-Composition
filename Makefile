CXX ?= c++
CXXFLAGS ?= -std=c++17 -O3 -Wall -Wextra -I. -I./third_party
TARGET = ecommerce_composer
SRCS = src/main.cpp

all: $(TARGET)

$(TARGET): $(SRCS) include/types.hpp include/embedding.hpp include/planner.hpp include/engine.hpp include/experiments.hpp
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

run: $(TARGET)
	./$(TARGET) ecommerce_purchase_flow.json --experiments --vectors --compatibility --compose

clean:
	rm -f $(TARGET) workflow_execution.txt visualizer_manifest.json

.PHONY: all run clean
