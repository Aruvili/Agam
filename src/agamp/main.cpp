/// Agam Package Manager (agamp) - Inspired by Cargo
///
/// Commands:
///   agamp new <app-name>        Create a new Agam application project
///   agamp build [--release]     Build the project into ./target/
///   agamp run [--release]       Build and execute the application
///   agamp check                 Validate and type-check without building
///   agamp add <package> | i     Add and install a package dependency
///   agamp update                Update installed package modules
///   agamp clean                 Clean build target artifacts
///   agamp list                  List installed dependencies

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <sstream>
#if defined(__linux__) || defined(__APPLE__)
#include <unistd.h>
#endif
#if defined(_WIN32)
#include <windows.h>
#endif

namespace fs = std::filesystem;

static fs::path getExecutableDir(const char *argv0) {
#if defined(__linux__)
    char result[1024];
    ssize_t count = ::readlink("/proc/self/exe", result, sizeof(result) - 1);
    if (count != -1) {
        result[count] = '\0';
        return fs::path(result).parent_path();
    }
#elif defined(_WIN32)
    char result[MAX_PATH];
    DWORD ret = GetModuleFileNameA(NULL, result, MAX_PATH);
    if (ret > 0) {
        return fs::path(result).parent_path();
    }
#endif
    fs::path p(argv0);
    if (p.is_absolute()) return p.parent_path();
    if (p.has_parent_path()) return fs::absolute(p).parent_path();
    const char *pathEnv = std::getenv("PATH");
    if (pathEnv) {
        std::stringstream ss(pathEnv);
        std::string item;
#if defined(_WIN32)
        char delim = ';';
#else
        char delim = ':';
#endif
        while (std::getline(ss, item, delim)) {
            fs::path candidate = fs::path(item) / p;
            if (fs::exists(candidate)) {
                return fs::canonical(candidate).parent_path();
            }
        }
    }
    return fs::absolute(p).parent_path();
}

static void printUsage() {
    std::cout << "========================================================\n"
              << " அகம் தொகுப்பு மேலாளர் (Agam Package Manager - agamp)\n"
              << "========================================================\n"
              << "பயன்பாடு (Usage):\n"
              << "  agamp new <பெயர்>             புதிய அகம் திட்டத்தை உருவாக்குக (Cargo new)\n"
              << "  agamp build [--release]       திட்டப்பொறி தொகுத்தல் (Cargo build)\n"
              << "  agamp run [--release]         திட்டப்பொறி இயக்குதல் (Cargo run)\n"
              << "  agamp check                   வகைப் பரிசோதனை (Cargo check)\n"
              << "  agamp add <தொகுப்பு>          தொகுப்பு சேர்த்தல் (Cargo add / agamp i)\n"
              << "  agamp update                  தொகுப்புகளைப் புதுப்பித்தல் (Cargo update)\n"
              << "  agamp clean                   இலக்குக் கோப்பகத்தைச் சுத்தம் செய்க (Cargo clean)\n"
              << "  agamp list                    நிறுவப்பட்ட தொகுப்புகளைப் பட்டியலிடுக\n"
              << "  agamp -h, --help              உதவிச் செய்தியை அச்சிடுக\n";
}

static fs::path findStdPackagesPath(const fs::path &exeDir) {
    std::vector<fs::path> candidates = {
        exeDir / ".." / "std" / "packages",
        exeDir / ".." / "share" / "agam" / "std" / "packages",
        fs::current_path() / "std" / "packages",
        fs::current_path() / ".." / "std" / "packages"
    };

    for (const auto &p : candidates) {
        if (fs::exists(p)) {
            return p;
        }
    }
    return "";
}

static int handleNew(const std::string &projName) {
    fs::path projectRoot = fs::current_path() / projName;
    if (fs::exists(projectRoot)) {
        std::cerr << "[ERROR] பிழை: '" << projName << "' என்ற கோப்பகம் ஏற்கனவே உள்ளது.\n";
        return 1;
    }

    fs::create_directories(projectRoot);
    fs::path srcDir = projectRoot / "src";
    fs::path modulesDir = projectRoot / "modules";
    fs::path targetDebugDir = projectRoot / "target" / "debug";
    fs::path targetReleaseDir = projectRoot / "target" / "release";

    fs::create_directories(srcDir);
    fs::create_directories(modulesDir);
    fs::create_directories(targetDebugDir);
    fs::create_directories(targetReleaseDir);

    // 1. Create Cargo-like manifest: pk.arpk
    std::ofstream arpk(projectRoot / "pk.arpk");
    arpk << "projectname: \"" << projName << "\"\n"
         << "version: \"0.1.0\"\n"
         << "author: \"\"\n"
         << "edition: \"2026\"\n\n"
         << "#dependencies\n"
         << "[name]: [version]\n";
    arpk.close();

    // 2. Create .gitignore
    std::ofstream gitignore(projectRoot / ".gitignore");
    gitignore << "/target/\n"
              << "/modules/\n"
              << "*.exe\n"
              << "*.o\n";
    gitignore.close();

    // 3. Create src/main.agam
    std::ofstream mainAgam(srcDir / "main.agam");
    mainAgam << "இறக்குமதி \"std/io.agam\";\n\n"
             << "செயல் மைய(): எண் {\n"
             << "    வரியிறக்கி_பதிப்பி(\"வணக்கம் உலகம்!\");\n"
             << "    விடை 0;\n"
             << "}\n";
    mainAgam.close();

    std::cout << "[SUCCESS] புதிய அகம் திட்டம் '" << projName << "' உருவாக்கப்பட்டது (Created binary app `" << projName << "` package).\n";
    std::cout << "--> தொடங்குவதற்கு: cd " << projName << " && agamp run\n";
    return 0;
}

static fs::path findRegistryPath(const fs::path &exeDir) {
    std::vector<fs::path> candidates = {
        exeDir / ".." / "registry" / "index.json",
        exeDir / ".." / "share" / "agam" / "registry" / "index.json",
        fs::current_path() / "registry" / "index.json",
        fs::current_path() / ".." / "registry" / "index.json"
    };

    for (const auto &p : candidates) {
        if (fs::exists(p)) {
            return p;
        }
    }
    return "";
}

static void updateLockfile(const std::string &pkgName, const std::string &version, const std::string &source, const std::string &destPath) {
    fs::path lockPath = fs::current_path() / "pk.lock";
    std::ofstream lock(lockPath, std::ios::app);
    if (lock.is_open()) {
        lock << "\n[[package]]\n";
        lock << "name = \"" << pkgName << "\"\n";
        lock << "version = \"" << version << "\"\n";
        lock << "source = \"" << source << "\"\n";
        lock << "installed_path = \"" << destPath << "\"\n";
        lock.close();
    }
}

static bool isPackageInstalled(const std::string &pkgName, const fs::path &currentDir) {
    fs::path modulesDir = currentDir / "modules";
    std::string stemName = fs::path(pkgName).stem().string();

    if (stemName.length() > 4 && stemName.substr(stemName.length() - 4) == ".git") {
        stemName = stemName.substr(0, stemName.length() - 4);
    }

    bool fileOrDirExists = false;
    if (fs::exists(modulesDir)) {
        if (fs::exists(modulesDir / pkgName) ||
            fs::exists(modulesDir / (pkgName + ".agam")) ||
            fs::exists(modulesDir / stemName) ||
            fs::exists(modulesDir / (stemName + ".agam"))) {
            fileOrDirExists = true;
        }
        if (pkgName == "web_server" || pkgName == "server" || pkgName == "web" || pkgName == "valaiccevaiyagam") {
            if (fs::exists(modulesDir / "வலைச்சேவையகம்.agam") ||
                fs::exists(modulesDir / "valaiccevaiyagam.agam") ||
                fs::exists(modulesDir / "valaiccevaiyagam")) {
                fileOrDirExists = true;
            }
        }
    }

    bool inArpk = false;
    fs::path arpkPath = currentDir / "pk.arpk";
    if (fs::exists(arpkPath)) {
        std::ifstream arpk(arpkPath);
        std::string line;
        while (std::getline(arpk, line)) {
            size_t start = line.find_first_not_of(" \t");
            if (start != std::string::npos) {
                line = line.substr(start);
            }
            if (line.rfind(pkgName + ":", 0) == 0 || line.rfind(stemName + ":", 0) == 0) {
                inArpk = true;
                break;
            }
        }
    }

    bool inLock = false;
    fs::path lockPath = currentDir / "pk.lock";
    if (fs::exists(lockPath)) {
        std::ifstream lock(lockPath);
        std::string line;
        std::string targetNameQuote = "name = \"" + pkgName + "\"";
        std::string targetStemQuote = "name = \"" + stemName + "\"";
        while (std::getline(lock, line)) {
            if (line.find(targetNameQuote) != std::string::npos ||
                line.find(targetStemQuote) != std::string::npos) {
                inLock = true;
                break;
            }
        }
    }

    return fileOrDirExists || inArpk || inLock;
}

static int handleAdd(const std::string &pkgName, const fs::path &exeDir) {
    fs::path currentDir = fs::current_path();

    if (isPackageInstalled(pkgName, currentDir)) {
        std::cout << "[INFO] தொகுப்பு '" << pkgName << "' ஏற்கனவே சேர்க்கப்பட்டு/நிறுவப்பட்டு உள்ளது (Package '" << pkgName << "' is already added or installed).\n";
        return 0;
    }

    fs::path arpkPath = currentDir / "pk.arpk";
    fs::path modulesDir = currentDir / "modules";
    if (!fs::exists(modulesDir)) {
        fs::create_directories(modulesDir);
    }

    fs::path targetPkgFile;
    std::string sourceName = pkgName;

    // ── Tier 1: Check Central Registry Index (registry/index.json) ───────────
    fs::path regPath = findRegistryPath(exeDir);
    if (!regPath.empty()) {
        std::ifstream regFile(regPath);
        if (regFile.is_open()) {
            std::string content((std::istreambuf_iterator<char>(regFile)), std::istreambuf_iterator<char>());
            regFile.close();

            std::string searchKey = "\"" + pkgName + "\"";
            size_t pos = content.find(searchKey);
            if (pos != std::string::npos) {
                size_t urlPos = content.find("\"url\"", pos);
                if (urlPos != std::string::npos) {
                    size_t startQuote = content.find("\"", urlPos + 5);
                    size_t endQuote = content.find("\"", startQuote + 1);
                    if (startQuote != std::string::npos && endQuote != std::string::npos) {
                        std::string relUrl = content.substr(startQuote + 1, endQuote - startQuote - 1);
                        if (relUrl.rfind("http://", 0) == 0 || relUrl.rfind("https://", 0) == 0 || relUrl.rfind("git@", 0) == 0 || relUrl.find(".git") != std::string::npos) {
                            fs::path destRepo = modulesDir / pkgName;
                            std::cout << "    [REGISTRY] Resolved '" << pkgName << "' -> " << relUrl << "\n";
                            std::cout << "  [FETCHING] Remote Git repository: " << relUrl << " ...\n";
                            std::string cloneCmd = "git clone --depth 1 \"" + relUrl + "\" \"" + destRepo.string() + "\" > /dev/null 2>&1";
                            int res = std::system(cloneCmd.c_str());
                            if (res == 0) {
                                updateLockfile(pkgName, "1.0.0", relUrl, "modules/" + pkgName);
                                std::cout << "[SUCCESS] Remote package '" << pkgName << "' cloned into modules/\n";
                                std::cout << "[SUCCESS] Updated pk.lock\n";
                                return 0;
                            }
                        } else {
                            fs::path cand = regPath.parent_path() / ".." / relUrl;
                            if (fs::exists(cand)) {
                                targetPkgFile = cand;
                                sourceName = "registry:" + pkgName;
                            }
                        }
                    }
                }
            }
        }
    }

    // ── Tier 2: Check Local Standard Library Packages ─────────────────────────
    if (targetPkgFile.empty()) {
        fs::path stdPkgDir = findStdPackagesPath(exeDir);
        if (!stdPkgDir.empty()) {
            fs::path candidate = stdPkgDir / (pkgName + ".agam");
            if (fs::exists(candidate)) {
                targetPkgFile = candidate;
            } else if (pkgName == "web_server" || pkgName == "server" || pkgName == "web" || pkgName == "valaiccevaiyagam") {
                fs::path fallback = stdPkgDir / "வலைச்சேவையகம்.agam";
                if (fs::exists(fallback)) {
                    targetPkgFile = fallback;
                }
            }
        }
    }

    if (!targetPkgFile.empty()) {
        fs::path destFile = modulesDir / (pkgName + ".agam");
        fs::copy_file(targetPkgFile, destFile, fs::copy_options::overwrite_existing);

        if (fs::exists(arpkPath)) {
            std::ofstream arpk(arpkPath, std::ios::app);
            arpk << pkgName << ": 1.0.0\n";
            arpk.close();
        }
        updateLockfile(pkgName, "1.0.0", targetPkgFile.string(), "modules/" + pkgName + ".agam");

        std::cout << "    [REGISTRY] Resolved '" << pkgName << "' v1.0.0\n";
        std::cout << "[SUCCESS] தொகுப்பு '" << pkgName << "' வெற்றிகரமாக நிறுவப்பட்டது!\n";
        std::cout << "[SUCCESS] Updated pk.lock\n";
        return 0;
    }

    // ── Tier 3: Remote Git Repository Fallback (http/https/git@ or github.com/user/repo) ──
    std::string gitUrl = pkgName;
    if (gitUrl.find("http://") == std::string::npos && gitUrl.find("https://") == std::string::npos && gitUrl.find("git@") == std::string::npos) {
        gitUrl = "https://github.com/" + pkgName + ".git";
    }

    fs::path repoName = fs::path(pkgName).stem();
    fs::path destRepo = modulesDir / repoName;
    std::cout << "  [FETCHING] Git repository fallback: " << gitUrl << " ...\n";
    std::string cloneCmd = "git clone --depth 1 \"" + gitUrl + "\" \"" + destRepo.string() + "\" > /dev/null 2>&1";
    int res = std::system(cloneCmd.c_str());
    if (res == 0) {
        updateLockfile(repoName.string(), "1.0.0", gitUrl, "modules/" + repoName.string());
        std::cout << "[SUCCESS] Remote package '" << repoName.string() << "' cloned into modules/\n";
        std::cout << "[SUCCESS] Updated pk.lock\n";
        return 0;
    }

    std::cerr << "[ERROR] பிழை: '" << pkgName << "' என்ற தொகுப்பு Registry, Standard Library, அல்லது Git repo இல் கிடைக்கவில்லை.\n";
    return 1;
}

static fs::path findAgamcExecutable(const fs::path &exeDir) {
    fs::path cand = exeDir / "agamc";
#ifdef _WIN32
    if (!fs::exists(cand)) cand = exeDir / "agamc.exe";
#endif
    if (fs::exists(cand)) return cand;
    return "agamc";
}

static int handleCheck(const fs::path &exeDir) {
    fs::path mainPath = fs::current_path() / "src" / "main.agam";
    if (!fs::exists(mainPath)) mainPath = fs::current_path() / "main.agam";

    if (!fs::exists(mainPath)) {
        std::cerr << "[ERROR] பிழை: 'src/main.agam' காணப்படவில்லை.\n";
        return 1;
    }

    fs::path agamcPath = findAgamcExecutable(exeDir);

    std::cout << "    [CHECKING] Checking project for errors...\n";
    std::string cmd = "\"" + agamcPath.string() + "\" --emit-mir \"" + mainPath.string() + "\" > /dev/null";
    int status = std::system(cmd.c_str());
    if (status == 0) {
        std::cout << "[SUCCESS] Finished checking (0 errors).\n";
    }
    return status;
}

static int handleBuild(bool isRelease, const fs::path &exeDir) {
    fs::path mainPath = fs::current_path() / "src" / "main.agam";
    if (!fs::exists(mainPath)) mainPath = fs::current_path() / "main.agam";

    if (!fs::exists(mainPath)) {
        std::cerr << "[ERROR] பிழை: 'src/main.agam' காணப்படவில்லை.\n";
        return 1;
    }

    fs::path targetSubDir = fs::current_path() / "target" / (isRelease ? "release" : "debug");
    fs::create_directories(targetSubDir);

    fs::path projName = fs::current_path().filename();
    fs::path outBin = targetSubDir / projName;

    fs::path agamcPath = findAgamcExecutable(exeDir);

    std::cout << "   [COMPILING] " << projName.string() << " (" << (isRelease ? "release [optimized]" : "debug [unoptimized]") << ")...\n";
    std::string optFlag = isRelease ? "-O3" : "-O0";
    
    // Explicitly compile binary artifact inside target/debug/ or target/release/
    std::string cmd = "\"" + agamcPath.string() + "\" " + optFlag + " -o \"" + outBin.string() + "\" \"" + mainPath.string() + "\"";
    int status = std::system(cmd.c_str());
    if (status == 0) {
        std::cout << "[SUCCESS] Compiled artifact saved: target/" << (isRelease ? "release" : "debug") << "/" << projName.string() << "\n";
    }
    return status;
}

static int handleRun(bool isRelease, const fs::path &exeDir) {
    fs::path mainPath = fs::current_path() / "src" / "main.agam";
    if (!fs::exists(mainPath)) mainPath = fs::current_path() / "main.agam";

    if (!fs::exists(mainPath)) {
        std::cerr << "[ERROR] பிழை: 'src/main.agam' காணப்படவில்லை.\n";
        return 1;
    }

    fs::path projName = fs::current_path().filename();
    std::cout << "    [RUNNING] `target/" << (isRelease ? "release" : "debug") << "/" << projName.string() << "`\n";

    fs::path agamcPath = findAgamcExecutable(exeDir);

    std::string optFlag = isRelease ? "-O3" : "-O0";
    std::string cmd = "\"" + agamcPath.string() + "\" " + optFlag + " --run \"" + mainPath.string() + "\"";
    return std::system(cmd.c_str());
}

static int handleUpdate(const fs::path &exeDir) {
    fs::path modulesDir = fs::current_path() / "modules";
    if (!fs::exists(modulesDir)) {
        std::cout << "புதுப்பிக்கத் தொகுப்புகள் எதுவுமில்லை (No modules directory).\n";
        return 0;
    }

    fs::path stdPkgDir = findStdPackagesPath(exeDir);
    if (stdPkgDir.empty()) {
        std::cerr << "[ERROR] பிழை: Standard packages directory missing.\n";
        return 1;
    }

    std::cout << "[UPDATING] Updating installed package modules...\n";
    int count = 0;
    for (const auto &entry : fs::directory_iterator(modulesDir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".agam") {
            fs::path srcPkg = stdPkgDir / entry.path().filename();
            if (fs::exists(srcPkg)) {
                fs::copy_file(srcPkg, entry.path(), fs::copy_options::overwrite_existing);
                std::cout << "   Updated " << entry.path().filename().string() << "\n";
                count++;
            }
        }
    }
    std::cout << "[SUCCESS] " << count << " package module(s) updated successfully.\n";
    return 0;
}

static int handleClean() {
    fs::path targetDir = fs::current_path() / "target";
    if (fs::exists(targetDir)) {
        fs::remove_all(targetDir);
        std::cout << "[CLEANED] Removed target/ directory.\n";
    } else {
        std::cout << "target/ directory is already clean.\n";
    }
    return 0;
}

static int handleList() {
    fs::path arpkPath = fs::current_path() / "pk.arpk";
    if (!fs::exists(arpkPath)) {
        std::cout << "பிழை: 'pk.arpk' காணப்படவில்லை.\n";
        return 1;
    }

    std::cout << "[INSTALLED PACKAGES] நிறுவப்பட்ட தொகுப்புகள்:\n";
    std::ifstream arpk(arpkPath);
    std::string line;
    while (std::getline(arpk, line)) {
        if (!line.empty() && line[0] != '#' && line.find("projectname:") == std::string::npos && line.find("[name]") == std::string::npos) {
            std::cout << "  - " << line << "\n";
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printUsage();
        return 0;
    }

    fs::path exeDir = getExecutableDir(argv[0]);
    std::string cmd = argv[1];

    bool isRelease = false;
    for (int i = 2; i < argc; ++i) {
        if (std::string(argv[i]) == "--release") {
            isRelease = true;
        }
    }

    if (cmd == "-h" || cmd == "--help") {
        printUsage();
        return 0;
    } else if (cmd == "new" || cmd == "create" || cmd == "init") {
        if (argc < 3) {
            std::cerr << "பிழை: 'agamp new <app-name>' செய்ய திட்ட பெயர் தேவை.\n";
            return 1;
        }
        return handleNew(argv[2]);
    } else if (cmd == "add" || cmd == "install" || cmd == "i") {
        if (argc < 3) {
            std::cerr << "பிழை: 'agamp add <package>' செய்ய தொகுப்பு பெயர் தேவை.\n";
            return 1;
        }
        return handleAdd(argv[2], exeDir);
    } else if (cmd == "build") {
        return handleBuild(isRelease, exeDir);
    } else if (cmd == "run") {
        return handleRun(isRelease, exeDir);
    } else if (cmd == "check") {
        return handleCheck(exeDir);
    } else if (cmd == "update") {
        return handleUpdate(exeDir);
    } else if (cmd == "clean") {
        return handleClean();
    } else if (cmd == "list") {
        return handleList();
    } else {
        std::cerr << "தெரியாத கட்டளை (Unknown command): " << cmd << "\n\n";
        printUsage();
        return 1;
    }
}
