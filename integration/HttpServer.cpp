
#include "HttpServer.h"

#include <sstream>
#include <cstring>
#include <cstdlib>
#include <cctype>

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef int socklen_t;
    #define CLOSE_SOCKET(s) closesocket(s)
    #define BAD_SOCKET INVALID_SOCKET
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #define CLOSE_SOCKET(s) ::close(s)
    #define BAD_SOCKET (-1)
#endif

using namespace std;

int HttpRequest::segmentCount() const {
    int count = 0;
    size_t i = 0;

    while (i < path.size()) {
        while (i < path.size() && path[i] == '/') {
            i++;
        }

        if (i >= path.size()) {
            break;
        }

        count++;

        while (i < path.size() && path[i] != '/') {
            i++;
        }
    }

    return count;
}

string HttpRequest::segment(int index) const {
    int current = 0;
    size_t i = 0;

    while (i < path.size()) {
        while (i < path.size() && path[i] == '/') {
            i++;
        }

        if (i >= path.size()) {
            break;
        }

        const size_t start = i;

        while (i < path.size() && path[i] != '/') {
            i++;
        }

        if (current == index) {
            return path.substr(start, i - start);
        }

        current++;
    }

    return "";
}

string HttpRequest::lastSegment() const {
    const int n = segmentCount();
    return (n == 0) ? string() : segment(n - 1);
}

string HttpRequest::queryParam(const string& key) const {
    size_t i = 0;

    while (i < query.size()) {
        const size_t amp = query.find('&', i);

        const string pair = query.substr(
            i,
            amp == string::npos ? string::npos : amp - i
        );

        const size_t eq = pair.find('=');

        if (eq != string::npos && pair.substr(0, eq) == key) {
            return HttpServer::urlDecode(pair.substr(eq + 1));
        }

        if (amp == string::npos) {
            break;
        }

        i = amp + 1;
    }

    return "";
}

HttpResponse HttpResponse::json(int status, const string& body) {
    HttpResponse r;
    r.status = status;
    r.contentType = "application/json";
    r.body = body;
    return r;
}

HttpResponse HttpResponse::text(int status, const string& body) {
    HttpResponse r;
    r.status = status;
    r.contentType = "text/plain; charset=utf-8";
    r.body = body;
    return r;
}

HttpResponse HttpResponse::file(const string& contentType,
                                const string& body) {
    HttpResponse r;
    r.status = 200;
    r.contentType = contentType;
    r.body = body;
    return r;
}

HttpServer::HttpServer() {
    listenSocket = BAD_SOCKET;
    port = 0;
    running = false;
    initialised = false;
    callback = 0;
    callbackContext = 0;
    idleCallback = 0;
    idleContext = 0;
}

HttpServer::~HttpServer() {
    stop();

    if (initialised) {
        platformShutdown();
    }
}

bool HttpServer::platformInit(string& error) {
#ifdef _WIN32
    WSADATA data;
    const int rc = WSAStartup(MAKEWORD(2, 2), &data);

    if (rc != 0) {
        ostringstream msg;
        msg << "WSAStartup failed with code " << rc;
        error = msg.str();
        return false;
    }
#else
    (void)error;
#endif

    return true;
}

void HttpServer::platformShutdown() {
#ifdef _WIN32
    WSACleanup();
#endif
}

void HttpServer::closeSocket(socket_handle_t sock) {
    if (sock != BAD_SOCKET) {
        CLOSE_SOCKET(sock);
    }
}

void HttpServer::setHandler(RequestCallback handler, void* context) {
    callback = handler;
    callbackContext = context;
}

void HttpServer::setIdleHandler(IdleCallback handler, void* context) {
    idleCallback = handler;
    idleContext = context;
}

bool HttpServer::start(int listenPort, string& error) {
    if (!initialised) {
        if (!platformInit(error)) {
            return false;
        }

        initialised = true;
    }

    listenSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (listenSocket == BAD_SOCKET) {
        error = "Could not create socket";
        return false;
    }

    int reuse = 1;

    setsockopt(
        listenSocket,
        SOL_SOCKET,
        SO_REUSEADDR,
        (const char*)&reuse,
        sizeof(reuse)
    );

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));

    address.sin_family = AF_INET;
    address.sin_port = htons((unsigned short)listenPort);
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (bind(listenSocket,
             (struct sockaddr*)&address,
             sizeof(address)) != 0) {

        ostringstream msg;
        msg << "Port " << listenPort
            << " is already in use. Close the other program or pick "
               "another port.";

        error = msg.str();

        closeSocket(listenSocket);
        listenSocket = BAD_SOCKET;

        return false;
    }

    if (listen(listenSocket, 16) != 0) {
        error = "listen() failed";

        closeSocket(listenSocket);
        listenSocket = BAD_SOCKET;

        return false;
    }

    socklen_t addrLen = sizeof(address);

    if (getsockname(listenSocket,
                    (struct sockaddr*)&address,
                    &addrLen) == 0) {
        port = ntohs(address.sin_port);
    }
    else {
        port = listenPort;
    }

    running = true;
    lastError.clear();

    return true;
}

bool HttpServer::readRequest(socket_handle_t client,
                             HttpRequest& request) {
    string raw;
    char chunk[4096];

    size_t headerEnd = string::npos;

    while (headerEnd == string::npos) {
        const int got = (int)recv(client, chunk, sizeof(chunk), 0);

        if (got <= 0) {
            return false;
        }

        raw.append(chunk, (size_t)got);
        headerEnd = raw.find("\r\n\r\n");

        if (raw.size() > 1024 * 1024) {
            return false;
        }
    }

    const string head = raw.substr(0, headerEnd);
    string body = raw.substr(headerEnd + 4);

    istringstream lines(head);
    string line;

    if (!getline(lines, line)) {
        return false;
    }

    if (!line.empty() && line[line.size() - 1] == '\r') {
        line.erase(line.size() - 1);
    }

    {
        istringstream first(line);
        string target;

        first >> request.method >> target;

        const size_t q = target.find('?');

        if (q == string::npos) {
            request.path = target;
        }
        else {
            request.path = target.substr(0, q);
            request.query = target.substr(q + 1);
        }

        request.path = urlDecode(request.path);
    }

    size_t contentLength = 0;

    while (getline(lines, line)) {
        if (!line.empty() && line[line.size() - 1] == '\r') {
            line.erase(line.size() - 1);
        }

        if (line.empty()) {
            continue;
        }

        const size_t colon = line.find(':');

        if (colon == string::npos) {
            continue;
        }

        string name = line.substr(0, colon);
        string value = line.substr(colon + 1);

        while (!value.empty() &&
               (value[0] == ' ' || value[0] == '\t')) {
            value.erase(0, 1);
        }

        for (size_t i = 0; i < name.size(); i++) {
            name[i] = (char)tolower((unsigned char)name[i]);
        }

        request.headers[name] = value;

        if (name == "content-length") {
            contentLength = (size_t)atoi(value.c_str());
        }
    }

    while (body.size() < contentLength) {
        const int got = (int)recv(client, chunk, sizeof(chunk), 0);

        if (got <= 0) {
            break;
        }

        body.append(chunk, (size_t)got);
    }

    request.body = body.substr(0, contentLength);

    return true;
}

void HttpServer::sendResponse(socket_handle_t client,
                              const HttpResponse& response) {
    ostringstream out;

    out << "HTTP/1.1 " << response.status << " "
        << statusText(response.status) << "\r\n"
        << "Content-Type: " << response.contentType << "\r\n"
        << "Content-Length: " << response.body.size() << "\r\n"
        << "Access-Control-Allow-Origin: *\r\n"
        << "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n"
        << "Access-Control-Allow-Headers: Content-Type\r\n"
        << "Cache-Control: no-store\r\n"
        << "Connection: close\r\n"
        << "\r\n"
        << response.body;

    const string payload = out.str();
    size_t sent = 0;

    while (sent < payload.size()) {
        const int wrote = (int)send(
            client,
            payload.data() + sent,
            (int)(payload.size() - sent),
            0
        );

        if (wrote <= 0) {
            break;
        }

        sent += (size_t)wrote;
    }
}

void HttpServer::runFor(int maxRequests) {
    int served = 0;

    while (running && (maxRequests <= 0 || served < maxRequests)) {
        fd_set readable;

        FD_ZERO(&readable);
        FD_SET(listenSocket, &readable);

        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 50 * 1000;

        const int ready = select(
            (int)listenSocket + 1,
            &readable,
            0,
            0,
            &timeout
        );

        if (!running) {
            break;
        }

        if (ready <= 0) {
            if (idleCallback) {
                idleCallback(idleContext);
            }

            continue;
        }

        struct sockaddr_in peer;
        socklen_t peerLen = sizeof(peer);

        const socket_handle_t client = accept(
            listenSocket,
            (struct sockaddr*)&peer,
            &peerLen
        );

        if (client == BAD_SOCKET) {
            if (!running) {
                break;
            }

            continue;
        }

        HttpRequest request;

        if (readRequest(client, request)) {
            HttpResponse response;

            if (request.method == "OPTIONS") {
                response = HttpResponse::text(204, "");
            }
            else if (callback) {
                response = callback(request, callbackContext);
            }
            else {
                response = HttpResponse::text(
                    500,
                    "No handler installed"
                );
            }

            sendResponse(client, response);
        }

        closeSocket(client);
        served++;
    }
}

void HttpServer::run() {
    runFor(0);
}

void HttpServer::stop() {
    running = false;

    if (listenSocket != BAD_SOCKET) {
        closeSocket(listenSocket);
        listenSocket = BAD_SOCKET;
    }
}

bool HttpServer::isRunning() const {
    return running;
}

int HttpServer::getPort() const {
    return port;
}

string HttpServer::getLastError() const {
    return lastError;
}

string HttpServer::statusText(int status) {
    switch (status) {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 400: return "Bad Request";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 409: return "Conflict";
        case 500: return "Internal Server Error";
        case 503: return "Service Unavailable";
        default: return "Unknown";
    }
}

string HttpServer::mimeTypeFor(const string& path) {
    const size_t dot = path.rfind('.');

    if (dot == string::npos) {
        return "application/octet-stream";
    }

    const string ext = path.substr(dot);

    if (ext == ".html" || ext == ".htm") {
        return "text/html; charset=utf-8";
    }

    if (ext == ".css") {
        return "text/css; charset=utf-8";
    }

    if (ext == ".js") {
        return "application/javascript; charset=utf-8";
    }

    if (ext == ".json") {
        return "application/json";
    }

    if (ext == ".svg") {
        return "image/svg+xml";
    }

    if (ext == ".png") {
        return "image/png";
    }

    if (ext == ".ico") {
        return "image/x-icon";
    }

    return "text/plain; charset=utf-8";
}

string HttpServer::urlDecode(const string& text) {
    string out;
    out.reserve(text.size());

    for (size_t i = 0; i < text.size(); i++) {
        if (text[i] == '%' && i + 2 < text.size() &&
            isxdigit((unsigned char)text[i + 1]) &&
            isxdigit((unsigned char)text[i + 2])) {

            const string hex = text.substr(i + 1, 2);
            out += (char)strtol(hex.c_str(), 0, 16);
            i += 2;
        }
        else if (text[i] == '+') {
            out += ' ';
        }
        else {
            out += text[i];
        }
    }

    return out;
}
