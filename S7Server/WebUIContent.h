#pragma once

#include <string>

/*
 * WebUIContent - Embedded HTML/CSS/JavaScript for web UI
 * 
 * Contains the entire single-page web application embedded as a string literal.
 * Served at the root path (/) by the HTTP server.
 */

class WebUIContent {
public:
    // Get the complete HTML content for the web UI
    static std::string getHTML();
};
