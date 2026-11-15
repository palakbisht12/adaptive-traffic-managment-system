
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
        VehicleCount = 0;
        TrafficDensity = 0;
        WaitingTime = 0;
        EmergencyVehicleCount = 0;
        EmergencyStatus = false;
        CurrentPriority = 0;
        SignalStatus = "RED";
        ActiveStatus = true;

        setVehicleCount(vehicles);
        setTrafficDensity(density);
        setWaitingTime(waiting);
        setEmergencyVehicleCount(emergencyVehicles);
    }

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

    void setRoadID(string id) {
        RoadID = id;
    }

    void setRoadName(string name) {
        RoadName = name;
    }

    void setVehicleCount(int vehicles) {
        VehicleCount = (vehicles >= 0) ? vehicles : 0;
    }

    void setTrafficDensity(int density) {
        if (density < 0) {
            density = 0;
        }

        if (density > 100) {
            density = 100;
        }

        TrafficDensity = density;
    }

    void setWaitingTime(int waiting) {
        WaitingTime = (waiting >= 0) ? waiting : 0;
    }

    void setEmergencyVehicleCount(int count) {
        EmergencyVehicleCount = (count >= 0) ? count : 0;
        EmergencyStatus = (EmergencyVehicleCount > 0);
    }

    void setCurrentPriority(int priority) {
        CurrentPriority = (priority >= 0) ? priority : 0;
    }

    void setSignalStatus(string status) {
        if (status == "RED" ||
            status == "YELLOW" ||
            status == "GREEN") {
            SignalStatus = status;
        } else {
            SignalStatus = "RED";
        }
    }

    void setActiveStatus(bool status) {
        ActiveStatus = status;

        if (!ActiveStatus) {
            SignalStatus = "RED";
        }
    }

    void updateTraffic(int vehicles, int density, int waiting) {
        setVehicleCount(vehicles);
        setTrafficDensity(density);
        setWaitingTime(waiting);
    }

    void updateEmergencyVehicles(int count) {
        setEmergencyVehicleCount(count);
    }

    void activateRoad() {
        setActiveStatus(true);
    }

    void deactivateRoad() {
        setActiveStatus(false);
    }

    void displayRoad() const {
        cout << "Road ID: " << RoadID << endl;
        cout << "Road Name: " << RoadName << endl;
        cout << "Vehicle Count: " << VehicleCount << endl;
        cout << "Traffic Density: " << TrafficDensity << "%" << endl;
        cout << "Waiting Time: " << WaitingTime << endl;
        cout << "Emergency Vehicle Count: "
             << EmergencyVehicleCount << endl;

        cout << "Emergency Status: "
             << (EmergencyStatus ? "Active" : "Inactive") << endl;

        cout << "Current Priority: " << CurrentPriority << endl;
        cout << "Signal Status: " << SignalStatus << endl;

        cout << "Road Status: "
             << (ActiveStatus ? "Active" : "Inactive") << endl;
    }
};

#endif
