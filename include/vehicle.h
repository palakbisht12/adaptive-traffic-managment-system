#ifndef VEHICLE_H
#define VEHICLE_H

#include <iostream>
#include <string>
using namespace std;

class Vehicle {
private:
    string VehicleID;
    string VehicleType;
    int ArrivalTime;
    bool IsEmergency;


    static bool isEmergencyType(const string& type) {
        return type == "Emergency" ||
               type == "Ambulance" ||
               type == "Fire Brigade" ||
               type == "Police";
    }

public:
    Vehicle() {
        VehicleID = "";
        VehicleType = "Normal";
        ArrivalTime = 0;
        IsEmergency = false;
    }

    Vehicle(string id, string type, int time, bool status) {
        VehicleID = id;
        VehicleType = type;

        if (time >= 0) {
            ArrivalTime = time;
        }
        else {
            ArrivalTime = 0;
        }


        IsEmergency = isEmergencyType(type) || status;
    }

    string getVehicleID() const {
        return VehicleID;
    }

    void setVehicleID(string id) {
        VehicleID = id;
    }

    string getVehicleType() const {
        return VehicleType;
    }

    void setVehicleType(string type) {
        VehicleType = type;
        IsEmergency = isEmergencyType(type);
    }

    int getArrivalTime() const {
        return ArrivalTime;
    }

    void setArrivalTime(int time) {
        if (time >= 0) {
            ArrivalTime = time;
        }
        else {
            ArrivalTime = 0;
        }
    }

    bool getIsEmergency() const {
        return IsEmergency;
    }

    void setIsEmergency(bool emergency) {
        IsEmergency = emergency;


        if (IsEmergency == true && isEmergencyType(VehicleType) == false) {
            VehicleType = "Emergency";
        }
        else if (IsEmergency == false && isEmergencyType(VehicleType) == true) {
            VehicleType = "Normal";
        }
    }

    void displayVehicleInfo() const {
        cout << "Vehicle ID: " << VehicleID << endl;
        cout << "Vehicle Type: " << VehicleType << endl;
        cout << "Arrival Time: " << ArrivalTime << endl;

        if (IsEmergency == true) {
            cout << "Emergency Status: Yes" << endl;
        }
        else {
            cout << "Emergency Status: No" << endl;
        }
    }
};

#endif