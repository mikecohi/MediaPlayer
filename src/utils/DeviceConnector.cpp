#include "utils/DeviceConnector.h"
#include <iostream> // For logging

DeviceConnector::DeviceConnector() {
    // Constructor
    std::cout << "DeviceConnector: Skeleton created." << std::endl;
}

DeviceConnector::~DeviceConnector() {
    // Destructor
}

bool DeviceConnector::connect(const std::string& port) {
    std::cout << "DeviceConnector: (Skeleton) connect to " << port << std::endl;
    // (Logic thật sẽ mở file /dev/ttyUSB0 ở đây)
    return true; // Giả lập kết nối thành công
}

void DeviceConnector::sendData(const std::string& data) {
    std::cout << "DeviceConnector: (Skeleton) send: " << data << std::endl;
    // (Logic thật sẽ ghi vào file descriptor)
}

std::string DeviceConnector::readData() {
    // (Logic thật sẽ đọc từ file descriptor)
    return ""; // Giả lập không có dữ liệu
}