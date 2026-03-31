#pragma once

#include <string>
#include <memory>
#include <thread>
#include <atomic>
#include <functional>

/*
 * HttpServer - Wrapper for cpp-httplib HTTP server
 * 
 * Manages the lifecycle of an embedded HTTP server running on a separate thread.
 * Provides methods to register API endpoints and serve static content.
 */

// Forward declaration to avoid including httplib.h in header
namespace httplib {
    class Server;
}

class HttpServer {
public:
    HttpServer(int port = 8080);
    ~HttpServer();

    // Start the HTTP server on a background thread
    bool start();

    // Stop the HTTP server gracefully
    void stop();

    // Check if server is running
    bool isRunning() const;

    // Get the port number
    int getPort() const;

    // Register a GET endpoint with a handler function
    // Handler signature: void(const httplib::Request&, httplib::Response&)
    using GetHandler = std::function<void(const void*, void*)>;
    void registerGetEndpoint(const std::string& pattern, GetHandler handler);

    // Register a POST endpoint with a handler function
    using PostHandler = std::function<void(const void*, void*)>;
    void registerPostEndpoint(const std::string& pattern, PostHandler handler);

    // Get the underlying httplib::Server instance (for advanced usage)
    httplib::Server* getServer();

private:
    int m_port;
    std::unique_ptr<httplib::Server> m_server;
    std::unique_ptr<std::thread> m_serverThread;
    std::atomic<bool> m_running;

    // Thread function that runs the server
    void serverThreadFunc();
};
