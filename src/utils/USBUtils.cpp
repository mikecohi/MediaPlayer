#include "USBUtils.h"
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <algorithm>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

// ===============================================
// Helper: kiểm tra có phải đang chạy trong WSL
// ===============================================
bool USBUtils::isWSLEnvironment() {
    FILE* f = fopen("/proc/version", "r");
    if (!f) return false;
    char buf[256];
    std::string ver;
    while (fgets(buf, sizeof(buf), f)) ver += buf;
    fclose(f);
    return ver.find("Microsoft") != std::string::npos || ver.find("WSL") != std::string::npos;
}


// ===============================================
// Helper: kiểm tra thư mục có đang được mount không
// ===============================================
bool USBUtils::isMounted(const std::string& path) {
    FILE* fp = popen(("mount | grep \"" + path + "\"").c_str(), "r");
    if (!fp) return false;
    char buf[256];
    bool mounted = fgets(buf, sizeof(buf), fp) != nullptr;
    pclose(fp);
    return mounted;
}

// ===============================================
// API cũ: phát hiện USB (chỉ dùng trong WSL)
// ===============================================
std::string USBUtils::detectWSLUSBDrive() {
    const char* cmd =
        "/mnt/c/Windows/System32/WindowsPowerShell/v1.0/powershell.exe "
        "-Command \"(Get-Volume | Where-Object {\\$_.DriveType -eq 'Removable'} | "
        "Select -ExpandProperty DriveLetter)\"";

    FILE* pipe = popen(cmd, "r");
    if (!pipe) {
        std::cerr << "[USBUtils] ❌ Cannot open PowerShell pipe\n";
        return "";
    }

    char buffer[128];
    std::string result;

    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    pclose(pipe);

    // Remove BOM + whitespace
    if (!result.empty() && (unsigned char)result[0] == 0xEF) {
        result.erase(0, 3);
    }
    result.erase(std::remove_if(result.begin(), result.end(), ::isspace), result.end());

    if (result.empty()) {
        std::cerr << "[USBUtils] ⚠️ No removable drive detected.\n";
        return "";
    }

    // Chỉ lấy ký tự đầu tiên (tránh nhiều USB)
    if (result.size() > 1) result = result.substr(0, 1);

    return result + ":";
}

// ===============================================
// API cũ: mount ổ USB trong WSL (drvfs)
// ===============================================
bool USBUtils::mountWSLDrive(const std::string& driveLetter, const std::string& mountPath) {
    //ensureMountPathExists(mountPath);
    std::string cmd = "sudo -n mount -t drvfs " + driveLetter + " " + mountPath + " >/dev/null 2>&1";
    int ret = std::system(cmd.c_str());
    if (ret == 0) {
        std::cout << "[USBUtils] ✅ Mounted " << driveLetter << " to " << mountPath << "\n";
        return true;
    } else {
        std::cerr << "[USBUtils] ⚠️ Mount failed: " << cmd << "\n";
        return false;
    }
}

// ===============================================
// API cũ: reload USB trong WSL
// ===============================================
bool USBUtils::reloadUSBLibrary(const std::string& mountPath) {
    std::string newDrive = detectWSLUSBDrive();
    if (newDrive.empty()) {
        std::cerr << "[USBUtils] ❌ Cannot reload. No removable USB found.\n";
        return false;
    }

    std::cout << "[USBUtils] 🔁 Reloading USB drive " << newDrive << "\n";
    std::string cmd = "sudo umount " + mountPath + " >/dev/null 2>&1";
    std::system(cmd.c_str());

    if (!mountWSLDrive(newDrive, mountPath)) {
        std::cerr << "[USBUtils] ❌ Reload failed.\n";
        return false;
    }

    std::cout << "[USBUtils] ✅ Reload success. Mounted at " << mountPath << "\n";
    return true;
}

// ===============================================
// API cũ: eject USB (umount)
// ===============================================
bool USBUtils::ejectUSB(const std::string& mountPath) {
    std::string cmd = "sudo -n umount " + mountPath + " >/dev/null 2>&1";
    int ret = std::system(cmd.c_str());
    if (ret == 0) {
        std::cout << "[USBUtils] ✅ Ejected USB successfully from: " << mountPath << "\n";
        return true;
    } else {
        std::cerr << "[USBUtils] ❌ Failed to eject USB at: " << mountPath << "\n";
        return false;
    }
}

// ===============================================
// 🔹 NEW API: Trả về đường dẫn thực tế chứa dữ liệu USB
// ===============================================
std::string USBUtils::detectUSBPath() {
    if (isWSLEnvironment()) {
        // --- Trong WSL ---
        std::string mountPath = "/home/" + std::string(getenv("USER") ? getenv("USER") : "user") + "/project/media/usb";
        //ensureMountPathExists(mountPath);

        std::string drive = detectWSLUSBDrive();
        if (drive.empty()) {
            std::cerr << "[USBUtils] ⚠️ No USB detected in WSL.\n";
            return "";
        }

        if (!isMounted(mountPath)) {
            if (!mountWSLDrive(drive, mountPath)) return "";
        }

        return mountPath;
    } else {
        // --- Trong Linux thật ---
        std::string user = getenv("USER") ? getenv("USER") : "user";
        std::string base = "/media/" + user + "/";
        FILE* fp = popen(("ls " + base).c_str(), "r");
        if (!fp) return "";

        char buf[128];
        std::string folder;
        if (fgets(buf, sizeof(buf), fp)) folder = buf;
        pclose(fp);

        folder.erase(std::remove_if(folder.begin(), folder.end(), ::isspace), folder.end());
        if (folder.empty()) {
            std::cerr << "[USBUtils] ⚠️ No USB mounted under /media/" << user << "\n";
            return "";
        }

        std::string fullPath = base + folder;
        std::cout << "[USBUtils] ✅ Linux USB mounted at " << fullPath << "\n";
        return fullPath;
    }
}