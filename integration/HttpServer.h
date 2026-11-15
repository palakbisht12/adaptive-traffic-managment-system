
#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <string>
#include <map>
#include <vector>

using namespace std;

#ifdef _WIN32
#include <basetsd.h>
typedef UINT_PTR socket_handle_t;
#else
typedef int socket_handle_t;
#endif

struct HttpRequest {
    string method;
    string path;
    string query;
    string body;
    map<string, string> headers;

    string lastSegment() const;
    int segmentCount() const;
    string segment(int index) const;
    string queryParam(const string& key) const;
};

struct HttpResponse {
    int status = 200;
    string contentType = "application/json";
    string body;

    static HttpResponse json(int status, const string& body);
    static HttpResponse text(int status, const string& body);
    static HttpResponse file(const string& contentType,
                             const string& body);
};

typedef HttpResponse (*RequestCallback)(const HttpRequest&, void* context);

typedef void (*IdleCallback)(void* context);

class HttpServer {
private:
    socket_handle_t listenSocket;
    int port;
    bool running;
    bool initialised;
    string lastError;

    RequestCallback callback;
    void* callbackContext;

    IdleCallback idleCallback;
    void* idleContext;

    static bool platformInit(string& error);
    static void platformShutdown();
    static void closeSocket(socket_handle_t sock);

    bool readRequest(socket_handle_t client, HttpRequest& request);
    void sendResponse(socket_handle_t client,
                     const HttpResponse& response);

public:
    HttpServer();
    ~HttpServer();

    HttpServer(const HttpServer&) = delete;
    HttpServer& operator=(const HttpServer&) = delete;

    void setHandler(RequestCallback handler, void* context);
    void setIdleHandler(IdleCallback handler, void* context);

    bool start(int listenPort, string& error);
    void run();
    void runFor(int maxRequests);
    void stop();

    bool isRunning() const;
    int getPort() const;
    string getLastError() const;

    static string statusText(int status);
    static string mimeTypeFor(const string& path);
    static string urlDecode(const string& text);
};

#endif
