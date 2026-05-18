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

// ── Keyword Completion Items with Documentation ─────────────────────────────
struct CompletionEntry {
    std::string label;       // Tamil keyword
    std::string detail;      // Short English label
    std::string doc;         // Rich documentation (Tamil + English)
    int kind;                // LSP CompletionItemKind
    std::string insertText;  // Snippet to insert
    int insertTextFormat;    // 1 = PlainText, 2 = Snippet
};

// LSP CompletionItemKind values
constexpr int KIND_KEYWORD  = 14;
constexpr int KIND_FUNCTION = 3;
constexpr int KIND_VARIABLE = 6;
constexpr int KIND_CLASS    = 7;
constexpr int KIND_ENUM     = 13;
constexpr int KIND_MODULE   = 9;
constexpr int KIND_TYPE     = 25;
constexpr int KIND_CONSTANT = 21;
constexpr int KIND_SNIPPET  = 15;

static std::vector<CompletionEntry> get_completions() {
    return {
        // ── Control Flow ─────────────────────────────────────────────
        {
            "செயல்", "Function Declaration",
            "### செயல் (Function)\n\n"
            "Declares a new function.\n\n"
            "```agam\n"
            "செயல் கூட்டல்(அ: எண், ஆ: எண்): எண் {\n"
            "    விடை அ + ஆ;\n"
            "}\n"
            "```\n\n"
            "**English:** `func` — Defines a reusable block of code with parameters and a return type.",
            KIND_KEYWORD,
            "செயல் ${1:பெயர்}(${2:}): ${3:எண்} {\n    ${0}\n}",
            2
        },
        {
            "மாறி", "Variable Declaration",
            "### மாறி (Variable)\n\n"
            "Declares a new variable with a type annotation.\n\n"
            "```agam\n"
            "மாறி வயது: எண் = 25;\n"
            "மாறி பெயர்: சரம் = \"தமிழ்\";\n"
            "```\n\n"
            "**English:** `let` — Creates a new variable binding.",
            KIND_VARIABLE,
            "மாறி ${1:பெயர்}: ${2:எண்} = ${0};",
            2
        },
        {
            "நிலைமாறிலி", "Constant Modifier",
            "### நிலைமாறிலி (Constant)\n\n"
            "Marks a variable as immutable (constant).\n\n"
            "```agam\n"
            "மாறி நிலைமாறிலி பை: தசமம்64 = 3.14159;\n"
            "```\n\n"
            "**English:** `const` — The value cannot be changed after initialization.",
            KIND_CONSTANT,
            "நிலைமாறிலி",
            1
        },
        {
            "எனில்", "If Condition",
            "### எனில் (If)\n\n"
            "Conditional branching statement.\n\n"
            "```agam\n"
            "எனில் (மதிப்பு > 10) {\n"
            "    பதிப்பி(\"பெரியது\");\n"
            "}\n"
            "```\n\n"
            "**English:** `if` — Executes the block only when the condition is true.",
            KIND_KEYWORD,
            "எனில் (${1:நிபந்தனை}) {\n    ${0}\n}",
            2
        },
        {
            "இல்லையெனில்", "Else Clause",
            "### இல்லையெனில் (Else)\n\n"
            "Alternative branch for an `எனில்` block.\n\n"
            "```agam\n"
            "எனில் (x > 0) {\n"
            "    பதிப்பி(\"நேர்மறை\");\n"
            "} இல்லையெனில் {\n"
            "    பதிப்பி(\"எதிர்மறை\");\n"
            "}\n"
            "```\n\n"
            "**English:** `else` — Executes when the preceding `if` condition is false.",
            KIND_KEYWORD,
            "இல்லையெனில் {\n    ${0}\n}",
            2
        },
        {
            "வரை", "While Loop",
            "### வரை (While)\n\n"
            "Repeats a block while the condition remains true.\n\n"
            "```agam\n"
            "மாறி நிலை i: எண் = 0;\n"
            "வரை (i < 10) {\n"
            "    பதிப்பி(i);\n"
            "    i = i + 1;\n"
            "}\n"
            "```\n\n"
            "**English:** `while` — Loop that runs until the condition becomes false.",
            KIND_KEYWORD,
            "வரை (${1:நிபந்தனை}) {\n    ${0}\n}",
            2
        },
        {
            "சுற்று", "For Loop",
            "### சுற்று (For)\n\n"
            "Range-based for loop.\n\n"
            "```agam\n"
            "சுற்று (i உள் 10) {\n"
            "    பதிப்பி(i);\n"
            "}\n"
            "```\n\n"
            "**English:** `for` — Iterates over a range of values.",
            KIND_KEYWORD,
            "சுற்று (${1:i} உள் ${2:10}) {\n    ${0}\n}",
            2
        },
        {
            "விடை", "Return Statement",
            "### விடை (Return)\n\n"
            "Returns a value from a function.\n\n"
            "```agam\n"
            "செயல் இரட்டை(x: எண்): எண் {\n"
            "    விடை x * 2;\n"
            "}\n"
            "```\n\n"
            "**English:** `return` — Exits the function and returns the specified value.",
            KIND_KEYWORD,
            "விடை ${0};",
            2
        },
        {
            "உள்", "In (Range Iterator)",
            "### உள் (In)\n\n"
            "Used with `சுற்று` to iterate over a range.\n\n"
            "```agam\n"
            "சுற்று (i உள் 5) { ... }\n"
            "```\n\n"
            "**English:** `in` — Specifies the range to iterate over.",
            KIND_KEYWORD,
            "உள்",
            1
        },

        // ── Types ────────────────────────────────────────────────────
        {
            "எண்", "Integer Type (int64)",
            "### எண் (Integer)\n\n"
            "64-bit signed integer type. Default integer type in Agam.\n\n"
            "```agam\n"
            "மாறி வயது: எண் = 25;\n"
            "```\n\n"
            "**Aliases:** எண்8, எண்16, எண்32, எண்64, எண்128\n\n"
            "**English:** `int` — Stores whole numbers.",
            KIND_TYPE,
            "எண்",
            1
        },
        {
            "தசமம்", "Float Type (float64)",
            "### தசமம் (Float)\n\n"
            "64-bit floating-point number type.\n\n"
            "```agam\n"
            "மாறி பை: தசமம் = 3.14;\n"
            "```\n\n"
            "**Aliases:** தசமம்32, தசமம்64\n\n"
            "**English:** `float` — Stores decimal numbers.",
            KIND_TYPE,
            "தசமம்",
            1
        },
        {
            "தசமம்32", "32-bit Float", "### தசமம்32\n\n32-bit floating-point (single precision).", KIND_TYPE, "தசமம்32", 1
        },
        {
            "தசமம்64", "64-bit Float", "### தசமம்64\n\n64-bit floating-point (double precision).", KIND_TYPE, "தசமம்64", 1
        },
        {
            "சரம்", "String Type",
            "### சரம் (String)\n\n"
            "UTF-8 string type for text data.\n\n"
            "```agam\n"
            "மாறி பெயர்: சரம் = \"வணக்கம்\";\n"
            "```\n\n"
            "**English:** `string` — Stores text data.",
            KIND_TYPE,
            "சரம்",
            1
        },
        {
            "மெய்மை", "Boolean Type",
            "### மெய்மை (Bool)\n\n"
            "Boolean type — can be `உண்மை` (true) or `பொய்` (false).\n\n"
            "```agam\n"
            "மாறி செயலில்: மெய்மை = உண்மை;\n"
            "```\n\n"
            "**English:** `bool` — Stores true/false values.",
            KIND_TYPE,
            "மெய்மை",
            1
        },
        {
            "வெற்று", "Void Type",
            "### வெற்று (Void)\n\n"
            "Represents the absence of a return value.\n\n"
            "```agam\n"
            "செயல் காட்டு(): வெற்று { ... }\n"
            "```\n\n"
            "**English:** `void` — Function returns nothing.",
            KIND_TYPE,
            "வெற்று",
            1
        },

        // ── Constants ────────────────────────────────────────────────
        {
            "உண்மை", "Boolean True",
            "### உண்மை (True)\n\nBoolean literal representing `true`.",
            KIND_CONSTANT, "உண்மை", 1
        },
        {
            "பொய்", "Boolean False",
            "### பொய் (False)\n\nBoolean literal representing `false`.",
            KIND_CONSTANT, "பொய்", 1
        },
        {
            "இல்லை", "Null / Nil",
            "### இல்லை (Nil)\n\nNull pointer literal.\n\n**English:** `nil` — Represents an empty or uninitialized pointer.",
            KIND_CONSTANT, "இல்லை", 1
        },

        // ── Data Structures ──────────────────────────────────────────
        {
            "அமைப்பு", "Struct Declaration",
            "### அமைப்பு (Struct)\n\n"
            "Defines a new data structure with named fields.\n\n"
            "```agam\n"
            "அமைப்பு புள்ளி {\n"
            "    x: எண்,\n"
            "    y: எண்,\n"
            "}\n"
            "```\n\n"
            "**English:** `struct` — Groups related data together.",
            KIND_CLASS,
            "அமைப்பு ${1:பெயர்} {\n    ${0}\n}",
            2
        },
        {
            "பட்டியல்", "Enum Declaration",
            "### பட்டியல் (Enum)\n\n"
            "Defines an enumeration with named variants.\n\n"
            "```agam\n"
            "பட்டியல் நிறம் {\n"
            "    சிவப்பு,\n"
            "    பச்சை,\n"
            "    நீலம்,\n"
            "}\n"
            "```\n\n"
            "**English:** `enum` — Defines a set of named constants.",
            KIND_ENUM,
            "பட்டியல் ${1:பெயர்} {\n    ${0}\n}",
            2
        },
        {
            "பொருத்து", "Match Expression",
            "### பொருத்து (Match)\n\n"
            "Pattern matching on enums or values.\n\n"
            "```agam\n"
            "பொருத்து (நிறம்) {\n"
            "    சிவப்பு => பதிப்பி(\"red\"),\n"
            "    பச்சை => பதிப்பி(\"green\"),\n"
            "}\n"
            "```\n\n"
            "**English:** `match` — Exhaustive pattern matching.",
            KIND_KEYWORD,
            "பொருத்து (${1:மதிப்பு}) {\n    ${0}\n}",
            2
        },
        {
            "பண்பு", "Trait Declaration",
            "### பண்பு (Trait)\n\n"
            "Defines a set of methods that a type must implement.\n\n"
            "```agam\n"
            "பண்பு காட்டக்கூடிய {\n"
            "    செயல் காட்டு(சுய): வெற்று;\n"
            "}\n"
            "```\n\n"
            "**English:** `trait` — Interface for shared behavior.",
            KIND_CLASS,
            "பண்பு ${1:பெயர்} {\n    ${0}\n}",
            2
        },
        {
            "செயல்படுத்து", "Impl Block",
            "### செயல்படுத்து (Impl)\n\n"
            "Implements methods on a struct or trait.\n\n"
            "```agam\n"
            "செயல்படுத்து புள்ளி {\n"
            "    செயல் நகர்(சுய, dx: எண்): வெற்று { ... }\n"
            "}\n"
            "```\n\n"
            "**English:** `impl` — Attach methods to a type.",
            KIND_KEYWORD,
            "செயல்படுத்து ${1:வகை} {\n    ${0}\n}",
            2
        },

        // ── Memory Management (ZPM) ──────────────────────────────────
        {
            "மண்டலம்", "ZPM Zone Block",
            "### மண்டலம் (Zone)\n\n"
            "Creates a memory zone for the Zone-Pulse Memory (ZPM) model.\n"
            "All allocations within a zone are automatically freed when the zone exits.\n\n"
            "```agam\n"
            "மண்டலம் A {\n"
            "    மாறி p = ஒதுக்கீடு<எண்>(10);\n"
            "}\n"
            "```\n\n"
            "**English:** `zone` — Scoped memory region for automatic deallocation.",
            KIND_MODULE,
            "மண்டலம் ${1:A} {\n    ${0}\n}",
            2
        },
        {
            "ஒதுக்கீடு", "ZPM Allocation",
            "### ஒதுக்கீடு (Alloc)\n\n"
            "Allocates memory within the current zone.\n\n"
            "```agam\n"
            "மாறி p = ஒதுக்கீடு<எண்>(5);\n"
            "```\n\n"
            "**English:** `alloc` — Allocate memory in the active zone.",
            KIND_FUNCTION,
            "ஒதுக்கீடு<${1:எண்}>(${0})",
            2
        },
        {
            "கடன்", "ZPM Borrow",
            "### கடன் (Borrow)\n\n"
            "Borrows a reference to data (shared or mutable).\n\n"
            "```agam\n"
            "கடன் பகிர்வு(x);\n"
            "கடன் மாற்றக்கூடிய(x);\n"
            "```\n\n"
            "**English:** `borrow` — Creates a safe reference without ownership transfer.",
            KIND_KEYWORD,
            "கடன் ${1|பகிர்வு,மாற்றக்கூடிய|}(${0})",
            2
        },
        {
            "பகிர்வு", "Shared Borrow",
            "### பகிர்வு (Shared)\n\n"
            "Read-only borrow. Multiple shared borrows are allowed simultaneously.\n\n"
            "**English:** `shared` — Immutable reference.",
            KIND_KEYWORD,
            "பகிர்வு",
            1
        },
        {
            "தப்பித்தல்", "ZPM Escape",
            "### தப்பித்தல் (Escape)\n\n"
            "Moves data out of a zone before it is deallocated.\n\n"
            "```agam\n"
            "தப்பித்தல்(p) -> B;\n"
            "```\n\n"
            "**English:** `escape` — Transfer ownership to an outer zone.",
            KIND_KEYWORD,
            "தப்பித்தல்(${1}) -> ${0}",
            2
        },
        {
            "புதிய", "Heap Allocation",
            "### புதிய (New)\n\n"
            "Allocates memory on the heap.\n\n"
            "```agam\n"
            "மாறி p = புதிய எண்;\n"
            "```\n\n"
            "**English:** `new` — Dynamic heap allocation.",
            KIND_KEYWORD,
            "புதிய ${0}",
            2
        },
        {
            "நீக்கு", "Delete / Free",
            "### நீக்கு (Delete)\n\n"
            "Frees heap-allocated memory.\n\n"
            "```agam\n"
            "நீக்கு p;\n"
            "```\n\n"
            "**English:** `delete` — Deallocates heap memory.",
            KIND_KEYWORD,
            "நீக்கு ${0};",
            2
        },

        // ── Modules ──────────────────────────────────────────────────
        {
            "இறக்குமதி", "Import Module",
            "### இறக்குமதி (Import)\n\n"
            "Imports an external module or file.\n\n"
            "```agam\n"
            "இறக்குமதி \"io.agam\";\n"
            "```\n\n"
            "**English:** `import` — Loads definitions from another file.",
            KIND_MODULE,
            "இறக்குமதி \"${0}\";",
            2
        },
        {
            "வெளி", "Extern (FFI)",
            "### வெளி (Extern)\n\n"
            "Declares an external C/FFI function.\n\n"
            "```agam\n"
            "வெளி செயல் printf(fmt: சரம்): எண்;\n"
            "```\n\n"
            "**English:** `extern` — Foreign function interface declaration.",
            KIND_KEYWORD,
            "வெளி செயல் ${1:பெயர்}(${2}): ${0};",
            2
        },

        // ── Other ────────────────────────────────────────────────────
        {
            "ஆக", "Type Cast",
            "### ஆக (As)\n\n"
            "Casts a value to a different type.\n\n"
            "```agam\n"
            "மாறி f: தசமம்64 = x ஆக தசமம்64;\n"
            "```\n\n"
            "**English:** `as` — Explicit type conversion.",
            KIND_KEYWORD,
            "ஆக ${0}",
            2
        },
        {
            "நிலை", "Mutable Modifier",
            "### நிலை (Mut)\n\n"
            "Marks a variable or pointer as mutable.\n\n"
            "```agam\n"
            "மாறி நிலை x: எண் = 10;\n"
            "```\n\n"
            "**English:** `mut` — Allows the value to be modified after declaration.",
            KIND_KEYWORD,
            "நிலை",
            1
        },

        // ── Built-in Functions ───────────────────────────────────────
        {
            "பதிப்பி", "Print Function",
            "### பதிப்பி (Print)\n\n"
            "Prints a value to the console with a newline.\n\n"
            "```agam\n"
            "பதிப்பி(\"வணக்கம் உலகம்!\");\n"
            "பதிப்பி(42);\n"
            "```\n\n"
            "**English:** `print` — Outputs text to standard output.",
            KIND_FUNCTION,
            "பதிப்பி(${0});",
            2
        },
        {
            "மைய", "Main Function (Entry Point)",
            "### மைய (Main)\n\n"
            "The program entry point. Every Agam program must have a `மைய` function.\n\n"
            "```agam\n"
            "செயல் மைய(): எண் {\n"
            "    பதிப்பி(\"வணக்கம்!\");\n"
            "    விடை 0;\n"
            "}\n"
            "```\n\n"
            "**English:** `main` — The first function called when the program starts.",
            KIND_FUNCTION,
            "செயல் மைய(): எண் {\n    ${0}\n    விடை 0;\n}",
            2
        },
    };
}

// ── LSP Handlers ────────────────────────────────────────────────────────────

void send_response(const json& msg) {
    std::string response_str = msg.dump();
    std::cout << "Content-Length: " << response_str.length() << "\r\n\r\n" << response_str << std::flush;
}

void handle_initialize(const json& request, json& response) {
    response["result"] = {
        {"capabilities", {
            {"textDocumentSync", 1}, // Full sync
            {"hoverProvider", true},
            {"completionProvider", {
                {"resolveProvider", false},
                {"triggerCharacters", json::array()}
            }}
        }},
        {"serverInfo", {
            {"name", "agam-lsp"},
            {"version", "1.0.1"}
        }}
    };
}

void handle_completion(const json& request, json& response) {
    json items = json::array();

    for (const auto& entry : get_completions()) {
        json item = {
            {"label", entry.label},
            {"kind", entry.kind},
            {"detail", entry.detail},
            {"documentation", {
                {"kind", "markdown"},
                {"value", entry.doc}
            }},
            {"insertText", entry.insertText},
            {"insertTextFormat", entry.insertTextFormat}
        };
        items.push_back(item);
    }

    response["result"] = items;
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
        int severity = 1; // Error
        if (d.level == agam::DiagnosticLevel::Warning) severity = 2;
        else if (d.level == agam::DiagnosticLevel::Note) severity = 3;

        int line = d.loc.line > 0 ? d.loc.line - 1 : 0;
        int col = d.loc.column > 0 ? d.loc.column - 1 : 0;

        json diag_json = {
            {"range", {
                {"start", {{"line", line}, {"character", col}}},
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

    send_response(notification);
}

void process_message(const std::string& message) {
    try {
        json request = json::parse(message);

        if (!request.contains("method")) return;

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
            return;
        } else if (method == "textDocument/didChange") {
            std::string uri = request["params"]["textDocument"]["uri"];
            std::string text = request["params"]["contentChanges"][0]["text"];
            publish_diagnostics(uri, text);
            return;
        } else if (method == "textDocument/completion") {
            handle_completion(request, response);
        } else {
            if (request.contains("id")) {
                response["error"] = {
                    {"code", -32601},
                    {"message", "Method not found"}
                };
            } else {
                return;
            }
        }

        if (response.contains("id")) {
            send_response(response);
        }
    } catch (const std::exception& e) {
        std::cerr << "LSP Error: " << e.what() << std::endl;
    }
}

int main() {
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

            // Consume the empty line before payload
            std::getline(std::cin, line);

            std::vector<char> buffer(content_length);
            std::cin.read(buffer.data(), content_length);
            std::string payload(buffer.begin(), buffer.end());

            process_message(payload);
        }
    }
    return 0;
}
