
#include "PriorityManager.h"

int PriorityManager::calculatePriority(const Road& road, int fairnessBonus) {
    if (!road.getActiveStatus()) {
        return 0;
    }

    int emergencyCount = road.getEmergencyVehicleCount();

    int priority = 0;

    if (road.getEmergencyStatus() || emergencyCount > 0) {
        priority += EMERGENCY_WEIGHT;
    }

    priority += road.getVehicleCount() * VEHICLE_WEIGHT;
    priority += road.getTrafficDensity() * DENSITY_WEIGHT;
    priority += road.getWaitingTime() * WAITING_WEIGHT;

  
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

int PriorityManager::greenDurationFor(const Road& road) const {
    int needed = road.getVehicleCount() * SECONDS_PER_VEHICLE;

    if (needed < MIN_GREEN_SECONDS) {
        return MIN_GREEN_SECONDS;
    }
    if (needed > MAX_GREEN_SECONDS) {
        return MAX_GREEN_SECONDS;
    }
    return needed;
}

int PriorityManager::vehiclesServedIn(int greenSeconds) const {
    if (greenSeconds <= 0) {
        return 0;
    }
    return greenSeconds/SECONDS_PER_VEHICLE ;
}
