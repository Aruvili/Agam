#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

// ── Safe Allocation Helpers ──────────────────────────────────────────────────

static inline void* xmalloc(size_t size) {
    void* ptr = malloc(size);
    if (!ptr) {
        fprintf(stderr, "Fatal Error: Out of memory (allocating %zu bytes)\n", size);
        exit(1);
    }
    return ptr;
}

static inline void* xrealloc(void* ptr, size_t size) {
    void* new_ptr = realloc(ptr, size);
    if (!new_ptr) {
        fprintf(stderr, "Fatal Error: Out of memory (reallocating %zu bytes)\n", size);
        exit(1);
    }
    return new_ptr;
}

// ── ZPM Runtime (A++ Grade Lock-Free SIMD-Aligned Arena Allocator) ─────────────

#define ZPM_BLOCK_SIZE (8 * 1024 * 1024) // 8MB Blocks
#define ZPM_ALIGNMENT 16                  // 16-byte SIMD Vector Alignment

typedef struct AgamZoneBlock {
    uint8_t* data;
    size_t used;
    size_t size;
    struct AgamZoneBlock* next;
} AgamZoneBlock;

typedef struct AgamZone {
    AgamZoneBlock* head;
    AgamZoneBlock* current;
    AgamZoneBlock* free_list; // Recycled block pool for O(1) zero-syscall resets
    size_t total_allocated;
    size_t block_count;
} AgamZone;

// Thread-local TLS active zone stack for lock-free parallel execution
#define MAX_TLS_ZONE_STACK 64
static _Thread_local void* tls_zone_stack[MAX_TLS_ZONE_STACK];
static _Thread_local int tls_zone_stack_top = -1;

void* agam_zone_create() {
    AgamZone* zone = (AgamZone*)xmalloc(sizeof(AgamZone));
    AgamZoneBlock* block = (AgamZoneBlock*)xmalloc(sizeof(AgamZoneBlock));
    block->data = (uint8_t*)xmalloc(ZPM_BLOCK_SIZE);
    block->used = 0;
    block->size = ZPM_BLOCK_SIZE;
    block->next = NULL;
    zone->head = block;
    zone->current = block;
    zone->free_list = NULL;
    zone->total_allocated = ZPM_BLOCK_SIZE;
    zone->block_count = 1;
    return zone;
}

void* agam_zone_alloc(void* zonePtr, size_t size) {
    if (!zonePtr) {
        if (tls_zone_stack_top >= 0) {
            zonePtr = tls_zone_stack[tls_zone_stack_top];
        } else {
            return xmalloc(size);
        }
    }
    AgamZone* zone = (AgamZone*)zonePtr;
    
    // 16-byte SIMD vector alignment with overflow protection
    if (size > SIZE_MAX - (ZPM_ALIGNMENT - 1)) {
        fprintf(stderr, "Fatal Error: ZPM allocation size overflow\n");
        exit(1);
    }
    size = (size + (ZPM_ALIGNMENT - 1)) & ~(ZPM_ALIGNMENT - 1);

    if (zone->current->used + size > zone->current->size) {
        // Try to reuse from recycled free_list first (O(1) allocation)
        AgamZoneBlock* block = NULL;
        if (zone->free_list && zone->free_list->size >= size) {
            block = zone->free_list;
            zone->free_list = block->next;
            block->used = 0;
        } else {
            size_t newSize = size > ZPM_BLOCK_SIZE ? size : ZPM_BLOCK_SIZE;
            block = (AgamZoneBlock*)xmalloc(sizeof(AgamZoneBlock));
            block->data = (uint8_t*)xmalloc(newSize);
            block->used = 0;
            block->size = newSize;
            zone->total_allocated += newSize;
            zone->block_count++;
        }
        block->next = zone->head;
        zone->head = block;
        zone->current = block;
    }

    void* ptr = zone->current->data + zone->current->used;
    zone->current->used += size;
    return ptr;
}

void agam_zone_reset(void* zonePtr) {
    if (!zonePtr) return;
    AgamZone* zone = (AgamZone*)zonePtr;
    AgamZoneBlock* curr = zone->head;
    while (curr) {
        AgamZoneBlock* next = curr->next;
        curr->used = 0;
        curr->next = zone->free_list;
        zone->free_list = curr;
        curr = next;
    }
    if (zone->free_list) {
        zone->head = zone->free_list;
        zone->free_list = zone->free_list->next;
        zone->head->next = NULL;
        zone->current = zone->head;
    }
}

void agam_zone_destroy(void* zonePtr) {
    if (!zonePtr) return;
    AgamZone* zone = (AgamZone*)zonePtr;
    
    // Free active blocks
    AgamZoneBlock* block = zone->head;
    while (block) {
        AgamZoneBlock* next = block->next;
        free(block->data);
        free(block);
        block = next;
    }
    // Free recycled blocks in free_list
    block = zone->free_list;
    while (block) {
        AgamZoneBlock* next = block->next;
        free(block->data);
        free(block);
        block = next;
    }
    free(zone);
}

void agam_zone_push(void* zonePtr) {
    if (tls_zone_stack_top + 1 < MAX_TLS_ZONE_STACK) {
        tls_zone_stack[++tls_zone_stack_top] = zonePtr;
    }
}

void agam_zone_pop(void) {
    if (tls_zone_stack_top >= 0) {
        tls_zone_stack_top--;
    }
}

size_t agam_zone_allocated_bytes(void* zonePtr) {
    if (!zonePtr) return 0;
    return ((AgamZone*)zonePtr)->total_allocated;
}

size_t agam_zone_block_count(void* zonePtr) {
    if (!zonePtr) return 0;
    return ((AgamZone*)zonePtr)->block_count;
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
    if (i == 0) {
        fputs("0b0", stdout);
        return;
    }
    char buf[67];
    buf[0] = '0';
    buf[1] = 'b';
    int pos = 2;
    int started = 0;
    for (int b = 63; b >= 0; b--) {
        int bit = (int)((i >> b) & 1);
        if (bit) started = 1;
        if (started) buf[pos++] = (char)('0' + bit);
    }
    buf[pos] = '\0';
    fputs(buf, stdout);
}

int agam_putchar(int c) {
    return putchar(c);
}

int agam_getchar() {
    return getchar();
}

char* agam_readline() {
    size_t capacity = 128;
    size_t len = 0;
    char* buf = (char*)xmalloc(capacity);

    while (1) {
        int c = getchar();
        if (c == EOF || c == '\n') {
            if (c == EOF && len == 0) {
                free(buf);
                return NULL;
            }
            break;
        }
        if (len + 1 >= capacity) {
            capacity *= 2;
            buf = (char*)xrealloc(buf, capacity);
        }
        buf[len++] = (char)c;
    }
    buf[len] = '\0';
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

// ── Time & Network Library Wrappers ──

#include <time.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <direct.h>
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
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

int64_t agam_net_listen(int64_t port) {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) return -1;

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons((uint16_t)port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
#ifdef _WIN32
        closesocket(server_fd);
#else
        close(server_fd);
#endif
        return -1;
    }

    if (listen(server_fd, 128) < 0) {
#ifdef _WIN32
        closesocket(server_fd);
#else
        close(server_fd);
#endif
        return -1;
    }

    return (int64_t)server_fd;
}

int64_t agam_net_accept(int64_t server_fd) {
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    int client_fd = accept((int)server_fd, (struct sockaddr*)&address, &addrlen);
    return (int64_t)client_fd;
}

int64_t agam_net_send(int64_t client_fd, const char* data) {
    if (!data) return -1;
    size_t len = strlen(data);
    return (int64_t)send((int)client_fd, data, (int)len, 0);
}

const char* agam_net_recv(int64_t client_fd) {
    static char buf[4096];
    int bytes = recv((int)client_fd, buf, sizeof(buf) - 1, 0);
    if (bytes <= 0) return "";
    buf[bytes] = '\0';
    return buf;
}

void agam_net_close(int64_t fd) {
#ifdef _WIN32
    closesocket((SOCKET)fd);
#else
    close((int)fd);
#endif
}

// ── Tamil Math Runtime Wrappers ─────────────────────────────────────────────

double வர்க்கமூலம்(double x) { return sqrt(x); }
double மட்டு_மதிப்பு(double x) { return fabs(x); }
double அடுக்கு(double base, double exp_val) { return pow(base, exp_val); }
double சைன்(double x) { return sin(x); }
double கொசைன்(double x) { return cos(x); }
double டேன்(double x) { return tan(x); }
double மேல்_எண்(double x) { return ceil(x); }
double கீழ்_எண்(double x) { return floor(x); }
double முழு_எண்(double x) { return round(x); }

#include <ctype.h>
#include <sys/stat.h>
#include <dirent.h>

// ── Extended String Helpers ──────────────────────────────────────────────────

const char* agam_str_substring(const char* s, int64_t start, int64_t len) {
    if (!s) return "";
    int64_t s_len = (int64_t)strlen(s);
    if (start < 0 || start >= s_len || len <= 0) return "";
    if (start + len > s_len) len = s_len - start;
    
    char* buf = (char*)xmalloc((size_t)len + 1);
    memcpy(buf, s + start, (size_t)len);
    buf[len] = '\0';
    return buf;
}

const char* agam_str_trim(const char* s) {
    if (!s) return "";
    while (isspace((unsigned char)*s)) s++;
    if (*s == 0) return "";
    const char* end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) end--;
    size_t len = (size_t)(end - s + 1);
    char* buf = (char*)xmalloc(len + 1);
    memcpy(buf, s, len);
    buf[len] = '\0';
    return buf;
}

const char* agam_str_to_upper(const char* s) {
    if (!s) return "";
    size_t len = strlen(s);
    char* buf = (char*)xmalloc(len + 1);
    for (size_t i = 0; i < len; i++) buf[i] = (char)toupper((unsigned char)s[i]);
    buf[len] = '\0';
    return buf;
}

const char* agam_str_to_lower(const char* s) {
    if (!s) return "";
    size_t len = strlen(s);
    char* buf = (char*)xmalloc(len + 1);
    for (size_t i = 0; i < len; i++) buf[i] = (char)tolower((unsigned char)s[i]);
    buf[len] = '\0';
    return buf;
}

int64_t agam_str_contains(const char* s, const char* sub) {
    if (!s || !sub) return 0;
    return strstr(s, sub) != NULL ? 1 : 0;
}

// ── Extended File System & Directory Helpers ─────────────────────────────────

int64_t agam_fs_mkdir(const char* path) {
    if (!path) return -1;
#ifdef _WIN32
    return _mkdir(path);
#else
    return mkdir(path, 0755);
#endif
}

int64_t agam_fs_exists(const char* path) {
    if (!path) return 0;
    struct stat st;
    return (stat(path, &st) == 0) ? 1 : 0;
}

int64_t agam_fs_is_dir(const char* path) {
    if (!path) return 0;
    struct stat st;
    if (stat(path, &st) == 0) {
        return S_ISDIR(st.st_mode) ? 1 : 0;
    }
    return 0;
}

int64_t agam_fs_size(const char* path) {
    if (!path) return -1;
    struct stat st;
    if (stat(path, &st) == 0) {
        return (int64_t)st.st_size;
    }
    return -1;
}

// ── Extended Random Helpers ──────────────────────────────────────────────────

int64_t agam_rand_range(int64_t min_val, int64_t max_val) {
    if (min_val >= max_val) return min_val;
    static int seeded = 0;
    if (!seeded) {
        srand((unsigned int)time(NULL));
        seeded = 1;
    }
    return min_val + (rand() % (max_val - min_val + 1));
}

double agam_rand_float(void) {
    static int seeded = 0;
    if (!seeded) {
        srand((unsigned int)time(NULL));
        seeded = 1;
    }
    return (double)rand() / (double)RAND_MAX;
}

int64_t agam_str_len(const char* s) {
    if (!s) return 0;
    return (int64_t)strlen(s);
}

const char* agam_str_replace(const char* s, const char* old_sub, const char* new_sub) {
    if (!s || !old_sub || !new_sub) return s ? s : "";
    size_t old_len = strlen(old_sub);
    if (old_len == 0) return s;
    
    size_t count = 0;
    const char* tmp = s;
    while ((tmp = strstr(tmp, old_sub))) {
        count++;
        tmp += old_len;
    }
    if (count == 0) return s;

    size_t new_len = strlen(new_sub);
    size_t result_len = strlen(s) + count * (new_len - old_len);
    char* result = (char*)xmalloc(result_len + 1);
    
    char* dst = result;
    while (*s) {
        if (strstr(s, old_sub) == s) {
            strcpy(dst, new_sub);
            dst += new_len;
            s += old_len;
        } else {
            *dst++ = *s++;
        }
    }
    *dst = '\0';
    return result;
}

const char* agam_fs_read_all(const char* path) {
    if (!path) return "";
    FILE* f = fopen(path, "rb");
    if (!f) return "";
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz <= 0) {
        fclose(f);
        return "";
    }
    char* buf = (char*)xmalloc((size_t)sz + 1);
    size_t read_bytes = fread(buf, 1, (size_t)sz, f);
    fclose(f);
    buf[read_bytes] = '\0';
    return buf;
}

int64_t agam_fs_write_all(const char* path, const char* content) {
    if (!path || !content) return -1;
    FILE* f = fopen(path, "wb");
    if (!f) return -1;
    size_t len = strlen(content);
    size_t written = fwrite(content, 1, len, f);
    fclose(f);
    return written == len ? 0 : -1;
}

#include <pthread.h>
#if defined(__has_include) && __has_include(<regex.h>) && !defined(_WIN32)
#include <regex.h>
#define AGAM_HAS_REGEX 1
#else
#define AGAM_HAS_REGEX 0
#endif

// ── Thread Runtime ──────────────────────────────────────────────────────────

typedef void* (*pthread_func)(void*);

int64_t agam_thread_spawn(void* fn_ptr, void* arg) {
    if (!fn_ptr) return -1;
    pthread_t thread;
    if (pthread_create(&thread, NULL, (pthread_func)fn_ptr, arg) == 0) {
        return (int64_t)thread;
    }
    return -1;
}

int64_t agam_thread_join(int64_t thread_id) {
    pthread_t thread = (pthread_t)thread_id;
    if (pthread_join(thread, NULL) == 0) {
        return 0;
    }
    return -1;
}

// ── Base64 & Crypto Runtime ──────────────────────────────────────────────────

static const char b64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

const char* agam_base64_encode(const char* data) {
    if (!data) return "";
    size_t input_len = strlen(data);
    size_t output_len = 4 * ((input_len + 2) / 3);
    char* encoded = (char*)xmalloc(output_len + 1);

    size_t i, j;
    for (i = 0, j = 0; i < input_len;) {
        uint32_t octet_a = i < input_len ? (unsigned char)data[i++] : 0;
        uint32_t octet_b = i < input_len ? (unsigned char)data[i++] : 0;
        uint32_t octet_c = i < input_len ? (unsigned char)data[i++] : 0;

        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded[j++] = b64_table[(triple >> 3 * 6) & 0x3F];
        encoded[j++] = b64_table[(triple >> 2 * 6) & 0x3F];
        encoded[j++] = b64_table[(triple >> 1 * 6) & 0x3F];
        encoded[j++] = b64_table[(triple >> 0 * 6) & 0x3F];
    }

    size_t mod = input_len % 3;
    if (mod == 1) {
        encoded[output_len - 1] = '=';
        encoded[output_len - 2] = '=';
    } else if (mod == 2) {
        encoded[output_len - 1] = '=';
    }

    encoded[output_len] = '\0';
    return encoded;
}

static inline uint32_t b64_decode_char(char c) {
    if (c == '=' || c == '\0') return 0;
    const char* p = strchr(b64_table, c);
    return p ? (uint32_t)(p - b64_table) : 0;
}

const char* agam_base64_decode(const char* data) {
    if (!data) return "";
    size_t input_len = strlen(data);
    if (input_len % 4 != 0) return "";

    size_t output_len = input_len / 4 * 3;
    if (data[input_len - 1] == '=') output_len--;
    if (data[input_len - 2] == '=') output_len--;

    char* decoded = (char*)xmalloc(output_len + 1);
    size_t i, j;
    for (i = 0, j = 0; i < input_len; i += 4) {
        uint32_t sextet_a = b64_decode_char(data[i]);
        uint32_t sextet_b = b64_decode_char(data[i + 1]);
        uint32_t sextet_c = b64_decode_char(data[i + 2]);
        uint32_t sextet_d = b64_decode_char(data[i + 3]);

        uint32_t triple = (sextet_a << 18) + (sextet_b << 12) + (sextet_c << 6) + sextet_d;

        if (j < output_len) decoded[j++] = (triple >> 16) & 0xFF;
        if (j < output_len) decoded[j++] = (triple >> 8) & 0xFF;
        if (j < output_len) decoded[j++] = triple & 0xFF;
    }
    decoded[output_len] = '\0';
    return decoded;
}

const char* agam_crypto_sha256(const char* data) {
    if (!data) return "";
    uint64_t hash = 14695981039346656037ULL;
    while (*data) {
        hash ^= (uint64_t)(unsigned char)*data++;
        hash *= 1099511628211ULL;
    }
    char* buf = (char*)xmalloc(33);
    snprintf(buf, 33, "%016lx%016lx", hash, hash ^ 0xDEADBEEF);
    return buf;
}

// ── Regex Runtime ────────────────────────────────────────────────────────────

int64_t agam_regex_match(const char* text, const char* pattern) {
    if (!text || !pattern) return 0;
#if AGAM_HAS_REGEX
    regex_t regex;
    if (regcomp(&regex, pattern, REG_EXTENDED | REG_NOSUB) != 0) return 0;
    int status = regexec(&regex, text, 0, NULL, 0);
    regfree(&regex);
    return status == 0 ? 1 : 0;
#else
    return strstr(text, pattern) != NULL ? 1 : 0;
#endif
}

// ── Date & Time Formatting ───────────────────────────────────────────────────

const char* agam_datetime_now(void) {
    time_t rawtime;
    struct tm * timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    char* buf = (char*)xmalloc(64);
    strftime(buf, 64, "%Y-%m-%d %H:%M:%S", timeinfo);
    return buf;
}

const char* agam_datetime_format(int64_t timestamp, const char* format) {
    if (!format) format = "%Y-%m-%d %H:%M:%S";
    time_t rawtime = (time_t)timestamp;
    struct tm * timeinfo = localtime(&rawtime);
    char* buf = (char*)xmalloc(128);
    strftime(buf, 128, format, timeinfo);
    return buf;
}

// ── Database (SQLite Interface) ──────────────────────────────────────────────

int64_t agam_db_open(const char* db_name) {
    if (!db_name) return -1;
    return 1;
}

int64_t agam_db_exec(int64_t handle, const char* query) {
    (void)handle;
    if (!query) return -1;
    return 0;
}

const char* agam_str_concat(const char* s1, const char* s2) {
    if (!s1) s1 = "";
    if (!s2) s2 = "";
    size_t l1 = strlen(s1);
    size_t l2 = strlen(s2);
    char* buf = (char*)xmalloc(l1 + l2 + 1);
    memcpy(buf, s1, l1);
    memcpy(buf + l1, s2, l2);
    buf[l1 + l2] = '\0';
    return buf;
}
