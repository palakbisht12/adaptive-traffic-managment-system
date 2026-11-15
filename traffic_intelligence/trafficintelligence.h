
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

    void calculatePriority() {
        manager.calculateAllPriorities();
    }

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
