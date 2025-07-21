#ifndef YOLO11SEGONNX_H
#define YOLO11SEGONNX_H

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

#include "segmentation.h"

// ============================================================================
// Constants / Thresholds
// ============================================================================
static const float CONFIDENCE_THRESHOLD = 0.40f; // Filter boxes below this confidence
static const float IOU_THRESHOLD        = 0.45f; // NMS IoU threshold
static const float MASK_THRESHOLD       = 0.40f; // Slightly lower to capture partial objects


// ============================================================================
// YOLOv11SegDetector Class
// ============================================================================
class YOLOv11SegDetectorOnnx : public Yolo11Segementation {
public:
    YOLOv11SegDetectorOnnx(const std::string &modelPath,
                       const std::string &labelsPath,
                       bool useGPU = false);

    // Main API
    std::vector<Segmentation> segment(const cv::Mat &image,
                                      float confThreshold = CONFIDENCE_THRESHOLD,
                                      float iouThreshold  = IOU_THRESHOLD);

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


#endif // YOLO11SEGONNX_H
