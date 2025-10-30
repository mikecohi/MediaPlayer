#include "USBUtils.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <algorithm>

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

std::string USBUtils::detectWSLUSBDrive() {
    const char* cmd =
    "/mnt/c/Windows/System32/WindowsPowerShell/v1.0/powershell.exe "
    "-Command \"(Get-Volume | Where-Object {\\$_.DriveType -eq 'Removable'} | Select -ExpandProperty DriveLetter)\"";


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

    // Xóa BOM UTF-8 nếu có (0xEF,0xBB,0xBF)
    if (!result.empty() && (unsigned char)result[0] == 0xEF) {
        result.erase(0, 3);
    }

    // Xóa whitespace thừa: \n \r \t space
    result.erase(std::remove_if(result.begin(), result.end(), ::isspace), result.end());

    if (result.empty()) {
        std::cerr << "[USBUtils] ⚠️ PowerShell returned no drive letter\n";
        return "";
    }

    return result + ":"; // Ví dụ "F:"
}

std::string USBUtils::detectUSBMount() {
    if (isRunningOnWSL()) {
        std::string mountPath = "usb";

        // Tạo thư mục mount nếu chưa có
        // if (!fs::exists(mountPath)) {
        //     std::cout<<"in"<<std::endl;
        //     fs::create_directories(mountPath);
        //     std::cout << "[USBUtils] Created WSL USB mount folder: " << mountPath << "\n";
        // }
        // std::cout<<"next1"<<std::endl;
        // ✅ Dò đúng removable USB drive bằng PowerShell (không nhầm với HDD/SSD khác)
        std::string detectedDrive = detectWSLUSBDrive();
        if (detectedDrive.empty()) {
            std::cerr << "[USBUtils] ⚠️ No removable USB drive detected in WSL.\n";
            return "";
        }

        std::cout << "[USBUtils] ✅ USB drive detected: " << detectedDrive << "\n";

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

    std::string cmd = "sudo -n umount " + mountPath + " >/dev/null 2>&1";
    int ret = std::system(cmd.c_str());
    if (ret == 0) {
        std::cout << "[USBUtils] ✅ USB safely unmounted: " << mountPath << "\n";
        return true;
    } else {
        std::cerr << "[USBUtils] ❌ Failed to unmount: " << mountPath << "\n";
        return false;
    }
}

