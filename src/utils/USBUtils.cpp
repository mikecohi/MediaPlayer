#include "utils/USBUtils.h"
#include <iostream>

USBUtils::USBUtils() {
    // Constructor
    std::cout << "USBUtils: Skeleton created." << std::endl;
}

std::vector<std::string> USBUtils::findHotpluggableDrives() {
    std::cout << "USBUtils: (Skeleton) finding drives..." << std::endl;
    // (Logic thật sẽ dùng udev hoặc quét /sys/block)
    return {}; // Trả về vector rỗng
}

std::string USBUtils::mountDrive(const std::string& devicePath) {
    std::cout << "USBUtils: (Skeleton) mounting " << devicePath << std::endl;
    // (Logic thật sẽ gọi system("mount ..."))
    return "/media/mock_usb"; // Giả lập mount thành công
}

void USBUtils::unmountDrive(const std::string& mountPoint) {
    std::cout << "USBUtils: (Skeleton) unmounting " << mountPoint << std::endl;
    // (Logic thật sẽ gọi system("umount ..."))
}