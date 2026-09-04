#include "traffic.h"
#include <iostream>

using namespace std;



string selectRoad()
{
    int choice;


    cout << "\nSelect Road:\n";

    cout << "1. Road A\n";
    cout << "2. Road B\n";
    cout << "3. Road C\n";
    cout << "4. Road D\n";


    cout << "Enter road number: ";
    cin >> choice;


    if (choice == 1)
    {
        return "Road A";
    }

    if (choice == 2)
    {
        return "Road B";
    }

    if (choice == 3)
    {
        return "Road C";
    }

    if (choice == 4)
    {
        return "Road D";
    }


    cout << "Invalid road number.\n";

    return "";
}


// =====================================================
// MAIN
// =====================================================

int main()
{
    TrafficSystem system;

    int choice;


    do
    {
        cout << "\n=================================\n";
        cout << " ADAPTIVE TRAFFIC MANAGEMENT\n";
        cout << "=================================\n";

        cout << "1. Add Car\n";
        cout << "2. Remove Vehicle\n";
        cout << "3. Send Ambulance\n";
        cout << "4. Run Traffic Cycle\n";
        cout << "5. Show Traffic Status\n";
        cout << "6. Show Statistics\n";
        cout << "7. Show Logs\n";
        cout << "0. Exit\n";


        cout << "\nEnter choice: ";
        cin >> choice;


        switch (choice)
        {

        // =============================================
        // ADD CAR
        // =============================================

        case 1:
        {
            string road =
                selectRoad();


            if (road != "")
            {
                system.addVehicle(
                    road,
                    VehicleType::CAR);


                cout
                    << "Car added successfully to "
                    << road
                    << ".\n";
            }


            break;
        }


        

        case 2:
        {
            string road =
                selectRoad();


            if (road != "")
            {
                system.removeVehicle(
                    road);


                cout
                    << "Vehicle removed from "
                    << road
                    << ".\n";
            }


            break;
        }


        case 3:
        {
            string road =
                selectRoad();


            if (road != "")
            {
                system.sendAmbulance(
                    road);


                cout
                    << "Ambulance sent to "
                    << road
                    << ".\n";
            }


            break;
        }


     
        case 4:
        {
            system.runCycle();


            cout
                << "Traffic cycle completed.\n";


            break;
        }



        case 5:
        {
            system.showRoads();

            break;
        }



        case 6:
        {
            SimulationStats stats =
                system.getStatistics();


            cout
                << "\n========== STATISTICS ==========\n";


            cout
                << "Total Vehicles: "
                << stats.totalVehicles
                << endl;


            cout
                << "Cycles Run: "
                << stats.cyclesRun
                << endl;


            cout
                << "Active Road: "
                << stats.activeRoadLabel
                << endl;


            break;
        }



        case 7:
        {
            system.showLogs();

            break;
        }


       

        case 0:
        {
            cout
                << "Exiting program...\n";

            break;
        }


        default:
        {
            cout
                << "Invalid choice.\n";

            break;
        }

        }

    }
    while (choice != 0);


    return 0;
}
