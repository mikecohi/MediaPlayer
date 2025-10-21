#pragma once
#include <string>

/**
 * @class DeviceConnector
 * @brief Handles serial/UART communication with the S32K144 board.
 * (Skeleton implementation for Giai đoạn 2/3).
 */
class DeviceConnector {
public:
    DeviceConnector();
    ~DeviceConnector();

    /**
     * @brief Tries to connect to the hardware device.
     * @param port e.g., "/dev/ttyUSB0"
     * @return true on success.
     */
    bool connect(const std::string& port);

    /**
     * @brief Sends a string of data to the device.
     */
    void sendData(const std::string& data);

    /**
     * @brief Tries to read data from the device (non-blocking).
     * @return Data string, or empty string if no data.
     */
    std::string readData();
};