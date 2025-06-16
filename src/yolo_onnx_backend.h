#pragma once

#include "yolobackend.h"
#include "segmentation.h"
#include <onnxruntime_cxx_api.h>
#include <opencv2/imgproc.hpp>
#include <string>
#include <vector>

class YoloOnnxBackend : public YoloBackend {
public:
    YoloOnnxBackend(const std::string& modelPath, const std::string& labelsPath, bool useGPU = false);
    std::vector<Segmentation> segment(const cv::Mat &image, float confThreshold, float iouThreshold) override;

private:
    Ort::Env env;
    Ort::Session session;
    Ort::SessionOptions sessionOptions;
    std::vector<std::string> classNames;
    int inputWidth = 640;
    int inputHeight = 640;

    std::vector<Segmentation> postprocess(const cv::Mat& origImage, const std::vector<Ort::Value>& outputs, float confThreshold, float iouThreshold);
};
