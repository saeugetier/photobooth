#ifndef YOLO11SEG_H
#define YOLO11SEG_H

// Original Author: Abdalrahman M. Amer, www.linkedin.com/in/abdalrahman-m-amer
// Date: 25.01.2025
// Modified for use in photbooth

#include <onnxruntime_cxx_api.h>
#include <opencv2/opencv.hpp>

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <memory>
#include <numeric>
#include <random>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include <QDebug>

// ============================================================================
// Constants / Thresholds
// ============================================================================
static const float CONFIDENCE_THRESHOLD = 0.40f; // Filter boxes below this confidence
static const float IOU_THRESHOLD        = 0.45f; // NMS IoU threshold
static const float MASK_THRESHOLD       = 0.40f; // Slightly lower to capture partial objects

// ============================================================================
// Structs
// ============================================================================
struct BoundingBox {
    int x{0};
    int y{0};
    int width{0};
    int height{0};

    BoundingBox() = default;
    BoundingBox(int _x, int _y, int w, int h)
        : x(_x), y(_y), width(w), height(h) {}

    float area() const { return static_cast<float>(width * height); }

    BoundingBox intersect(const BoundingBox &other) const {
        int xStart = std::max(x, other.x);
        int yStart = std::max(y, other.y);
        int xEnd   = std::min(x + width,  other.x + other.width);
        int yEnd   = std::min(y + height, other.y + other.height);
        int iw     = std::max(0, xEnd - xStart);
        int ih     = std::max(0, yEnd - yStart);
        return BoundingBox(xStart, yStart, iw, ih);
    }
};

struct Segmentation {
    BoundingBox box;
    float       conf{0.f};
    int         classId{0};
    cv::Mat     mask;  // Single-channel (8UC1) mask in full resolution
};

// ============================================================================
// YOLOv11SegDetector Class
// ============================================================================
class YOLOv11SegDetector {
public:
    YOLOv11SegDetector(const std::string &modelPath,
                       const std::string &labelsPath,
                       bool useGPU = false);

    // Main API
    std::vector<Segmentation> segment(const cv::Mat &image,
                                      float confThreshold = CONFIDENCE_THRESHOLD,
                                      float iouThreshold  = IOU_THRESHOLD);

    // Draw results
    void drawSegmentationsAndBoxes(cv::Mat &image,
                                   const std::vector<Segmentation> &results,
                                   float maskAlpha = 0.5f) const;

    void drawSegmentations(cv::Mat &image,
                           const std::vector<Segmentation> &results,
                           float maskAlpha = 0.5f) const;

    void drawSegmentationMask(cv::Mat &image,
                           const std::vector<Segmentation> &results,
                           const std::vector<int> &classesFilter) const;
    // Accessors
    const std::vector<std::string> &getClassNames()  const { return classNames;  }
    const std::vector<cv::Scalar>  &getClassColors() const { return classColors; }

private:
    Ort::Env           env;
    Ort::SessionOptions sessionOptions;
    Ort::Session       session{nullptr};

    bool     isDynamicInputShape{false};
    cv::Size inputImageShape;

    std::vector<Ort::AllocatedStringPtr> inputNameAllocs;
    std::vector<const char*>             inputNames;
    std::vector<Ort::AllocatedStringPtr> outputNameAllocs;
    std::vector<const char*>             outputNames;

    size_t numInputNodes  = 0;
    size_t numOutputNodes = 0;

    std::vector<std::string> classNames;
    std::vector<cv::Scalar>  classColors;

    // Helpers
    cv::Mat preprocess(const cv::Mat &image,
                       float *&blobPtr,
                       std::vector<int64_t> &inputTensorShape);

    std::vector<Segmentation> postprocess(const cv::Size &origSize,
                                          const cv::Size &letterboxSize,
                                          const std::vector<Ort::Value> &outputs,
                                          float confThreshold,
                                          float iouThreshold);
};


#endif // YOLO11SEG_H
