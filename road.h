#ifndef ROAD_H
#define ROAD_H

#include <iostream>
#include <string>
using namespace std;

class Road {
private:
    string RoadID;
    string RoadName;
    int VehicleCount;
    int TrafficDensity;
    int WaitingTime;
    int EmergencyVehicleCount;
    bool EmergencyStatus;
    int CurrentPriority;
    string SignalStatus;
    bool ActiveStatus;

public:
    Road() {
        RoadID = "";
        RoadName = "";
        VehicleCount = 0;
        TrafficDensity = 0;
        WaitingTime = 0;
        EmergencyVehicleCount = 0;
        EmergencyStatus = false;
        CurrentPriority = 0;
        SignalStatus = "RED";
        ActiveStatus = true;
    }

    Road(string id, string name, int vehicles, int density,
         int waiting, int emergencyVehicles) {
        RoadID = id;
        RoadName = name;

        CurrentPriority = 0;
        SignalStatus = "RED";
        ActiveStatus = true;

        // Reuse the setters so validation lives in exactly one place.
        setVehicleCount(vehicles);
        setTrafficDensity(density);
        setWaitingTime(waiting);
        setEmergencyVehicleCount(emergencyVehicles);
    }

    // ---- Getters ----
    // All getters are const: they promise not to modify the object.
    // Without this you cannot read a Road through a "const Road&".
    string getRoadID() const {
        return RoadID;
    }

    string getRoadName() const {
        return RoadName;
    }

    int getVehicleCount() const {
        return VehicleCount;
    }

    int getTrafficDensity() const {
        return TrafficDensity;
    }

    int getWaitingTime() const {
        return WaitingTime;
    }

    int getEmergencyVehicleCount() const {
        return EmergencyVehicleCount;
    }

    bool getEmergencyStatus() const {
        return EmergencyStatus;
    }

    int getCurrentPriority() const {
        return CurrentPriority;
    }

    string getSignalStatus() const {
        return SignalStatus;
    }

    bool getActiveStatus() const {
        return ActiveStatus;
    }

    // ---- Setters ----
    void setRoadID(string id) {
        RoadID = id;
    }

    void setRoadName(string name) {
        RoadName = name;
    }

    void setVehicleCount(int vehicles) {
        if (vehicles >= 0) {
            VehicleCount = vehicles;
        } else {
            VehicleCount = 0;
        }
    }

    void setTrafficDensity(int density) {
        if (density >= 0 && density <= 100) {
            TrafficDensity = density;
        } else {
            TrafficDensity = 0;
        }
    }

    void setWaitingTime(int waiting) {
        if (waiting >= 0) {
            WaitingTime = waiting;
        } else {
            WaitingTime = 0;
        }
    }

    void setEmergencyVehicleCount(int emergencyVehicles) {
        if (emergencyVehicles >= 0) {
            EmergencyVehicleCount = emergencyVehicles;
        } else {
            EmergencyVehicleCount = 0;
        }

        EmergencyStatus = (EmergencyVehicleCount > 0);
    }

    void setCurrentPriority(int priority) {
        if (priority >= 0) {
            CurrentPriority = priority;
        } else {
            CurrentPriority = 0;
        }
    }

    void setSignalStatus(string status) {
        if (status == "RED" || status == "YELLOW" ||
            status == "GREEN") {
            SignalStatus = status;
        } else {
            SignalStatus = "RED";
        }
    }

    void setActiveStatus(bool status) {
        ActiveStatus = status;

        // An inactive road must never hold a green signal.
        if (ActiveStatus == false) {
            SignalStatus = "RED";
        }
    }

    // ---- Operations ----
    void updateTraffic(int vehicles, int density, int waiting) {
        setVehicleCount(vehicles);
        setTrafficDensity(density);
        setWaitingTime(waiting);
    }

    void updateEmergencyVehicles(int emergencyVehicles) {
        setEmergencyVehicleCount(emergencyVehicles);
    }

    void activateRoad() {
        setActiveStatus(true);
    }

    void deactivateRoad() {
        setActiveStatus(false);
    }

    void displayRoad() const {
        cout << "\nRoad ID: " << RoadID << endl;
        cout << "Road Name: " << RoadName << endl;
        cout << "Vehicle Count: " << VehicleCount << endl;
        cout << "Traffic Density: " << TrafficDensity << "%" << endl;
        cout << "Waiting Time: " << WaitingTime << endl;
        cout << "Emergency Vehicle Count: "
             << EmergencyVehicleCount << endl;

        if (EmergencyStatus == true) {
            cout << "Emergency Status: Active" << endl;
        }
        else {
            cout << "Emergency Status: Inactive" << endl;
        }

        cout << "Current Priority: " << CurrentPriority << endl;
        cout << "Signal Status: " << SignalStatus << endl;

        if (ActiveStatus == true) {
            cout << "Road Status: Active" << endl;
        } else {
            cout << "Road Status: Inactive" << endl;
        }
    }
};

#endif
