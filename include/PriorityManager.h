#ifndef PRIORITY_MANAGER_H
#define PRIORITY_MANAGER_H

#include <iostream>
#include <string>
#include "road.h"

using namespace std;

class PriorityManager {
private:

    static const int EMERGENCY_WEIGHT = 1000;
    static const int VEHICLE_WEIGHT = 2;
    static const int DENSITY_WEIGHT = 1;
    static const int WAITING_WEIGHT = 2;
    static const int FAIRNESS_WEIGHT = 5;

    static const int MIN_GREEN_SECONDS   = 15;
    static const int MAX_GREEN_SECONDS   = 50;

    static const int SECONDS_PER_VEHICLE = 2;

public:
    int calculatePriority(const Road& road, int fairnessBonus = 0);

    void updateRoadPriority(
        Road& road,
        int fairnessBonus = 0
    );

    string getPriorityReason(
        const Road& road,
        int fairnessBonus = 0
    );

    int greenDurationFor(const Road& road) const;

    int vehiclesServedIn(int greenSeconds) const;
};

#endif