
#ifndef TRAFFICINTELLIGENCE_H
#define TRAFFICINTELLIGENCE_H

#include <iostream>
#include <string>

#include "TrafficManager.h"
#include "sorting.h"

using namespace std;

class TrafficIntelligence {
private:
    TrafficManager manager;

public:
    TrafficIntelligence() = default;

    bool addRoad(const Road& road) {
        return manager.addRoad(road);
    }

    int getRoadCount() const {
        return manager.getRoadCount();
    }

    Road* getRoad(int index) {
        return manager.getRoad(index);
    }

    // Use the manager's single priority calculation.
    void calculatePriority() {
        manager.calculateAllPriorities();
    }

    // Display a sorted copy without changing the manager's road order.
    void displayRoadPriorities() {
        int count = manager.getRoadCount();

        if (count == 0) {
            cout << "No roads available." << endl;
            return;
        }

        Road sorted[100];

        for (int i = 0; i < count; i++) {
            Road* road = manager.getRoad(i);

            if (road != nullptr) {
                sorted[i] = *road;
            }
        }

        RoadSorting sorter;
        sorter.sortRoads(sorted, count);
        sorter.displayRoads(sorted, count);
    }

    // Delegate signal selection to TrafficManager.
    // FIX 3: the fallback selection loop that used to live here was a
    // second, independent way of choosing the green road. It made
    // TrafficIntelligence and TrafficManager disagree on identical data.
    // There must be exactly one selection path, and it lives in
    // TrafficManager::selectNextRoad().
    void updateSignals() {
        manager.selectNextRoad();
    }

    int selectGreenRoad() {
        return manager.selectNextRoad();
    }

    void displayGreenSignalRoad() {
        int index = manager.selectNextRoad();

        if (index == -1) {
            cout << "No active roads with waiting vehicles."
                 << endl;
            return;
        }

        Road* road = manager.getRoad(index);

        if (road == nullptr) {
            cout << "Selected road could not be found." << endl;
            return;
        }

        cout << "\n===== GREEN SIGNAL =====" << endl;
        cout << "Road ID: " << road->getRoadID() << endl;
        cout << "Road Name: " << road->getRoadName() << endl;
        cout << "Priority: " << road->getCurrentPriority() << endl;
        cout << "Signal: " << road->getSignalStatus() << endl;
    }

    void displayTrafficIntelligence() {
        displayRoadPriorities();
        displayGreenSignalRoad();
    }
};

#endif
