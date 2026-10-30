#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <iostream>
#include <string>
#include "road.h"
using namespace std;

class RoadHashTable {
private:
    static const int TABLE_SIZE = 100;

    struct Node {
        Road road;
        Node* next;

        Node(const Road& r) : road(r) {
            next = nullptr;
        }
    };

    Node* table[TABLE_SIZE];
    int count;

    int hashFunction(const string& roadID) const {
        unsigned long hashValue = 0;

        for (size_t i = 0; i < roadID.length(); i++) {
            hashValue = hashValue * 31 + (unsigned char)roadID[i];
        }

        return (int)(hashValue % TABLE_SIZE);
    }

public:
    RoadHashTable() {
        for (int i = 0; i < TABLE_SIZE; i++) {
            table[i] = nullptr;
        }
        count = 0;
    }

    RoadHashTable(const RoadHashTable&) = delete;
    RoadHashTable& operator=(const RoadHashTable&) = delete;

    bool insertRoad(const Road& road) {
        int index = hashFunction(road.getRoadID());

        Node* current = table[index];

        while (current != nullptr) {
            if (current->road.getRoadID() == road.getRoadID()) {
                current->road = road;
                return true;
            }

            current = current->next;
        }

        Node* newNode = new Node(road);
        newNode->next = table[index];
        table[index] = newNode;
        count++;

        return true;
    }

    Road* searchRoad(const string& roadID) {
        int index = hashFunction(roadID);
        Node* current = table[index];

        while (current != nullptr) {
            if (current->road.getRoadID() == roadID) {
                return &current->road;
            }

            current = current->next;
        }

        return nullptr;
    }

    bool contains(const string& roadID) {
        return searchRoad(roadID) != nullptr;
    }

    bool deleteRoad(const string& roadID) {
        int index = hashFunction(roadID);
        Node* current = table[index];
        Node* previous = nullptr;

        while (current != nullptr) {
            if (current->road.getRoadID() == roadID) {
                if (previous == nullptr) {
                    table[index] = current->next;
                }
                else {
                    previous->next = current->next;
                }

                delete current;
                count--;
                return true;
            }

            previous = current;
            current = current->next;
        }

        return false;
    }

    int size() const {
        return count;
    }

    void displayTable() const {
        if (count == 0) {
            cout << "Hash table is empty." << endl;
            return;
        }

        for (int i = 0; i < TABLE_SIZE; i++) {
            if (table[i] != nullptr) {
                cout << "Index " << i << ": ";

                Node* current = table[i];

                while (current != nullptr) {
                    cout << current->road.getRoadID() << " ";
                    current = current->next;
                }

                cout << endl;
            }
        }
    }

    ~RoadHashTable() {
        for (int i = 0; i < TABLE_SIZE; i++) {
            Node* current = table[i];

            while (current != nullptr) {
                Node* temp = current;
                current = current->next;
                delete temp;
            }

            table[i] = nullptr;
        }

        count = 0;
    }
};

#endif