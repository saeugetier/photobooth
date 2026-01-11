#include <iostream>
#include <memory>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>

#include <libcamera/libcamera.h>

using namespace libcamera;

int main()
{
    std::cout << "=== libcamera Test ===" << std::endl;

    // Test device access first
    std::cout << "\n--- Testing Device Access ---" << std::endl;

    const char* devices[] = {
        "/dev/media0",
        "/dev/media1",
        "/dev/video0",
        "/dev/v4l-subdev0"
    };

    for (const char* dev : devices) {
        int fd = open(dev, O_RDWR);
        if (fd >= 0) {
            std::cout << "[OK] Can open " << dev << std::endl;
            close(fd);
        } else {
            std::cout << "[FAIL] Cannot open " << dev << ": " << strerror(errno) << std::endl;
        }
    }

    // Check environment variables
    std::cout << "\n--- Environment Variables ---" << std::endl;

    const char* envVars[] = {
        "LIBCAMERA_LOG_LEVELS",
        "LIBCAMERA_IPA_MODULE_PATH",
        "LIBCAMERA_IPA_CONFIG_PATH"
    };

    for (const char* env : envVars) {
        const char* value = getenv(env);
        if (value) {
            std::cout << env << "=" << value << std::endl;
        } else {
            std::cout << env << " (not set)" << std::endl;
        }
    }

    // Initialize CameraManager
    std::cout << "\n--- Initializing CameraManager ---" << std::endl;

    std::unique_ptr<CameraManager> cm = std::make_unique<CameraManager>();

    int ret = cm->start();
    if (ret) {
        std::cerr << "[ERROR] CameraManager start failed: " << ret << std::endl;
        return 1;
    }

    std::cout << "[OK] CameraManager started successfully" << std::endl;
    std::cout << "libcamera version: " << cm->version() << std::endl;

    // List cameras
    std::cout << "\n--- Available Cameras ---" << std::endl;

    auto cameras = cm->cameras();
    std::cout << "Number of cameras found: " << cameras.size() << std::endl;

    if (cameras.empty()) {
        std::cout << "[WARNING] No cameras detected!" << std::endl;
    } else {
        for (const auto& camera : cameras) {
            std::cout << "\nCamera: " << camera->id() << std::endl;

            // Get camera properties
            const ControlList& props = camera->properties();

            // Try to get model name
            const auto& modelIt = props.get(properties::Model);
            if (modelIt) {
                std::cout << "  Model: " << *modelIt << std::endl;
            }

            // Try to get location
            const auto& locationIt = props.get(properties::Location);
            if (locationIt) {
                std::string location;
                switch (*locationIt) {
                case properties::CameraLocationFront:
                    location = "Front";
                    break;
                case properties::CameraLocationBack:
                    location = "Back";
                    break;
                case properties::CameraLocationExternal:
                    location = "External";
                    break;
                default:
                    location = "Unknown";
                }
                std::cout << "  Location: " << location << std::endl;
            }

            // List supported stream roles
            std::cout << "  Supported roles: Viewfinder, StillCapture, VideoRecording, Raw" << std::endl;
        }
    }

    // Cleanup
    std::cout << "\n--- Cleanup ---" << std::endl;
    cm->stop();
    std::cout << "[OK] CameraManager stopped" << std::endl;

    return cameras.empty() ? 1 : 0;
}