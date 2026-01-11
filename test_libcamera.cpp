#include <iostream>
#include <memory>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <dirent.h>
#include <fstream>
#include <sys/ioctl.h>
#include <linux/media.h>
#include <linux/videodev2.h>

#include <libcamera/libcamera.h>

using namespace libcamera;

void checkSysfs() {
    std::cout << "\n--- Sysfs Video4Linux Info ---" << std::endl;
    
    DIR* dir = opendir("/sys/class/video4linux");
    if (!dir) {
        std::cout << "Cannot open /sys/class/video4linux: " << strerror(errno) << std::endl;
        return;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_name[0] == '.')
            continue;
            
        std::string namePath = std::string("/sys/class/video4linux/") + entry->d_name + "/name";
        
        std::ifstream f(namePath);
        if (f.is_open()) {
            std::string name;
            std::getline(f, name);
            std::cout << "  " << entry->d_name << ": " << name << std::endl;
        }
    }
    closedir(dir);
}

bool checkMediaDeviceMatch(const char* devPath, const char* driverName, 
                           const char** requiredEntities, int numEntities) {
    int fd = open(devPath, O_RDWR);
    if (fd < 0) {
        std::cout << "  Cannot open " << devPath << std::endl;
        return false;
    }
    
    struct media_device_info info;
    memset(&info, 0, sizeof(info));
    
    if (ioctl(fd, MEDIA_IOC_DEVICE_INFO, &info) < 0) {
        close(fd);
        return false;
    }
    
    std::cout << "  " << devPath << " driver: '" << info.driver << "'" << std::endl;
    
    if (strcmp(info.driver, driverName) != 0) {
        std::cout << "    Driver mismatch: expected '" << driverName << "'" << std::endl;
        close(fd);
        return false;
    }
    
    // Check for required entities
    bool allFound = true;
    for (int i = 0; i < numEntities; i++) {
        bool found = false;
        
        struct media_entity_desc entity;
        memset(&entity, 0, sizeof(entity));
        entity.id = MEDIA_ENT_ID_FLAG_NEXT;
        
        while (ioctl(fd, MEDIA_IOC_ENUM_ENTITIES, &entity) == 0) {
            if (strcmp(entity.name, requiredEntities[i]) == 0) {
                found = true;
                break;
            }
            entity.id |= MEDIA_ENT_ID_FLAG_NEXT;
        }
        
        if (found) {
            std::cout << "    Entity '" << requiredEntities[i] << "': FOUND" << std::endl;
        } else {
            std::cout << "    Entity '" << requiredEntities[i] << "': NOT FOUND" << std::endl;
            allFound = false;
        }
    }
    
    close(fd);
    return allFound;
}

int main()
{
    std::cout << "=== libcamera CFE Device Match Test ===" << std::endl;

    // Check sysfs first
    checkSysfs();

    // Simulate what PiSP pipeline does to match CFE device
    std::cout << "\n--- Simulating PiSP CFE Device Match ---" << std::endl;
    std::cout << "PiSP looks for: driver='rp1-cfe', entities=['csi2', 'pisp-fe']" << std::endl;
    
    const char* cfeEntities[] = {"csi2", "pisp-fe"};
    
    for (int i = 0; i < 4; i++) {
        char devPath[32];
        snprintf(devPath, sizeof(devPath), "/dev/media%d", i);
        
        if (access(devPath, F_OK) == 0) {
            std::cout << "\nChecking " << devPath << ":" << std::endl;
            bool match = checkMediaDeviceMatch(devPath, "rp1-cfe", cfeEntities, 2);
            if (match) {
                std::cout << "  => MATCH for CFE!" << std::endl;
            }
        }
    }

    // Simulate what PiSP pipeline does to match ISP (pispbe) device  
    std::cout << "\n--- Simulating PiSP ISP Device Match ---" << std::endl;
    std::cout << "PiSP looks for: driver='pispbe', entities=['pispbe']" << std::endl;
    
    const char* ispEntities[] = {"pispbe"};
    
    for (int i = 0; i < 4; i++) {
        char devPath[32];
        snprintf(devPath, sizeof(devPath), "/dev/media%d", i);
        
        if (access(devPath, F_OK) == 0) {
            std::cout << "\nChecking " << devPath << ":" << std::endl;
            bool match = checkMediaDeviceMatch(devPath, "pispbe", ispEntities, 1);
            if (match) {
                std::cout << "  => MATCH for ISP!" << std::endl;
            }
        }
    }

    // Now test libcamera
    std::cout << "\n--- Initializing CameraManager ---" << std::endl;

    std::unique_ptr<CameraManager> cm = std::make_unique<CameraManager>();

    int ret = cm->start();
    if (ret) {
        std::cerr << "[ERROR] CameraManager start failed: " << ret << std::endl;
        return 1;
    }

    std::cout << "[OK] CameraManager started successfully" << std::endl;
    std::cout << "libcamera version: " << cm->version() << std::endl;

    auto cameras = cm->cameras();
    std::cout << "\n--- Available Cameras ---" << std::endl;
    std::cout << "Number of cameras found: " << cameras.size() << std::endl;

    if (cameras.empty()) {
        std::cout << "[WARNING] No cameras detected!" << std::endl;
    } else {
        for (const auto& camera : cameras) {
            std::cout << "\nCamera: " << camera->id() << std::endl;
            const ControlList& props = camera->properties();
            const auto& modelIt = props.get(properties::Model);
            if (modelIt) {
                std::cout << "  Model: " << *modelIt << std::endl;
            }
        }
    }

    cm->stop();
    std::cout << "\n[OK] CameraManager stopped" << std::endl;

    return cameras.empty() ? 1 : 0;
}