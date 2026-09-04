#include "traffic.h"
#include <iostream>

using namespace std;



Vehicle::Vehicle(VehicleType type)
{
    this->type = type;
}


VehicleType Vehicle::getType() const
{
    return type;
}


bool Vehicle::isEmergency() const
{
    return type == VehicleType::AMBULANCE;
}



Road::Road(string name)
{
    this->name = name;

    waitTime = 0;
    signalState = "red";
    remaining = 0;
}



void Road::addVehicle(VehicleType type)
{
    Vehicle vehicle(type);

    vehicles.push_back(vehicle);
}



void Road::removeVehicle()
{
    if (!vehicles.empty())
    {
        vehicles.pop_back();

        if (vehicles.empty())
        {
            waitTime = 0;
        }
    }
}



void Road::increaseWaitingTime()
{
    if (!vehicles.empty())
    {
        waitTime += 10;
    }
}



void Road::processGreenSignal()
{
    if (!vehicles.empty())
    {
        // One vehicle passes through the green signal
        vehicles.erase(vehicles.begin());

        // Reduce waiting time
        if (waitTime >= 10)
        {
            waitTime -= 10;
        }
    }

    // Reset waiting time if no vehicles remain
    if (vehicles.empty())
    {
        waitTime = 0;
    }
}


// -----------------------------------------------------
// GET ROAD NAME
// -----------------------------------------------------

string Road::getName() const
{
    return name;
}


int Road::getVehicleCount() const
{
    return static_cast<int>(vehicles.size());
}



int Road::getWaitTime() const
{
    return waitTime;
}



bool Road::hasEmergency() const
{
    for (const Vehicle& vehicle : vehicles)
    {
        if (vehicle.isEmergency())
        {
            return true;
        }
    }

    return false;
}


string Road::getSignalState() const
{
    return signalState;
}



int Road::getRemaining() const
{
    return remaining;
}


void Road::setSignalState(string state)
{
    signalState = state;
}


void Road::setRemaining(int time)
{
    remaining = time;
}


TrafficController::TrafficController()
{
    // Default configuration is automatically used.
}



Decision TrafficController::makeDecision(
    const vector<Road>& roads) const
{
    Decision decision;

    double highestScore = -1;




    for (const Road& road : roads)
    {
        if (road.hasEmergency())
        {
            decision.roadName = road.getName();

            decision.isEmergency = true;

            decision.greenDuration = config.maxGreen;

            return decision;
        }
    }



    for (const Road& road : roads)
    {
        double score =
            road.getVehicleCount() * config.densityWeight
            +
            road.getWaitTime() * config.waitWeight;


        if (score > highestScore)
        {
            highestScore = score;

            decision.roadName = road.getName();
        }
    }



    if (highestScore <= 0)
    {
        decision.roadName = "";

        decision.greenDuration = 0;

        return decision;
    }



    decision.greenDuration =
        config.minGreen +
        static_cast<int>(highestScore);


    // Minimum green time

    if (decision.greenDuration < config.minGreen)
    {
        decision.greenDuration = config.minGreen;
    }


    // Maximum green time

    if (decision.greenDuration > config.maxGreen)
    {
        decision.greenDuration = config.maxGreen;
    }


    return decision;
}



void TrafficController::updateSignals(
    vector<Road>& roads,
    const Decision& decision) const
{
    for (Road& road : roads)
    {
        if (road.getName() == decision.roadName)
        {
            road.setSignalState("green");

            road.setRemaining(decision.greenDuration);
        }
        else
        {
            road.setSignalState("red");

            road.setRemaining(0);
        }
    }
}



TrafficSystem::TrafficSystem()
{
    roads.push_back(Road("Road A"));
    roads.push_back(Road("Road B"));
    roads.push_back(Road("Road C"));
    roads.push_back(Road("Road D"));

    activeRoad = "";

    cyclesRun = 0;
}


void TrafficSystem::addVehicle(
    string roadName,
    VehicleType type)
{
    for (Road& road : roads)
    {
        if (road.getName() == roadName)
        {
            road.addVehicle(type);


            LogEntry entry;


            if (type == VehicleType::AMBULANCE)
            {
                entry.message =
                    "Ambulance added to " + roadName;

                entry.category = "emergency";
            }
            else
            {
                entry.message =
                    "Car added to " + roadName;

                entry.category = "info";
            }


            logs.push_back(entry);


            // IMPORTANT:
            // Immediately recalculate signal
            refreshSignals();


            return;
        }
    }
}



void TrafficSystem::removeVehicle(string roadName)
{
    for (Road& road : roads)
    {
        if (road.getName() == roadName)
        {
            road.removeVehicle();


            LogEntry entry;

            entry.message =
                "Vehicle removed from " + roadName;

            entry.category = "info";


            logs.push_back(entry);


            // Recalculate signal
            refreshSignals();


            return;
        }
    }
}




void TrafficSystem::sendAmbulance(string roadName)
{
    for (Road& road : roads)
    {
        if (road.getName() == roadName)
        {
            road.addVehicle(VehicleType::AMBULANCE);


            LogEntry entry;

            entry.message =
                "Emergency ambulance sent to " + roadName;

            entry.category = "emergency";


            logs.push_back(entry);


            // Immediately give emergency priority
            refreshSignals();


            return;
        }
    }
}


// =====================================================
// REFRESH SIGNALS
// =====================================================

void TrafficSystem::refreshSignals()
{
    // Ask controller for the best road
    Decision decision =
        controller.makeDecision(roads);


    // Update all traffic signals
    controller.updateSignals(
        roads,
        decision);


    // Store active road
    activeRoad = decision.roadName;
}


void TrafficSystem::runCycle()
{
  

    Decision decision =
        controller.makeDecision(roads);


   

    controller.updateSignals(
        roads,
        decision);


   

    activeRoad =
        decision.roadName;


    

    for (Road& road : roads)
    {
        if (road.getName() == activeRoad)
        {
            // One vehicle passes
            road.processGreenSignal();
        }
        else
        {
            // Red roads wait
            road.increaseWaitingTime();
        }
    }


    cyclesRun++;



    Decision nextDecision =
        controller.makeDecision(roads);


    controller.updateSignals(
        roads,
        nextDecision);


    activeRoad =
        nextDecision.roadName;


    LogEntry entry;


    if (activeRoad != "")
    {
        entry.message =
            activeRoad +
            " received GREEN signal for " +
            to_string(nextDecision.greenDuration) +
            " seconds.";


        if (nextDecision.isEmergency)
        {
            entry.category = "emergency";
        }
        else
        {
            entry.category = "info";
        }
    }
    else
    {
        entry.message =
            "No traffic detected.";

        entry.category = "info";
    }


    logs.push_back(entry);
}



SimulationStats TrafficSystem::getStatistics() const
{
    SimulationStats stats;


    stats.cyclesRun =
        cyclesRun;


    if (activeRoad == "")
    {
        stats.activeRoadLabel = "-";
    }
    else
    {
        stats.activeRoadLabel =
            activeRoad;
    }


    for (const Road& road : roads)
    {
        stats.totalVehicles +=
            road.getVehicleCount();
    }


    return stats;
}



void TrafficSystem::showRoads() const
{
    cout << "\n========== TRAFFIC STATUS ==========\n";


    for (const Road& road : roads)
    {
        cout
            << road.getName()

            << " | Vehicles: "
            << road.getVehicleCount()

            << " | Waiting: "
            << road.getWaitTime()
            << " sec"

            << " | Emergency: "
            << (road.hasEmergency()
                ? "YES"
                : "NO")

            << " | Signal: "
            << road.getSignalState()

            << " | Remaining: "
            << road.getRemaining()
            << " sec\n";
    }
}



void TrafficSystem::showLogs() const
{
    cout << "\n========== SYSTEM LOGS ==========\n";


    for (const LogEntry& log : logs)
    {
        cout
            << "["
            << log.category
            << "] "
            << log.message
            << endl;
    }
}
