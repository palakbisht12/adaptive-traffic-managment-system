
#ifndef SEARCHING_H
#define SEARCHING_H

#include <iostream>
#include <string>
#include "road.h"
#include "vehicle.h"

using namespace std;

class RoadVehicleSearch {
public:
    int searchRoad(Road roads[], int count, string roadID) {
        for (int i = 0; i < count; i++) {
            if (roads[i].getRoadID() == roadID) {
                return i;
            }
        }

        return -1;
    }

    int searchVehicle(Vehicle vehicles[], int count, string vehicleID) {
        for (int i = 0; i < count; i++) {
            if (vehicles[i].getVehicleID() == vehicleID) {
                return i;
            }
        }

        return -1;
    }

    void displayRoadSearch(Road roads[], int count, string roadID) {
        int index = searchRoad(roads, count, roadID);

        if (index != -1) {
            cout << "Road found: "
                 << roads[index].getRoadName() << endl;
        } else {
            cout << "Road not found." << endl;
        }
    }

    void displayVehicleSearch(
        Vehicle vehicles[], int count, string vehicleID) {

        int index = searchVehicle(vehicles, count, vehicleID);

        if (index != -1) {
            cout << "Vehicle found: "
                 << vehicles[index].getVehicleID() << endl;
        } else {
            cout << "Vehicle not found." << endl;
        }
    }
};

#endif
