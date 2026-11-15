#include "traffic.h"

std::string simulationStateToString(SimulationState state) {
    switch (state) {
        case SimulationState::RUNNING: return "RUNNING";
        case SimulationState::PAUSED:  return "PAUSED";
        case SimulationState::STOPPED:
        default:                       return "STOPPED";
    }
}
