#ifndef YOLOBACKEND_H
#define YOLOBACKEND_H


#include "segmentation.h"

class YoloBackend {
public:
    virtual ~YoloBackend() = default;
    virtual std::vector<Segmentation> segment(const cv::Mat &image, float confThreshold, float iouThreshold) = 0;
};

#endif // YOLOBACKEND_H
