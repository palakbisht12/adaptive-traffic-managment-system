#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <iostream>
#include <string>
#include "road.h"  
using namespace std;

class RoadHashTable {
private:
    
    static const int TABLE_SIZE = 101;

    struct Node {
        Road road;      
        int roadIndex; 
        Node* next;

        Node(const Road& r, int index = -1) : road(r), roadIndex(index) {
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

    bool insertRoad(const Road& road, int roadIndex = -1) {
        int index = hashFunction(road.getRoadID());

        Node* current = table[index];

        while (current != nullptr) {
            if (current->road.getRoadID() == road.getRoadID()) {
                current->road = road;
                current->roadIndex = roadIndex;
                return true;
            }

            current = current->next;
        }

        Node* newNode = new Node(road, roadIndex);
        newNode->next = table[index];
        table[index] = newNode;
        count++;

        return true;
    }

   
    int searchIndex(const string& roadID) const {
        int index = hashFunction(roadID);

        for (Node* current = table[index]; current != nullptr;
             current = current->next) {
            if (current->road.getRoadID() == roadID) {
                return current->roadIndex;
            }
        }
        return -1;
    }

    int longestChain() const {
        int longest = 0;

        for (int i = 0; i < TABLE_SIZE; i++) {
            int chain = 0;
            for (Node* current = table[i]; current != nullptr;
                 current = current->next) {
                chain++;
            }
            if (chain > longest) {
                longest = chain;
            }
        }
        return longest;
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