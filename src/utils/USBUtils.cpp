#include "USBUtils.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <vector>

namespace fs = std::filesystem;

bool USBUtils::isRunningOnWSL() {
    std::ifstream versionFile("/proc/version");
    std::string content;
    if (versionFile.is_open()) {
        std::getline(versionFile, content);
        versionFile.close();
        return content.find("Microsoft") != std::string::npos ||
               content.find("WSL") != std::string::npos;
    }
    return false;
}
bool USBUtils::isDeviceRemovable(const std::string& deviceName) {
    std::string sysPath = "/sys/block/" + deviceName + "/removable";
    std::ifstream file(sysPath);
    if (!file.is_open()) return false;

    int flag = 0;
    file >> flag;
    file.close();
    return (flag == 1);
}

std::string USBUtils::detectUSBMount() {
    if (isRunningOnWSL()) {
        std::string mountPath = "/home/dung20210222/Documents/F/MediaPlayer";

        // Tạo thư mục mount nếu chưa có
        if (!fs::exists(mountPath)) {
            fs::create_directories(mountPath);
            std::cout << "[USBUtils] Created WSL USB mount folder: " << mountPath << "\n";
        }

        // 🔹 Dò ổ đĩa có thể là USB (E:, F:, G:, ...)
        std::vector<char> drives = {'d','e','f','g','h','i','j'};
        std::string detectedDrive;

        for (char letter : drives) {
            std::string mountProbe = "/mnt/";
            mountProbe.push_back(letter);
            if (!fs::exists(mountProbe)) continue;

            // 🔹 Kiểm tra nếu có dấu hiệu là USB (cờ removable trong /sys/block/sdX)
            // Trong WSL, ta map drive letter sang sdX nếu có thể
            std::string deviceName = "sd" + std::string(1, letter); // e.g. /sys/block/sde
            if (isDeviceRemovable(deviceName)) {
                detectedDrive = std::string(1, std::toupper(letter)) + ":";
                std::cout << "[USBUtils] ✅ Removable device detected: " << detectedDrive
                          << " (" << mountProbe << ")\n";
                break;
            }
        }

        if (detectedDrive.empty()) {
            std::cerr << "[USBUtils] ⚠️ No removable USB drive detected in WSL.\n";
            return "";
        }

        // Mount vào thư mục usb/
        if (!mountWSLDrive(detectedDrive, mountPath)) {
            std::cerr << "[USBUtils] ❌ Failed to mount " << detectedDrive << "\n";
            return "";
        }

        std::cout << "[USBUtils] ✅ USB mounted successfully at: " << mountPath << "\n";
        return mountPath;
    }

    // --- Linux thật ---
    const char* username = getenv("USER");
    if (!username) return "";

    std::vector<std::string> basePaths = {
        "/media/" + std::string(username),
        "/run/media/" + std::string(username)
    };

    for (const auto& base : basePaths) {
        if (!fs::exists(base)) continue;

        for (const auto& entry : fs::directory_iterator(base)) {
            if (fs::is_directory(entry.path())) {
                std::cout << "[USBUtils] ✅ Found USB at: " << entry.path() << "\n";
                return entry.path().string();
            }
        }
    }

    std::cerr << "[USBUtils] ❌ No USB device found.\n";
    return "";
}

bool USBUtils::mountWSLDrive(const std::string& driveLetter, const std::string& mountPath) {
    std::string cmd = "sudo mount -t drvfs " + driveLetter + " " + mountPath + " >/dev/null 2>&1";
    int ret = std::system(cmd.c_str());
    if (ret == 0) {
        std::cout << "[USBUtils] ✅ Mounted " << driveLetter << " to " << mountPath << "\n";
        return true;
    } else {
        std::cerr << "[USBUtils] ⚠️ Mount command failed: " << cmd << "\n";
        return false;
    }
}
bool USBUtils::unmountUSB(const std::string& mountPath) {
    if (mountPath.empty() || !fs::exists(mountPath)) {
        std::cerr << "[USBUtils] ⚠️ Invalid mount path for unmount: " << mountPath << "\n";
        return false;
    }

    std::string cmd = "sudo umount " + mountPath + " >/dev/null 2>&1";
    int ret = std::system(cmd.c_str());
    if (ret == 0) {
        std::cout << "[USBUtils] ✅ USB safely unmounted: " << mountPath << "\n";
        return true;
    } else {
        std::cerr << "[USBUtils] ❌ Failed to unmount: " << mountPath << "\n";
        return false;
    }
}

