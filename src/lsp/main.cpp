#include <iostream>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

#include "agam/lexer/lexer.h"
#include "agam/parser/parser.h"
#include "agam/utils/diagnostic.h"

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

using json = nlohmann::json;

void handle_initialize(const json& request, json& response) {
    response["result"] = {
        {"capabilities", {
            {"textDocumentSync", 1},
            {"hoverProvider", true}
        }},
        {"serverInfo", {
            {"name", "agam-lsp"},
            {"version", "1.0.1"}
        }}
    };
}

void publish_diagnostics(const std::string& uri, const std::string& text) {
    agam::SourceManager sm;
    sm.addSource(uri, text);
    agam::DiagnosticEngine diag(sm);
    
    // Run lexer
    agam::Lexer lexer(text, uri, diag);
    auto tokens = lexer.tokenize();
    
    // Run parser
    agam::Parser parser(tokens, text, uri, diag);
    auto ast = parser.parse();

    // Map Agam Diagnostics to LSP Diagnostics
    json diagnostics_array = json::array();
    for (const auto& d : diag.diagnostics()) {
        int severity = 1; // LSP 1 = Error
        if (d.level == agam::DiagnosticLevel::Warning) severity = 2; // Warning
        else if (d.level == agam::DiagnosticLevel::Note) severity = 3; // Information

        // LSP lines and columns are 0-indexed
        int line = d.loc.line > 0 ? d.loc.line - 1 : 0;
        int col = d.loc.column > 0 ? d.loc.column - 1 : 0;

        json diag_json = {
            {"range", {
                {"start", {{"line", line}, {"character", col}}},
                // Highlight at least 1 character. 
                // We could inspect the tokens for exact length later.
                {"end", {{"line", line}, {"character", col + 1}}} 
            }},
            {"severity", severity},
            {"source", "agam"},
            {"message", d.message + (d.hint.empty() ? "" : "\nHint: " + d.hint)}
        };
        diagnostics_array.push_back(diag_json);
    }

    json notification = {
        {"jsonrpc", "2.0"},
        {"method", "textDocument/publishDiagnostics"},
        {"params", {
            {"uri", uri},
            {"diagnostics", diagnostics_array}
        }}
    };

    std::string response_str = notification.dump();
    std::cout << "Content-Length: " << response_str.length() << "\r\n\r\n" << response_str << std::flush;
}

void process_message(const std::string& message) {
    try {
        json request = json::parse(message);
        
        // Ignore responses (where there is no method)
        if (!request.contains("method")) return;

        // Setup base response
        json response;
        response["jsonrpc"] = "2.0";
        if (request.contains("id")) {
            response["id"] = request["id"];
        }

        std::string method = request["method"].get<std::string>();
        
        if (method == "initialize") {
            handle_initialize(request, response);
        } else if (method == "initialized") {
            return;
        } else if (method == "shutdown") {
            response["result"] = nullptr;
        } else if (method == "exit") {
            std::exit(0);
        } else if (method == "textDocument/didOpen") {
            std::string uri = request["params"]["textDocument"]["uri"];
            std::string text = request["params"]["textDocument"]["text"];
            publish_diagnostics(uri, text);
            return; // Notification, no response ID
        } else if (method == "textDocument/didChange") {
            std::string uri = request["params"]["textDocument"]["uri"];
            // We use full sync mode (textDocumentSync = 1) so text is always the full document
            std::string text = request["params"]["contentChanges"][0]["text"];
            publish_diagnostics(uri, text);
            return; // Notification, no response ID
        } else {
            // Handle unimplemented requests silently
            if (request.contains("id")) {
                response["error"] = {
                    {"code", -32601},
                    {"message", "Method not found"}
                };
            } else {
                return; // Ignore unhandled notifications
            }
        }

        // Only send response if we actually set an ID or Error
        if (response.contains("id")) {
            std::string response_str = response.dump();
            std::cout << "Content-Length: " << response_str.length() << "\r\n\r\n" << response_str << std::flush;
        }
    } catch (const std::exception& e) {
        std::cerr << "LSP Error: " << e.what() << std::endl;
    }
}

int main() {
    // LSP requires binary I/O for precise content length tracking
#ifdef _WIN32
    _setmode(_fileno(stdin), _O_BINARY);
    _setmode(_fileno(stdout), _O_BINARY);
#endif

    std::string line;
    while (std::getline(std::cin, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        if (line.rfind("Content-Length: ", 0) == 0) {
            int content_length = std::stoi(line.substr(16));
            
            // Consume the empty line (\r\n) before payload
            std::getline(std::cin, line);
            
            std::vector<char> buffer(content_length);
            std::cin.read(buffer.data(), content_length);
            std::string payload(buffer.begin(), buffer.end());
            
            process_message(payload);
        }
    }
    return 0;
}
