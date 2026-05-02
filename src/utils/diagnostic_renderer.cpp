#include "agam/utils/diagnostic_renderer.h"

#include <iostream>
#include <sstream>

namespace agam {

#ifdef _WIN32
#include <windows.h>
#endif

// ANSI Color Codes
#define ANSI_RESET "\033[0m"
#define ANSI_BOLD "\033[1m"
#define ANSI_RED "\033[31m"
#define ANSI_GREEN "\033[32m"
#define ANSI_YELLOW "\033[33m"
#define ANSI_CYAN "\033[36m"
#define ANSI_WHITE "\033[37m"

void DiagnosticRenderer::renderToTerminal(const DiagnosticEngine &engine, std::ostream &os) {
#ifdef _WIN32
    // Enable ANSI escape codes on Windows Terminal
    HANDLE hOut = GetStdHandle(STD_ERROR_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
#endif

    for (const auto &diag : engine.diagnostics()) {
        std::string levelStr;
        std::string levelColor;
        switch (diag.level) {
        case DiagnosticLevel::Note:
            levelStr = "குறிப்பு (note)";
            levelColor = ANSI_CYAN;
            break;
        case DiagnosticLevel::Warning:
            levelStr = "எச்சரிக்கை (warning)";
            levelColor = ANSI_YELLOW;
            break;
        case DiagnosticLevel::Error:
            levelStr = "பிழை (error)";
            levelColor = ANSI_RED;
            break;
        case DiagnosticLevel::Fatal:
            levelStr = "முக்கிய பிழை (fatal)";
            levelColor = ANSI_RED;
            break;
        }

        os << ANSI_BOLD << diag.loc.filename << ":" << diag.loc.line << ":" << diag.loc.column
           << ": " << levelColor << levelStr << ": " << ANSI_RESET << ANSI_BOLD << diag.message
           << ANSI_RESET << "\n";

        renderSnippet(engine, diag, os);

        if (!diag.hint.empty()) {
            os << ANSI_BOLD << "  உதவிக்குறிப்பு (hint): " << ANSI_RESET << diag.hint << "\n";
        }
        os << "\n";
    }
}

void DiagnosticRenderer::renderSnippet(const DiagnosticEngine &engine, const Diagnostic &diag,
                                       std::ostream &os) {
    if (diag.loc.filename.empty())
        return;

    std::string line = engine.sourceManager().getLine(diag.loc.filename, diag.loc.line);
    if (line.empty())
        return;

    // Line number with some padding
    std::string lineNum = std::to_string(diag.loc.line);
    os << " " << lineNum << " | " << line << "\n";
    os << " " << std::string(lineNum.size(), ' ') << " | ";

    // Pointer marker
    for (int i = 1; i < diag.loc.column; ++i) {
        if (i <= (int)line.size() && line[i - 1] == '\t')
            os << "\t";
        else
            os << " ";
    }
    os << ANSI_BOLD << ANSI_RED << "^" << ANSI_RESET << "\n";
}

void DiagnosticRenderer::renderToJSON(const DiagnosticEngine &engine, std::ostream &os) {
    os << "[\n";
    for (size_t i = 0; i < engine.diagnostics().size(); ++i) {
        const auto &d = engine.diagnostics()[i];

        os << "  {\n";
        os << "    \"level\": \""
           << (d.level == DiagnosticLevel::Error     ? "error"
               : d.level == DiagnosticLevel::Warning ? "warning"
                                                     : "note")
           << "\",\n";
        os << "    \"filename\": \"" << d.loc.filename << "\",\n";
        os << "    \"line\": " << d.loc.line << ",\n";
        os << "    \"column\": " << d.loc.column << ",\n";
        os << "    \"message\": \"" << d.message << "\",\n";
        os << "    \"hint\": \"" << d.hint << "\"\n";
        os << "  }" << (i < engine.diagnostics().size() - 1 ? "," : "") << "\n";
    }
    os << "]\n";
}

} // namespace agam
