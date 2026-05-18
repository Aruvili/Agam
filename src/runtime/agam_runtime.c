#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

// ── ZPM Runtime (High-Performance Linear Allocator) ──────────────────────────

#define ZPM_BLOCK_SIZE (8 * 1024 * 1024) // 8MB Blocks

typedef struct AgamZoneBlock {
    uint8_t* data;
    size_t used;
    size_t size;
    struct AgamZoneBlock* next;
} AgamZoneBlock;

typedef struct AgamZone {
    AgamZoneBlock* head;
    AgamZoneBlock* current;
} AgamZone;

void* agam_zone_create() {
    AgamZone* zone = (AgamZone*)malloc(sizeof(AgamZone));
    AgamZoneBlock* block = (AgamZoneBlock*)malloc(sizeof(AgamZoneBlock));
    block->data = (uint8_t*)malloc(ZPM_BLOCK_SIZE);
    block->used = 0;
    block->size = ZPM_BLOCK_SIZE;
    block->next = NULL;
    zone->head = block;
    zone->current = block;
    return zone;
}

void* agam_zone_alloc(void* zonePtr, size_t size) {
    if (!zonePtr) return malloc(size);
    AgamZone* zone = (AgamZone*)zonePtr;
    
    // Aligmnent padding (8-byte)
    size = (size + 7) & ~7;

    if (zone->current->used + size > zone->current->size) {
        // Current block full, allocate new
        size_t newSize = size > ZPM_BLOCK_SIZE ? size : ZPM_BLOCK_SIZE;
        AgamZoneBlock* block = (AgamZoneBlock*)malloc(sizeof(AgamZoneBlock));
        block->data = (uint8_t*)malloc(newSize);
        block->used = 0;
        block->size = newSize;
        block->next = zone->head;
        zone->head = block;
        zone->current = block;
    }

    void* ptr = zone->current->data + zone->current->used;
    zone->current->used += size;
    return ptr;
}

void agam_zone_destroy(void* zonePtr) {
    if (!zonePtr) return;
    AgamZone* zone = (AgamZone*)zonePtr;
    AgamZoneBlock* block = zone->head;
    while (block) {
        AgamZoneBlock* next = block->next;
        free(block->data);
        free(block);
        block = next;
    }
    free(zone);
}

void print_int(int32_t i) {
    printf("%d\n", i);
}

// ── Standard Library Helpers ──────────

int printf_int(const char* fmt, int64_t i) {
    return printf(fmt, i);
}

int printf_float(const char* fmt, double f) {
    return printf(fmt, f);
}

int printf_hex(const char* fmt, int64_t i) {
    return printf(fmt, i);
}

void printf_bin(int64_t i) {
    printf("0b");
    if (i == 0) { printf("0"); return; }
    int started = 0;
    for (int b = 63; b >= 0; b--) {
        if ((i >> b) & 1) started = 1;
        if (started) printf("%d", (int)((i >> b) & 1));
    }
}

int agam_putchar(int c) {
    return putchar(c);
}

int agam_getchar() {
    return getchar();
}

char* agam_readline() {
    char* buf = (char*)malloc(1024);
    if (!fgets(buf, 1024, stdin)) {
        free(buf);
        return NULL;
    }
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') buf[len - 1] = '\0';
    return buf;
}

int scanf_int(const char* fmt, int* p) {
    return scanf(fmt, p);
}

int scanf_int64(const char* fmt, int64_t* p) {
    return scanf(fmt, p);
}

int scanf_float(const char* fmt, double* p) {
    return scanf(fmt, p);
}

int fprintf_stderr(const char* fmt, const char* s) {
    return fprintf(stderr, fmt, s);
}

int fprintf_stderr_int(const char* fmt, int64_t i) {
    return fprintf(stderr, fmt, i);
}

int fprintf_stderr_float(const char* fmt, double f) {
    return fprintf(stderr, fmt, f);
}

// ── Math Library Wrappers ──────────

double agam_math_abs(double x) { return fabs(x); }

// Memory safety check helper (if we ever emit it)
void agam_bound_error() {
    fprintf(stderr, "Index out of bounds\n");
    exit(1);
}

// ── OS Library Wrappers ──

void agam_os_exit(int64_t code) {
    exit((int)code);
}

const char* agam_os_getenv(const char* name) {
    const char* val = getenv(name);
    return val ? val : "";
}

int64_t agam_os_system(const char* cmd) {
    return system(cmd);
}

const char* agam_os_name() {
#ifdef _WIN32
    return "windows";
#elif __APPLE__
    return "macos";
#else
    return "linux";
#endif
}

// ── Time Library Wrappers ──

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <time.h>
#endif

int64_t agam_time_epoch() {
    return (int64_t)time(NULL);
}

void agam_time_sleep(double seconds) {
#ifdef _WIN32
    Sleep((DWORD)(seconds * 1000.0));
#else
    struct timespec ts;
    ts.tv_sec = (time_t)seconds;
    ts.tv_nsec = (long)((seconds - ts.tv_sec) * 1e9);
    nanosleep(&ts, NULL);
#endif
}

void agam_time_sleep_ms(int64_t ms) {
#ifdef _WIN32
    Sleep((DWORD)ms);
#else
    usleep(ms * 1000);
#endif
}
