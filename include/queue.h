
#ifndef QUEUE_H
#define QUEUE_H

#include <iostream>
#include "vehicle.h"

using namespace std;

class VehicleQueue {
private:
    struct Node {
        Vehicle vehicle;
        Node* next;

        Node(const Vehicle& v) : vehicle(v), next(nullptr) {}
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

    VehicleQueue(const VehicleQueue&) = delete;
    VehicleQueue& operator=(const VehicleQueue&) = delete;

    void enqueue(const Vehicle& vehicle) {
        Node* newNode = new Node(vehicle);

        if (frontNode == nullptr) {
            frontNode = newNode;
            rearNode = newNode;
        } else {
            rearNode->next = newNode;
            rearNode = newNode;
        }

        count++;

        cout << "Vehicle "
             << newNode->vehicle.getVehicleID()
             << " added to the queue." << endl;
    }

    bool serveVehicle(Vehicle& servedVehicle) {
        if (frontNode == nullptr) {
            return false;
        }

        Node* temp = frontNode;
        servedVehicle = temp->vehicle;

        frontNode = frontNode->next;

        if (frontNode == nullptr) {
            rearNode = nullptr;
        }

        delete temp;
        count--;

        return true;
    }

    bool dequeue() {
        Vehicle servedVehicle;

        if (!serveVehicle(servedVehicle)) {
            cout << "Queue is empty." << endl;
            return false;
        }

        cout << "Vehicle "
             << servedVehicle.getVehicleID()
             << " removed from the queue." << endl;

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


int getFirstEmergencyPosition() const {
    Node* current = frontNode;
    int position = 1;

    while (current != nullptr) {
        if (current->vehicle.getIsEmergency()) {
            return position;
        }

        current = current->next;
        position++;
    }

    return -1;  // No emergency vehicle found
}

bool serveEmergencyVehicle(Vehicle& servedVehicle) {
    Node* current = frontNode;
    Node* previous = nullptr;

    while (current != nullptr) {
        if (current->vehicle.getIsEmergency()) {
            servedVehicle = current->vehicle;

            // Removing the first node.
            if (previous == nullptr) {
                frontNode = current->next;
            }
            // Removing a node from the middle.
            else {
                previous->next = current->next;
            }

            // If the removed node was the rear node.
            if (current == rearNode) {
                rearNode = previous;
            }

            delete current;
            count--;

            return true;
        }

        previous = current;
        current = current->next;
    }

    // No emergency vehicle was found.
    return false;
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
