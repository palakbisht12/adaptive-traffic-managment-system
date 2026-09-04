#include <iostream>
#include "traffic.h"

using namespace std;

int main()
{
    TrafficState state;
    ControllerConfig config;
    vector<LogEntry> logs;

    // Create roads
    for (string road : ROAD_NAMES)
    {
        state.roads[road] = RoadState();
    }

    int choice;

    do
    {
        cout << "\n====================================\n";
        cout << "   ADAPTIVE TRAFFIC MANAGEMENT\n";
        cout << "====================================\n";

        cout << "1. Add Car\n";
        cout << "2. Remove Car\n";
        cout << "3. Send Ambulance\n";
        cout << "4. Show Traffic Status\n";
        cout << "5. Run Traffic Cycle\n";
        cout << "6. Show Statistics\n";
        cout << "0. Exit\n";

        cout << "\nEnter your choice: ";
        cin >> choice;

        if (choice == 1)
        {
            int roadChoice;

            cout << "\nSelect Road:\n";
            cout << "1. Road A\n";
            cout << "2. Road B\n";
            cout << "3. Road C\n";
            cout << "4. Road D\n";

            cout << "Enter road number: ";
            cin >> roadChoice;

            if (roadChoice >= 1 && roadChoice <= 4)
            {
                string road = ROAD_NAMES[roadChoice - 1];

                addVehicle(state, road, VehicleType::CAR);

                cout << "Car added to " << road << endl;
            }
            else
            {
                cout << "Invalid road!\n";
            }
        }

        else if (choice == 2)
        {
            int roadChoice;

            cout << "\nSelect Road:\n";
            cout << "1. Road A\n";
            cout << "2. Road B\n";
            cout << "3. Road C\n";
            cout << "4. Road D\n";

            cout << "Enter road number: ";
            cin >> roadChoice;

            if (roadChoice >= 1 && roadChoice <= 4)
            {
                string road = ROAD_NAMES[roadChoice - 1];

                removeVehicle(state, road);

                cout << "Car removed from " << road << endl;
            }
            else
            {
                cout << "Invalid road!\n";
            }
        }

        else if (choice == 3)
        {
            int roadChoice;

            cout << "\nAmbulance Road:\n";
            cout << "1. Road A\n";
            cout << "2. Road B\n";
            cout << "3. Road C\n";
            cout << "4. Road D\n";

            cout << "Enter road number: ";
            cin >> roadChoice;

            if (roadChoice >= 1 && roadChoice <= 4)
            {
                string road = ROAD_NAMES[roadChoice - 1];

                sendAmbulance(state, road);

                cout << "Ambulance sent to " << road << endl;
            }
            else
            {
                cout << "Invalid road!\n";
            }
        }

        else if (choice == 4)
        {
            cout << "\n========== TRAFFIC STATUS ==========\n";

            for (string road : ROAD_NAMES)
            {
                RoadState r = state.roads[road];

                cout << "\n" << road << endl;
                cout << "Vehicles: " << r.count << endl;
                cout << "Waiting Time: " << r.waitTime << " seconds" << endl;
                cout << "Signal: " << r.signalState << endl;

                if (r.hasEmergency)
                {
                    cout << "Emergency: YES" << endl;
                }
                else
                {
                    cout << "Emergency: NO" << endl;
                }
            }
        }

        else if (choice == 5)
        {
            Decision decision = makeDecision(state, config);

            if (decision.roadName != "")
            {
                updateSignals(state, decision);

                updateWaitingTime(state);

                state.cyclesRun++;

                cout << "\nTraffic cycle completed.\n";
                cout << "Active Road: " << decision.roadName << endl;
                cout << "Green Duration: "
                     << decision.greenDuration
                     << " seconds\n";

                if (decision.isEmergency)
                {
                    cout << "Emergency priority given!\n";
                }
            }
            else
            {
                cout << "\nNo vehicles or emergency found.\n";
            }
        }

        else if (choice == 6)
        {
            SimulationStats stats = getStatistics(state);

            cout << "\n========== STATISTICS ==========\n";
            cout << "Total Vehicles: "
                 << stats.totalVehicles << endl;

            cout << "Cycles Run: "
                 << stats.cyclesRun << endl;

            cout << "Active Road: "
                 << stats.activeRoadLabel << endl;
        }

        else if (choice == 0)
        {
            cout << "\nExiting program...\n";
        }

        else
        {
            cout << "\nInvalid choice!\n";
        }

    } while (choice != 0);

    return 0;
}
