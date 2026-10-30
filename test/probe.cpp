
#include <iostream>
#include "trafficintelligence.h"

int main() {
    TrafficIntelligence system;

    system.addRoad(Road("R1", "Light Traffic", 10, 20, 5, 0));
    system.addRoad(Road("R2", "Heavy Traffic", 90, 95, 40, 0));
    system.addRoad(Road("R3", "Ambulance Road", 5, 10, 2, 1));

    system.calculatePriority();

    for (int i = 0; i < system.getRoadCount(); ++i) {
        Road* road = system.getRoad(i);
        std::cout << road->getRoadID()
                  << " priority=" << road->getCurrentPriority()
                  << " active=" << road->getActiveStatus()
                  << " emergency=" << road->getEmergencyStatus()
                  << " vehicles=" << road->getVehicleCount()
                  << " signal=" << road->getSignalStatus()
                  << '\n';
    }

    system.updateSignals();

    std::cout << "Selected index: " << system.selectGreenRoad() << '\n';
}
