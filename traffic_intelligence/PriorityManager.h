
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
};

#endif
