#ifndef USB_UTILS_H
#define USB_UTILS_H

#include <string>

class USBUtils {
public:
    USBUtils() = default;
    ~USBUtils() = default;

    bool isRunningOnWSL();
    bool isDeviceRemovable(const std::string& deviceName);
    std::string detectUSBMount();
    std::string detectWSLUSBDrive();
    bool mountWSLDrive(const std::string& driveLetter, const std::string& mountPath);
    bool unmountUSB(const std::string& mountPath);
    
};



    

#endif
