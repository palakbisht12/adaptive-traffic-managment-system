
#ifndef TRAFFIC_MANAGER_H
#define TRAFFIC_MANAGER_H

#include <iostream>
#include <string>

#include "road.h"
#include "vehicle.h"
#include "queue.h"
#include "hashtable.h"
#include "heap.h"
#include "searching.h"
#include "PriorityManager.h"

using namespace std;

class TrafficManager {
private:
    static const int MAX_ROADS = 100;

    Road roads[MAX_ROADS];
    VehicleQueue vehicleQueues[MAX_ROADS];

    int roadCount;
    int nextVehicleID;
    int lastSelectedRoadIndex;
    int consecutiveSelections;

    RoadHashTable roadTable;
    PriorityManager priorityManager;

    int findRoadIndex(const string& roadID);

    bool hasDemand(int index) const;

public:
    TrafficManager();

    bool addRoad(const Road& road);
    bool removeRoad(const string& roadID);

    bool updateRoadTraffic(
        const string& roadID,
        int vehicleCount,
        int trafficDensity
    );

    bool addVehicle(const string& roadID, const Vehicle& vehicle);

    bool addEmergencyVehicle(
        const string& roadID,
        const Vehicle& vehicle
    );

    bool removeVehicle(const string& roadID);

    bool updateWaitingTime(const string& roadID, int waitingTime);
    bool updateTrafficDensity(const string& roadID, int density);

    void calculateAllPriorities();
    int selectNextRoad();
    bool changeSignal(const string& roadID, const string& signal);

    void processTrafficCycle();
    void displayAllRoads() const;

    int getRoadCount() const;

    Road* getRoad(const string& roadID);
    Road* getRoad(int index);

    string getSelectionReason(int index);

    int findVehiclePosition(
        const string& roadID,
        const string& vehicleID
    );

    int listQueue(
        const string& roadID,
        Vehicle out[],
        int maxCount
    ) const;
};

#endif
