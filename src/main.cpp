#include "app/App.h"
#include <iostream>

int main() {
    App myApp;

    // 1. Initialize
    if (!myApp.init()) {
        std::cerr << "Application failed to initialize." << std::endl;
        return 1;
    }

    // 2. Run
    myApp.run();

    // 3. Shutdown (handled by App's destructor)
    std::cout << "Application shutting down. Goodbye!" << std::endl;
    return 0;
}