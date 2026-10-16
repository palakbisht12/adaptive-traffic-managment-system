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

        if (vehicles >= 0) {
            VehicleCount = vehicles;
        } else {
            VehicleCount = 0;
        }

        if (density >= 0 && density <= 100) {
            TrafficDensity = density;
        } else {
            TrafficDensity = 0;
        }

        if (waiting >= 0) {
            WaitingTime = waiting;
        } else {
            WaitingTime = 0;
        }

        if (emergencyVehicles >= 0) {
            EmergencyVehicleCount = emergencyVehicles;
        } else {
            EmergencyVehicleCount = 0;
        }

        if (EmergencyVehicleCount > 0) {
            EmergencyStatus = true;
        } else {
            EmergencyStatus = false;
        }

        CurrentPriority = 0;
        SignalStatus = "RED";
        ActiveStatus = true;
    }

    string getRoadID() {
        return RoadID;
    }

    string getRoadName() {
        return RoadName;
    }

    int getVehicleCount() {
        return VehicleCount;
    }

    int getTrafficDensity() {
        return TrafficDensity;
    }

    int getWaitingTime() {
        return WaitingTime;
    }

    int getEmergencyVehicleCount() {
        return EmergencyVehicleCount;
    }

    bool getEmergencyStatus() {
        return EmergencyStatus;
    }

    int getCurrentPriority() {
        return CurrentPriority;
    }

    string getSignalStatus() {
        return SignalStatus;
    }

    bool getActiveStatus() {
        return ActiveStatus;
    }

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

        if (EmergencyVehicleCount > 0) {
            EmergencyStatus = true;
        } else {
            EmergencyStatus = false;
        }
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
    }

    void updateTraffic(int vehicles, int density, int waiting) {
        setVehicleCount(vehicles);
        setTrafficDensity(density);
        setWaitingTime(waiting);
    }

    void updateEmergencyVehicles(int emergencyVehicles) {
        setEmergencyVehicleCount(emergencyVehicles);
    }

    void activateRoad() {
        ActiveStatus = true;
    }

    void deactivateRoad() {
        ActiveStatus = false;
        SignalStatus = "RED";
    }

    void displayRoad() {
        cout << "\nRoad ID: " << RoadID << endl;
        cout << "Road Name: " << RoadName << endl;
        cout << "Vehicle Count: " << VehicleCount << endl;
        cout << "Traffic Density: " << TrafficDensity << "%" << endl;
        cout << "Waiting Time: " << WaitingTime << endl;
        cout << "Emergency Vehicle Count: "
             << EmergencyVehicleCount << endl;

        if (EmergencyStatus == true) {
            cout << "Emergency Status: Active" << endl;
        } else {
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
