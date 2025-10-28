#include "app/App.h"
#include <iostream>
#include <fstream>
#include <clocale>
#include <csignal> 
#include <ncurses.h>
void cleanupNcurses(int sig) {
    // TẮT NCURSES ĐỂ TRẢ LẠI TERMINAL
    endwin(); 
    
    // (Tùy chọn) In thông báo lỗi ra terminal (giờ đã bình thường)
    std::cerr << "Caught signal " << sig << ". Restoring terminal." << std::endl;
    std::cerr << "CRASH! Check debug.log for details before the crash." << std::endl;
    
    // Tự gửi lại tín hiệu để chương trình crash (và tạo core dump)
    signal(sig, SIG_DFL); // Đặt lại về trình xử lý mặc định
    raise(sig);           // Gửi lại tín hiệu
}

int main() {
    // --- SET LOCALE FIRST ---
    if (setlocale(LC_ALL, "") == NULL) {
         std::cerr << "Warning: Could not set locale." << std::endl;
    }

    signal(SIGSEGV, cleanupNcurses); // Lỗi Segmentation Fault (lỗi của bạn)
    signal(SIGABRT, cleanupNcurses); // Lỗi Abort
    signal(SIGINT, cleanupNcurses);  // Lỗi Ctrl+C
    signal(SIGTERM, cleanupNcurses); // Lỗi Terminate
    
    
    // --- REDIRECT OUTPUT TO FILE ---
    std::ofstream log_file("debug.log"); // Create/overwrite debug.log
    if (!log_file.is_open()) {
        std::cerr << "FATAL: Could not open debug.log for writing!" << std::endl;
        return 1; // Exit if log file fails
    }
    // Store original buffers (optional, but good practice)
    auto cout_buf = std::cout.rdbuf();
    auto cerr_buf = std::cerr.rdbuf();
    // Redirect std::cout and std::cerr to the file's buffer
    std::cout.rdbuf(log_file.rdbuf());
    std::cerr.rdbuf(log_file.rdbuf());
    // --- END REDIRECTION ---

    // Now, any std::cout or std::cerr will go to debug.log

    std::cout << "--- Application Started ---" << std::endl; // This goes to the file
{
    App myApp;

    // 1. Initialize
    if (!myApp.init()) {
        // This error message will now go to debug.log
        std::cerr << "Application failed to initialize." << std::endl;
        
        // Restore buffers before exiting to see error on console if needed
        std::cout.rdbuf(cout_buf);
        std::cerr.rdbuf(cerr_buf);
        log_file.close(); // Close the log file
        std::cerr << "FATAL: App initialization failed. Check debug.log for details." << std::endl;
        return 1;
    }

    // 2. Run the application loop
    myApp.run();

    // 3. Shutdown (App destructor handles ncurses cleanup)
    std::cout << "--- Application Shutting Down ---" << std::endl; // Goes to file
}
    // Restore original buffers before program truly exits
    std::cout.rdbuf(cout_buf);
    std::cerr.rdbuf(cerr_buf);
    log_file.close(); // Close the log file

    std::cout << "Application finished. Check debug.log for details." << std::endl; // Goes to console
    return 0;
}