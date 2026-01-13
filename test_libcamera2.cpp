#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <csignal>

#include <libcamera/libcamera.h>

using namespace libcamera;

volatile sig_atomic_t got_signal = 0;

void signalHandler(int sig) {
    got_signal = sig;
}

int main()
{
    signal(SIGSEGV, signalHandler);
    signal(SIGBUS, signalHandler);
    
    std::cerr << "Step 1: Creating CameraManager" << std::endl;
    
    auto cm = std::make_unique<CameraManager>();

    std::cerr << "Step 2: Starting CameraManager" << std::endl;
    
    int ret = cm->start();
    if (ret) {
        std::cerr << "Failed to start: " << ret << std::endl;
        return 1;
    }

    std::cerr << "Step 3: CameraManager started, version: " << cm->version() << std::endl;
    
    // Small delay to let background initialization complete
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    std::cerr << "Step 4: Getting cameras" << std::endl;
    
    if (got_signal) {
        std::cerr << "Signal caught: " << got_signal << std::endl;
        return 1;
    }
    
    auto cameras = cm->cameras();
    
    std::cerr << "Step 5: Got " << cameras.size() << " cameras" << std::endl;

    if (got_signal) {
        std::cerr << "Signal caught: " << got_signal << std::endl;
        return 1;
    }

    for (size_t i = 0; i < cameras.size(); i++) {
        std::cerr << "Step 6." << i << ": Camera " << cameras[i]->id() << std::endl;
    }

    std::cerr << "Step 7: Sleeping before stop" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cerr << "Step 8: Stopping CameraManager" << std::endl;
    cm->stop();

    std::cerr << "Step 9: Releasing CameraManager" << std::endl;
    cm.reset();

    std::cerr << "Step 10: Done!" << std::endl;

    return cameras.empty() ? 1 : 0;
}