#include "HttpServer.h"
#include "httplib/httplib.h"
#include <iostream>

HttpServer::HttpServer(int port)
    : m_port(port), m_running(false) {
    m_server = std::make_unique<httplib::Server>();
}

HttpServer::~HttpServer() {
    stop();
}

bool HttpServer::start() {
    if (m_running) {
        std::cerr << "HTTP server is already running" << std::endl;
        return false;
    }

    // Set server timeout settings
    m_server->set_read_timeout(5, 0);  // 5 seconds
    m_server->set_write_timeout(5, 0); // 5 seconds

    // Start server thread
    m_running = true;
    m_serverThread = std::make_unique<std::thread>(&HttpServer::serverThreadFunc, this);

    // Give server time to start
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    if (m_running) {
        std::cout << "HTTP server started on port " << m_port << std::endl;
        std::cout << "Access web UI at: http://localhost:" << m_port << std::endl;
        return true;
    }

    return false;
}

void HttpServer::stop() {
    if (!m_running) {
        return;
    }

    std::cout << "Stopping HTTP server..." << std::endl;
    m_running = false;

    // Stop the server
    if (m_server) {
        m_server->stop();
    }

    // Wait for thread to finish
    if (m_serverThread && m_serverThread->joinable()) {
        m_serverThread->join();
    }

    std::cout << "HTTP server stopped" << std::endl;
}

bool HttpServer::isRunning() const {
    return m_running;
}

int HttpServer::getPort() const {
    return m_port;
}

void HttpServer::registerGetEndpoint(const std::string& pattern, GetHandler handler) {
    m_server->Get(pattern, [handler](const httplib::Request& req, httplib::Response& res) {
        handler(&req, &res);
    });
}

void HttpServer::registerPostEndpoint(const std::string& pattern, PostHandler handler) {
    m_server->Post(pattern, [handler](const httplib::Request& req, httplib::Response& res) {
        handler(&req, &res);
    });
}

httplib::Server* HttpServer::getServer() {
    return m_server.get();
}

void HttpServer::serverThreadFunc() {
    try {
        std::cout << "HTTP server thread starting on port " << m_port << "..." << std::endl;
        
        // This call blocks until server is stopped
        bool success = m_server->listen("0.0.0.0", m_port);
        
        if (!success && m_running) {
            std::cerr << "ERROR: Failed to start HTTP server on port " << m_port << std::endl;
            std::cerr << "Port may be already in use or requires administrator privileges" << std::endl;
            m_running = false;
        }
    } catch (const std::exception& e) {
        std::cerr << "HTTP server thread exception: " << e.what() << std::endl;
        m_running = false;
    }
}
