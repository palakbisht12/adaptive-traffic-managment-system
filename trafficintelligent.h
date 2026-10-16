#ifndef TRAFFICINTELLIGENCE_H
#define TRAFFICINTELLIGENCE_H

#include <iostream>
#include "road.h"
#include "heap.h"
#include "sorting.h"
using namespace std;

// This used to live in trafficintelligent.cpp. A class defined only
// in a .cpp file cannot be used by main.cpp, so the whole class was
// dead code. Class definitions belong in headers.
class TrafficIntelligence {
private:
    static const int MAX_ROADS = 100;

    // Weights are explicit so the formula can be tuned and defended.
    static const int EMERGENCY_WEIGHT = 100;
    static const int DENSITY_WEIGHT   = 1;
    static const int WAITING_WEIGHT   = 1;

    Road roads[MAX_ROADS];
    int roadCount;

public:
    TrafficIntelligence() {
        roadCount = 0;
    }

    bool addRoad(const Road& road) {
        if (roadCount == MAX_ROADS) {
            cout << "Road limit reached." << endl;
            return false;
        }

        roads[roadCount] = road;
        roadCount++;

        return true;
    }

    int getRoadCount() const {
        return roadCount;
    }

    Road* getRoad(int index) {
        if (index < 0 || index >= roadCount) {
            return nullptr;
        }

        return &roads[index];
    }

    void calculatePriority() {
        for (int i = 0; i < roadCount; i++) {
            if (roads[i].getActiveStatus() == false) {
                roads[i].setCurrentPriority(0);
                continue;
            }

            int priority =
                roads[i].getTrafficDensity() * DENSITY_WEIGHT +
                roads[i].getWaitingTime()    * WAITING_WEIGHT +
                roads[i].getEmergencyVehicleCount() * EMERGENCY_WEIGHT;

            roads[i].setCurrentPriority(priority);
        }
    }

    void displayRoadPriorities() {
        if (roadCount == 0) {
            cout << "No roads available." << endl;
            return;
        }

        calculatePriority();

        // Sort a COPY. The original version sorted the member array
        // in place, so merely displaying the report permanently
        // reordered the system's road list - a side effect a
        // "display" function should never have.
        Road sorted[MAX_ROADS];
        for (int i = 0; i < roadCount; i++) {
            sorted[i] = roads[i];
        }

        RoadSorter sorter;
        sorter.sortRoads(sorted, roadCount);

        cout << "\n===== ROAD PRIORITIES =====" << endl;
        sorter.displaySortedRoads(sorted, roadCount);
    }

    // Picks the winner AND updates the stored signal state, so the
    // objects agree with what was printed. The old version printed
    // "Signal: GREEN" without ever calling setSignalStatus, leaving
    // every road stuck on RED internally.
    void updateSignals() {
        calculatePriority();

        RoadHeap heap;
        for (int i = 0; i < roadCount; i++) {
            if (roads[i].getActiveStatus() == true) {
                heap.insertRoad(roads[i]);
            }
        }

        Road* winner = heap.getHighestPriorityRoad();

        if (winner == nullptr) {
            cout << "No active roads available." << endl;
            return;
        }

        string greenID = winner->getRoadID();

        for (int i = 0; i < roadCount; i++) {
            if (roads[i].getRoadID() == greenID &&
                roads[i].getActiveStatus() == true) {
                roads[i].setSignalStatus("GREEN");
            }
            else {
                roads[i].setSignalStatus("RED");
            }
        }
    }

    void displayGreenSignalRoad() {
        if (roadCount == 0) {
            cout << "No roads available." << endl;
            return;
        }

        updateSignals();

        for (int i = 0; i < roadCount; i++) {
            if (roads[i].getSignalStatus() == "GREEN") {
                cout << "\n===== GREEN SIGNAL =====" << endl;
                cout << "Road ID: " << roads[i].getRoadID() << endl;
                cout << "Road Name: " << roads[i].getRoadName() << endl;
                cout << "Priority: "
                     << roads[i].getCurrentPriority() << endl;
                cout << "Signal: GREEN" << endl;
                return;
            }
        }
    }

    void displayTrafficIntelligence() {
        // The old version called calculatePriority() three times for
        // one report. Each helper already recomputes what it needs.
        displayRoadPriorities();
        displayGreenSignalRoad();
    }
};

#endif