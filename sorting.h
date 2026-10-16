#ifndef SORTING_H
#define SORTING_H

#include <iostream>
#include "road.h"   // was "Road.h"
using namespace std;

class RoadSorter {
public:
    // Bubble sort, descending by CurrentPriority.
    // Worst case O(n^2); the swapped flag makes an already-sorted
    // array cost O(n) instead of always running n-1 full passes.
    void sortRoads(Road roads[], int count) {
        if (count <= 1) {
            return;
        }

        for (int i = 0; i < count - 1; i++) {
            bool swapped = false;

            for (int j = 0; j < count - i - 1; j++) {
                if (roads[j].getCurrentPriority() <
                    roads[j + 1].getCurrentPriority()) {

                    Road temp = roads[j];
                    roads[j] = roads[j + 1];
                    roads[j + 1] = temp;

                    swapped = true;
                }
            }

            if (swapped == false) {
                break;
            }
        }
    }

    void displaySortedRoads(const Road roads[], int count) {
        if (count <= 0) {
            cout << "No roads available." << endl;
            return;
        }

        for (int i = 0; i < count; i++) {
            cout << "Road ID: " << roads[i].getRoadID() << endl;
            cout << "Priority: "
                 << roads[i].getCurrentPriority() << endl;
            cout << "--------------------" << endl;
        }
    }
};

#endif