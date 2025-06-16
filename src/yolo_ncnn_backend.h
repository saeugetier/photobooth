#pragma once

#include "yolobackend.h"
#include "segmentation.h"
#include <ncnn/net.h>
#include <opencv2/imgproc.hpp>
#include <string>
#include <vector>

class YoloNcnnBackend : public YoloBackend {
public:
    YoloNcnnBackend(const std::string& paramPath, const std::string& binPath, const std::string& labelsPath);
    std::vector<Segmentation> segment(const cv::Mat &image, float confThreshold, float iouThreshold) override;

private:
    ncnn::Net net;
    std::vector<std::string> classNames;
    int inputWidth = 640;
    int inputHeight = 640;

    std::vector<Segmentation> postprocess(const cv::Mat& image, const ncnn::Mat& out_boxes, const ncnn::Mat& out_masks, float confThreshold, float iouThreshold);
};