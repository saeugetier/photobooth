#include <iostream>
#include <memory>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <dirent.h>
#include <sys/ioctl.h>
#include <linux/media.h>
#include <linux/videodev2.h>

#include <libcamera/libcamera.h>

using namespace libcamera;

void listDirectory(const char* path) {
    DIR* dir = opendir(path);
    if (!dir) {
        std::cout << "  Cannot open " << path << ": " << strerror(errno) << std::endl;
        return;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_name[0] != '.') {
            std::cout << "  " << entry->d_name << std::endl;
        }
    }
    closedir(dir);
}

void testMediaDevice(const char* dev) {
    int fd = open(dev, O_RDWR);
    if (fd < 0) {
        std::cout << "[FAIL] Cannot open " << dev << ": " << strerror(errno) << std::endl;
        return;
    }
    
    struct media_device_info info;
    memset(&info, 0, sizeof(info));
    
    if (ioctl(fd, MEDIA_IOC_DEVICE_INFO, &info) < 0) {
        std::cout << "[FAIL] Cannot get device info for " << dev << ": " << strerror(errno) << std::endl;
        close(fd);
        return;
    }
    
    std::cout << "[OK] " << dev << ": " << info.driver << " - " << info.model << std::endl;
    
    // Enumerate entities
    struct media_entity_desc entity;
    memset(&entity, 0, sizeof(entity));
    entity.id = MEDIA_ENT_ID_FLAG_NEXT;
    
    std::cout << "  Entities:" << std::endl;
    while (ioctl(fd, MEDIA_IOC_ENUM_ENTITIES, &entity) == 0) {
        std::cout << "    [" << entity.id << "] " << entity.name << " (type: " << entity.type << ")" << std::endl;
        entity.id |= MEDIA_ENT_ID_FLAG_NEXT;
    }
    
    close(fd);
}

void testV4L2Device(const char* dev) {
    int fd = open(dev, O_RDWR);
    if (fd < 0) {
        std::cout << "[FAIL] Cannot open " << dev << ": " << strerror(errno) << std::endl;
        return;
    }
    
    struct v4l2_capability cap;
    memset(&cap, 0, sizeof(cap));
    
    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) {
        std::cout << "[FAIL] Cannot query " << dev << ": " << strerror(errno) << std::endl;
        close(fd);
        return;
    }
    
    std::cout << "[OK] " << dev << ": " << cap.card << " (" << cap.driver << ")" << std::endl;
    
    close(fd);
}

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

    // Check IPA files
    std::cout << "\n--- IPA Module Path ---" << std::endl;
    const char* ipaModulePath = getenv("LIBCAMERA_IPA_MODULE_PATH");
    if (ipaModulePath) {
        listDirectory(ipaModulePath);
        std::string ipaSubdir = std::string(ipaModulePath) + "/ipa";
        std::cout << "  Subdir " << ipaSubdir << ":" << std::endl;
        listDirectory(ipaSubdir.c_str());
    }

    std::cout << "\n--- IPA Config Path ---" << std::endl;
    const char* ipaConfigPath = getenv("LIBCAMERA_IPA_CONFIG_PATH");
    if (ipaConfigPath) {
        listDirectory(ipaConfigPath);
        std::string rpiPath = std::string(ipaConfigPath) + "/rpi";
        std::cout << "  Subdir " << rpiPath << ":" << std::endl;
        listDirectory(rpiPath.c_str());
        std::string pispPath = rpiPath + "/pisp";
        std::cout << "  Subdir " << pispPath << ":" << std::endl;
        listDirectory(pispPath.c_str());
    }

    // Test media devices with ioctls
    std::cout << "\n--- Media Device Details ---" << std::endl;
    testMediaDevice("/dev/media0");
    testMediaDevice("/dev/media1");

    // Test V4L2 devices
    std::cout << "\n--- V4L2 Device Details ---" << std::endl;
    testV4L2Device("/dev/video0");
    testV4L2Device("/dev/video1");

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