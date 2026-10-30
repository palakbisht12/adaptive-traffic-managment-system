
#ifndef SORTING_H
#define SORTING_H

#include <iostream>
#include "road.h"

using namespace std;

class RoadSorting {
private:
    void merge(Road roads[], int left, int middle, int right) {
        int n1 = middle - left + 1;
        int n2 = right - middle;

        Road* leftRoads = new Road[n1];
        Road* rightRoads = new Road[n2];

        for (int i = 0; i < n1; i++) {
            leftRoads[i] = roads[left + i];
        }

        for (int i = 0; i < n2; i++) {
            rightRoads[i] = roads[middle + 1 + i];
        }

        int i = 0;
        int j = 0;
        int k = left;

        while (i < n1 && j < n2) {
            if (leftRoads[i].getCurrentPriority() >=
                rightRoads[j].getCurrentPriority()) {
                roads[k] = leftRoads[i];
                i++;
            } else {
                roads[k] = rightRoads[j];
                j++;
            }
            k++;
        }

        while (i < n1) {
            roads[k] = leftRoads[i];
            i++;
            k++;
        }

        while (j < n2) {
            roads[k] = rightRoads[j];
            j++;
            k++;
        }

        delete[] leftRoads;
        delete[] rightRoads;
    }

    void mergeSort(Road roads[], int left, int right) {
        if (left >= right) {
            return;
        }

        int middle = left + (right - left) / 2;

        mergeSort(roads, left, middle);
        mergeSort(roads, middle + 1, right);

        merge(roads, left, middle, right);
    }

public:
    void sortRoads(Road roads[], int count) {
        if (count <= 1) {
            return;
        }

        mergeSort(roads, 0, count - 1);
    }

    void displayRoads(Road roads[], int count) {
        cout << "\nRoads sorted by priority:\n";

        for (int i = 0; i < count; i++) {
            cout << roads[i].getRoadID() << " - "
                 << roads[i].getRoadName()
                 << " - Priority: "
                 << roads[i].getCurrentPriority() << endl;
        }
    }
};

#endif
