#ifndef YOLO11SEGNCNN_H
#define YOLO11SEGNCNN_H

// Original Author: Abdalrahman M. Amer, www.linkedin.com/in/abdalrahman-m-amer
// Date: 25.01.2025
// Modified for use in photbooth

#include <ncnn/net.h>

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
class YOLOv11SegDetectorNcnn {
public:
    YOLOv11SegDetectorNcnn(const std::string &modelPath,
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
    ncnn::Net net;

    bool     isDynamicInputShape{false};
    cv::Size inputImageShape;

    std::vector<const char*>             inputNames;
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
                                          const ncnn::Mat &outputs_boxes,
                                          const ncnn::Mat &outputs_masks,
                                          float confThreshold,
                                          float iouThreshold);
};


#endif // YOLO11SEG_H
