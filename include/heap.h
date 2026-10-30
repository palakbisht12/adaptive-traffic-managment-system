#ifndef HEAP_H
#define HEAP_H

#include <iostream>
#include "road.h"
using namespace std;

class RoadHeap {
private:
    static const int MAX_ROADS = 100;

    Road roads[MAX_ROADS];
    int count;

    void swapRoads(int a, int b) {
        Road temp = roads[a];
        roads[a] = roads[b];
        roads[b] = temp;
    }

    void heapifyUp(int index) {
        while (index > 0) {
            int parent = (index - 1) / 2;

            if (roads[index].getCurrentPriority() >
                roads[parent].getCurrentPriority()) {
                swapRoads(index, parent);
                index = parent;
            }
            else {
                break;
            }
        }
    }

    void heapifyDown(int index) {
        while (true) {
            int largest = index;
            int left = 2 * index + 1;
            int right = 2 * index + 2;

            if (left < count &&
                roads[left].getCurrentPriority() >
                roads[largest].getCurrentPriority()) {
                largest = left;
            }

            if (right < count &&
                roads[right].getCurrentPriority() >
                roads[largest].getCurrentPriority()) {
                largest = right;
            }

            if (largest == index) {
                break;
            }

            swapRoads(index, largest);
            index = largest;
        }
    }

public:
    RoadHeap() {
        count = 0;
    }

    bool insertRoad(const Road& road) {
        if (count == MAX_ROADS) {
            cout << "Heap is full." << endl;
            return false;
        }

        roads[count] = road;
        count++;
        heapifyUp(count - 1);

        return true;
    }

    bool removeHighestPriorityRoad() {
        if (count == 0) {
            cout << "Heap is empty." << endl;
            return false;
        }

        roads[0] = roads[count - 1];
        count--;

        if (count > 0) {
            heapifyDown(0);
        }

        return true;
    }


    bool removeHighestPriorityRoad(Road& removed) {
        if (count == 0) {
            cout << "Heap is empty." << endl;
            return false;
        }

        removed = roads[0];
        return removeHighestPriorityRoad();
    }

    Road* getHighestPriorityRoad() {
        if (count == 0) {
            return nullptr;
        }

        return &roads[0];
    }

    bool isEmpty() const {
        return count == 0;
    }

    int size() const {
        return count;
    }

    void displayHeap() const {
        if (count == 0) {
            cout << "Heap is empty." << endl;
            return;
        }

        for (int i = 0; i < count; i++) {
            cout << "Road ID: " << roads[i].getRoadID()
                 << ", Priority: "
                 << roads[i].getCurrentPriority() << endl;
        }
    }
};

#endif