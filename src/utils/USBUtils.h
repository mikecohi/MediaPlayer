#ifndef USB_UTILS_H
#define USB_UTILS_H

#include <string>

class USBUtils {
public:
    USBUtils() = default;
    ~USBUtils() = default;

 // --- API cũ: giữ nguyên ---
    static std::string detectWSLUSBDrive();
    static bool mountWSLDrive(const std::string& driveLetter, const std::string& mountPath);
    static bool reloadUSBLibrary(const std::string& mountPath);
    static bool ejectUSB(const std::string& mountPath);

    // --- API mới ---
    static bool isWSLEnvironment();
    static std::string detectUSBPath();  // 🔹 trả về đường dẫn USB mount thực tế (Linux hoặc WSL)
    static bool isMounted(const std::string& path);
};


#endif
