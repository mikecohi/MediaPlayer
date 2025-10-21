#pragma once
#include <string>
#include <vector>

/**
 * @class USBUtils
 * @brief Handles detection and mounting of USB storage devices.
 * (Skeleton implementation for Giai đoạn 2/3).
 */
class USBUtils {
public:
    USBUtils();
    
    /**
     * @brief Scans the system for unmounted USB storage.
     * @return A vector of device paths (e.g., "/dev/sdb1").
     */
    std::vector<std::string> findHotpluggableDrives();

    /**
     * @brief Tries to mount a drive to a standard location.
     * @param devicePath e.g., "/dev/sdb1"
     * @return The mount point path (e.g., "/media/my_usb"), or empty on failure.
     */
    std::string mountDrive(const std::string& devicePath);

    /**
     * @brief Unmounts a drive.
     */
    void unmountDrive(const std::string& mountPoint);
};