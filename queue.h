#ifndef QUEUE_H
#define QUEUE_H

#include <iostream>
#include "vehicle.h"   // was "Vehicle.h" - fails on case-sensitive systems
using namespace std;

class VehicleQueue {
private:
    struct Node {
        Vehicle vehicle;
        Node* next;

        Node(const Vehicle& v) : vehicle(v) {
            next = nullptr;
        }
    };

    Node* frontNode;
    Node* rearNode;
    int count;

public:
    VehicleQueue() {
        frontNode = nullptr;
        rearNode = nullptr;
        count = 0;
    }

    // Rule of Three: this class owns raw "new" memory.
    // A default copy would duplicate the pointers, and both
    // destructors would delete the same nodes -> crash.
    // Copying a queue is not needed here, so forbid it at compile time.
    VehicleQueue(const VehicleQueue&) = delete;
    VehicleQueue& operator=(const VehicleQueue&) = delete;

    void enqueue(const Vehicle& vehicle) {
        Node* newNode = new Node(vehicle);

        if (frontNode == nullptr) {
            frontNode = newNode;
            rearNode = newNode;
        }
        else {
            rearNode->next = newNode;
            rearNode = newNode;
        }

        count++;

        cout << "Vehicle " << newNode->vehicle.getVehicleID()
             << " added to the queue." << endl;
    }

    bool dequeue() {
        if (frontNode == nullptr) {
            cout << "Queue is empty." << endl;
            return false;
        }

        Node* temp = frontNode;

        cout << "Vehicle " << temp->vehicle.getVehicleID()
             << " removed from the queue." << endl;

        frontNode = frontNode->next;

        if (frontNode == nullptr) {
            rearNode = nullptr;
        }

        delete temp;
        count--;

        return true;
    }

    Vehicle* front() {
        if (frontNode == nullptr) {
            return nullptr;
        }

        return &frontNode->vehicle;
    }

    bool isEmpty() const {
        return frontNode == nullptr;
    }

    int size() const {
        return count;
    }

    void display() const {
        if (frontNode == nullptr) {
            cout << "Queue is empty." << endl;
            return;
        }

        Node* current = frontNode;

        while (current != nullptr) {
            cout << current->vehicle.getVehicleID() << endl;
            current = current->next;
        }
    }

    ~VehicleQueue() {
        while (frontNode != nullptr) {
            Node* temp = frontNode;
            frontNode = frontNode->next;
            delete temp;
        }

        rearNode = nullptr;
        count = 0;
    }
};

#endif