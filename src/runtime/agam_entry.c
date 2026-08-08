// ═══════════════════════════════════════════════════════════════════════════════
//  agam_entry.c - Main Entrypoint for Standalone Compiled Agam Binaries
// ═══════════════════════════════════════════════════════════════════════════════

extern int மைய(void);

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    return மைய();
}
