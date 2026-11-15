
#include <iostream>
#include <string>
#include <unordered_map>

#include "road.h"
#include "vehicle.h"
#include "queue.h"
#include "heap.h"
#include "hashtable.h"
#include "sorting.h"
#include "TrafficSimulation.h"
#include "trafficintelligence.h"

using namespace std;

int main() {
    cout << "ADAPTIVE TRAFFIC MANAGEMENT SYSTEM" << endl;

    // Create sample roads.
    Road roads[3] = {
        Road("R101", "Rajpur Road", 40, 55, 12, 0),
        Road("R102", "Saharanpur Road", 75, 90, 30, 0),
        Road("R103", "Haridwar Road", 20, 30, 8, 1)
    };

    // HASH TABLE
    cout << "\n===== HASH TABLE =====" << endl;

    unordered_map<string, Road*> table;

    for (int i = 0; i < 3; i++) {
        table[roads[i].getRoadID()] = &roads[i];
    }

    for (const auto& entry : table) {
        cout << entry.first << " - "
             << entry.second->getRoadName() << endl;
    }

    auto foundEntry = table.find("R102");

    Road* found = nullptr;

    if (foundEntry != table.end()) {
        found = foundEntry->second;
    }

    if (found != nullptr) {
        cout << "Found " << found->getRoadID()
             << " - " << found->getRoadName() << endl;
    } else {
        cout << "Road not found." << endl;
    }

    cout << "Lookup of R999: "
         << (table.find("R999") != table.end()
                 ? "found"
                 : "not found")
         << endl;

    // CUSTOM HASH TABLE
    cout << "\n===== CUSTOM HASH TABLE =====" << endl;

    RoadHashTable roadTable;

    for (int i = 0; i < 3; i++) {
        roadTable.insertRoad(roads[i]);
    }

    roadTable.displayTable();

    Road* searchedRoad = roadTable.searchRoad("R102");

    if (searchedRoad != nullptr) {
        cout << "Custom hash table found: "
             << searchedRoad->getRoadName() << endl;
    } else {
        cout << "Road not found in custom hash table." << endl;
    }

    // TRAFFIC INTELLIGENCE
    cout << "\n===== TRAFFIC INTELLIGENCE =====" << endl;

    TrafficIntelligence system;

    for (int i = 0; i < 3; i++) {
        system.addRoad(roads[i]);
    }

    system.displayTrafficIntelligence();

    // SORTING ROADS BY PRIORITY
    cout << "\n===== SORTING ROADS BY PRIORITY =====" << endl;

    Road ranked[3];

    for (int i = 0; i < 3; i++) {
        ranked[i] = *system.getRoad(i);
    }

    RoadSorting sorter;

    sorter.sortRoads(ranked, 3);
    sorter.displayRoads(ranked, 3);

    // PRIORITY HEAP
    cout << "\n===== PRIORITY HEAP =====" << endl;

    RoadHeap heap;

    for (int i = 0; i < 3; i++) {
        heap.insertRoad(ranked[i]);
    }

    heap.displayHeap();

    Road highest;

    if (heap.removeHighestPriorityRoad(highest)) {
        cout << "Highest-priority road removed: "
             << highest.getRoadID()
             << " (priority "
             << highest.getCurrentPriority() << ")" << endl;

        cout << "Roads remaining in heap: "
             << heap.size() << endl;
    }

    // VEHICLE QUEUE
    cout << "\n===== VEHICLE QUEUE =====" << endl;

    VehicleQueue queue;

    queue.enqueue(Vehicle("V101", "Car", 1, false));
    queue.enqueue(Vehicle("V102", "Ambulance", 2, true));
    queue.enqueue(Vehicle("V103", "Bus", 3, false));

    cout << "\nVehicles in queue ("
         << queue.size() << "):" << endl;

    queue.display();

    Vehicle* nextVehicle = queue.front();

    if (nextVehicle != nullptr) {
        cout << "\nNext vehicle to be served:" << endl;
        nextVehicle->displayVehicleInfo();
    }

    cout << "\nRemoving the first vehicle:" << endl;

    queue.dequeue();

    cout << "\nRemaining vehicles ("
         << queue.size() << "):" << endl;

    queue.display();

    // TRAFFIC SIMULATION
    cout << "\n===== TRAFFIC SIMULATION =====" << endl;

    TrafficSimulation simulation;
    simulation.run(10);

    cout << "\nProgram finished." << endl;

    return 0;
}
