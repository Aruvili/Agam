#pragma once

#include "agam/ast/ast.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace agam {

/// Level of a single diagnostic.
enum class DiagnosticLevel {
    Note,
    Warning,
    Error,
    Fatal
};

/// A single structured diagnostic.
struct Diagnostic {
    DiagnosticLevel level;
    SourceLocation loc;
    std::string message;
    std::string hint; // Optional "did you mean?" or suggestion
};

/// Manages source code content for snippet recovery in diagnostics.
class SourceManager {
public:
    /// Add a source file to the manager and split it into lines.
    void addSource(const std::string &filename, const std::string &content) {
        std::vector<std::string> lines;
        std::string line;
        for (char c : content) {
            if (c == '\n') {
                lines.push_back(line);
                line.clear();
            } else if (c != '\r') {
                line += c;
            }
        }
        if (!line.empty() || (!content.empty() && content.back() == '\n')) {
            lines.push_back(line);
        }
        sources_[filename] = std::move(lines);
    }

    /// Get a specific line from a source file (1-indexed).
    std::string getLine(const std::string &filename, int line) const {
        auto it = sources_.find(filename);
        if (it == sources_.end() || line <= 0 || line > (int)it->second.size()) {
            return "";
        }
        return it->second[line - 1];
    }

    /// Check if a filename is present.
    bool hasSource(const std::string &filename) const {
        return sources_.find(filename) != sources_.end();
    }

private:
    std::unordered_map<std::string, std::vector<std::string>> sources_;
};

/// Collects and manages diagnostics during compilation phases.
class DiagnosticEngine {
public:
    explicit DiagnosticEngine(SourceManager &sm) : sm_(sm) {}

    void report(DiagnosticLevel level, const SourceLocation &loc, const std::string &msg, const std::string &hint = "") {
        diagnostics_.push_back({level, loc, msg, hint});
        if (level == DiagnosticLevel::Error || level == DiagnosticLevel::Fatal) {
            hasErrors_ = true;
        }
    }

    // Convenience helpers
    void error(const SourceLocation &loc, const std::string &msg, const std::string &hint = "") {
        report(DiagnosticLevel::Error, loc, msg, hint);
    }

    void warning(const SourceLocation &loc, const std::string &msg, const std::string &hint = "") {
        report(DiagnosticLevel::Warning, loc, msg, hint);
    }

    void note(const SourceLocation &loc, const std::string &msg, const std::string &hint = "") {
        report(DiagnosticLevel::Note, loc, msg, hint);
    }

    const std::vector<Diagnostic> &diagnostics() const { return diagnostics_; }
    bool hasErrors() const { return hasErrors_; }
    SourceManager &sourceManager() const { return sm_; }

    void clear() {
        diagnostics_.clear();
        hasErrors_ = false;
    }

private:
    SourceManager &sm_;
    std::vector<Diagnostic> diagnostics_;
    bool hasErrors_ = false;
};

} // namespace agam
