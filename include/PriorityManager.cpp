
#include "PriorityManager.h"

int PriorityManager::calculatePriority(const Road& road, int fairnessBonus) {
    // Inactive roads must not receive priority.
    if (!road.getActiveStatus()) {
        return 0;
    }

    int emergencyCount = road.getEmergencyVehicleCount();

    int priority = 0;

    // Emergency vehicles receive a very high priority.
    if (road.getEmergencyStatus() || emergencyCount > 0) {
        priority += EMERGENCY_WEIGHT;
    }

    priority += road.getVehicleCount() * VEHICLE_WEIGHT;
    priority += road.getTrafficDensity() * DENSITY_WEIGHT;
    priority += road.getWaitingTime() * WAITING_WEIGHT;

    // Fairness bonus helps roads that have waited longer.
    if (fairnessBonus > 0) {
        priority += fairnessBonus * FAIRNESS_WEIGHT;
    }

    return priority;
}

void PriorityManager::updateRoadPriority(Road& road, int fairnessBonus) {
    int priority = calculatePriority(road, fairnessBonus);
    road.setCurrentPriority(priority);
}

string PriorityManager::getPriorityReason(
    const Road& road,
    int fairnessBonus
) {
    if (!road.getActiveStatus()) {
        return "Road is inactive";
    }

    if (road.getEmergencyStatus() ||
        road.getEmergencyVehicleCount() > 0) {
        return "Emergency vehicle detected";
    }

    if (fairnessBonus > 0) {
        return "Fairness bonus for waiting road";
    }

    if (road.getWaitingTime() >= 30) {
        return "Long waiting time";
    }

    if (road.getTrafficDensity() >= 80) {
        return "High traffic density";
    }

    if (road.getVehicleCount() >= 50) {
        return "High vehicle count";
    }

    return "Normal traffic priority";
}
