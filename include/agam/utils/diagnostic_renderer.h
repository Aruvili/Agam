#pragma once

#include "agam/utils/diagnostic.h"
#include <iostream>

namespace agam {

/// Renders diagnostics to various formats.
class DiagnosticRenderer {
public:
    /// Render all diagnostics to the specified output stream (default: std::cerr).
    static void renderToTerminal(const DiagnosticEngine &engine, std::ostream &os = std::cerr);

    /// Render all diagnostics as a JSON array to the specified output stream.
    static void renderToJSON(const DiagnosticEngine &engine, std::ostream &os = std::cout);

private:
    static void renderSnippet(const DiagnosticEngine &engine, const Diagnostic &diag, std::ostream &os);
    static std::string colorize(DiagnosticLevel level, const std::string &text);
};

} // namespace agam
