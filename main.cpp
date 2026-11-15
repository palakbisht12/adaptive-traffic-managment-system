#include <iostream>
#include <string>
#include <cstdlib>

#include "integration/Integration.h"
#include "integration/API.h"
#include "integration/RequestHandler.h"
#include "integration/HttpServer.h"

using namespace std;

static void banner() {
    cout << "\n"
         << "===========================================================\n"
         << "  ADAPTIVE TRAFFIC MANAGEMENT SYSTEM\n"
         << "  Member 1 Intelligence | Member 2 Storage\n"
         << "  Member 3 Dashboard    | Member 4 Integration\n"
         << "===========================================================\n";
}

int main(int argc, char** argv) {
    int port = 8080;
    string dataTarget = "traffic_data";
    string frontendDir = "frontend";
    bool verbose = true;
    bool seed = true;

    for (int i = 1; i < argc; i++) {
        const string arg = argv[i];

        if (arg == "--port" && i + 1 < argc) {
            port = atoi(argv[++i]);
        }
        else if (arg == "--data" && i + 1 < argc) {
            dataTarget = argv[++i];
        }
        else if (arg == "--frontend" && i + 1 < argc) {
            frontendDir = argv[++i];
        }
        else if (arg == "--quiet") {
            verbose = false;
        }
        else if (arg == "--no-seed") {
            seed = false;
        }
        else if (arg == "--help" || arg == "-h") {
            cout << "Usage: traffic_server [--port N] [--data PATH]"
                 << " [--frontend DIR] [--quiet] [--no-seed]\n";
            return 0;
        }
        else {
            cout << "Unknown option: " << arg << " (try --help)\n";
            return 1;
        }
    }

    banner();

    Integration system;

    OperationResult storage = system.initialise(dataTarget);
    cout << "  storage   : " << storage.message << "\n";

    if (system.isDatabaseReady()) {
        OperationResult restored = system.loadFromDatabase();
        cout << "  restore   : " << restored.message << "\n";
    }

    if (seed) {
        OperationResult seeded = system.seedDemoRoads();
        cout << "  roads     : " << seeded.message << "\n";
    }

    API api(system, frontendDir);
    RequestHandler handler(api, verbose);

    HttpServer server;
    server.setHandler(&RequestHandler::dispatch, &handler);

    server.setIdleHandler(
        [](void* context) {
            static_cast<Integration*>(context)->tickIfRunning();
        },
        &system
    );

    string error;

    if (!server.start(port, error)) {
        cout << "\n  FAILED TO START: " << error << "\n\n";
        return 1;
    }

    cout << "  dashboard : http://127.0.0.1:" << server.getPort()
         << "/\n"
         << "  api       : http://127.0.0.1:" << server.getPort()
         << "/api/health\n"
         << "  frontend  : " << frontendDir << "/\n"
         << "\n  Open the dashboard URL in a browser."
         << " Press Ctrl+C to stop.\n\n";

    server.run();

    cout << "\n  Shutting down. Requests served: "
         << handler.getRequestCount()
         << " (" << handler.getErrorCount() << " errors)\n";

    system.shutdown();

    return 0;
}