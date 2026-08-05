#ifndef YOLO11SEGRKNN_H
#define YOLO11SEGRKNN_H

#include <rknn_api.h>
#include <opencv2/opencv.hpp>

#include <string>
#include <vector>

#include <QDebug>

#include "segmentation.h"

class YOLOv11SegDetectorRknn : public Yolo11Segementation {
public:
    YOLOv11SegDetectorRknn(const std::string &modelPath, const std::string &labelsPath);
    ~YOLOv11SegDetectorRknn();

    std::vector<Segmentation> segment(const cv::Mat &image,
                                      float confThreshold = CONFIDENCE_THRESHOLD,
                                      float iouThreshold  = IOU_THRESHOLD);
private:
    rknn_context ctx{0};
    cv::Size inputImageShape{640, 640};
    uint32_t numInputNodes{0};
    uint32_t numOutputNodes{0};
    std::vector<rknn_tensor_attr> outputAttrs;

    cv::Mat preprocess(const cv::Mat &image);

    std::vector<Segmentation> postprocess(const cv::Size &origSize,
                                          const cv::Size &letterboxSize,
                                          const float *output0,
                                          const std::vector<int64_t> &shape0,
                                          const float *output1,
                                          const std::vector<int64_t> &shape1,
                                          float confThreshold,
                                          float iouThreshold);
};

#endif // YOLO11SEGRKNN_H
