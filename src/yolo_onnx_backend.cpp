#include "yolo_onnx_backend.h"
#include <fstream>
#include <sstream>
#include <algorithm>

static std::vector<std::string> loadLabels(const std::string& path) {
    std::vector<std::string> labels;
    std::ifstream file(path);
    std::string line;
    while (std::getline(file, line)) {
        labels.push_back(line);
    }
    return labels;
}

YoloOnnxBackend::YoloOnnxBackend(const std::string& modelPath, const std::string& labelsPath, bool useGPU)
    : env(ORT_LOGGING_LEVEL_WARNING, "YOLOv11"),
      session(nullptr),
      sessionOptions()
{
    if (useGPU) {
        // Add CUDA or other provider if needed
        // sessionOptions.AppendExecutionProvider_CUDA(0);
    }
    sessionOptions.SetIntraOpNumThreads(1);
    sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);
    session = Ort::Session(env, modelPath.c_str(), sessionOptions);
    classNames = loadLabels(labelsPath);
}

std::vector<Segmentation> YoloOnnxBackend::segment(const cv::Mat &image, float confThreshold, float iouThreshold) {
    // Preprocess
    cv::Mat resized;
    cv::resize(image, resized, cv::Size(inputWidth, inputHeight));
    cv::Mat rgb;
    cv::cvtColor(resized, rgb, cv::COLOR_BGR2RGB);
    rgb.convertTo(rgb, CV_32F, 1.0 / 255);

    // HWC to CHW
    std::vector<float> inputTensorValues(inputWidth * inputHeight * 3);
    std::vector<cv::Mat> chw(3);
    for (int i = 0; i < 3; ++i)
        chw[i] = cv::Mat(inputHeight, inputWidth, CV_32F, inputTensorValues.data() + i * inputWidth * inputHeight);
    cv::split(rgb, chw);

    // Create input tensor
    std::vector<int64_t> inputShape = {1, 3, inputHeight, inputWidth};
    Ort::MemoryInfo memInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
        memInfo, inputTensorValues.data(), inputTensorValues.size(), inputShape.data(), inputShape.size());

    // Run inference
    std::vector<const char*> inputNames = {"images"}; // adjust if needed
    std::vector<const char*> outputNames = {"output0", "output1"}; // adjust if needed
    auto outputs = session.Run(Ort::RunOptions{nullptr}, inputNames.data(), &inputTensor, 1, outputNames.data(), 2);

    // Postprocess
    return postprocess(image, outputs, confThreshold, iouThreshold);
}

std::vector<Segmentation> YoloOnnxBackend::postprocess(const cv::Mat& origImage, const std::vector<Ort::Value>& outputs, float confThreshold, float iouThreshold) {
    std::vector<Segmentation> results;
    int origW = origImage.cols;
    int origH = origImage.rows;

    // Assume output0: [num, 6] (x, y, w, h, conf, class), output1: [num, mask_dim, mask_dim]
    const float* boxes = outputs[0].GetTensorData<float>();
    const float* masks = outputs[1].GetTensorData<float>();
    auto boxesShape = outputs[0].GetTensorTypeAndShapeInfo().GetShape();
    auto masksShape = outputs[1].GetTensorTypeAndShapeInfo().GetShape();
    int num = boxesShape[0];
    int mask_dim = masksShape[2];

    for (int i = 0; i < num; ++i) {
        float conf = boxes[i * 6 + 4];
        int classId = static_cast<int>(boxes[i * 6 + 5]);
        if (conf < confThreshold) continue;

        float x = boxes[i * 6 + 0];
        float y = boxes[i * 6 + 1];
        float w = boxes[i * 6 + 2];
        float h = boxes[i * 6 + 3];

        int x0 = std::max(int((x - w/2) * origW / inputWidth), 0);
        int y0 = std::max(int((y - h/2) * origH / inputHeight), 0);
        int x1 = std::min(int((x + w/2) * origW / inputWidth), origW-1);
        int y1 = std::min(int((y + h/2) * origH / inputHeight), origH-1);

        BoundingBox box(x0, y0, x1-x0, y1-y0);

        // Mask: [num, mask_dim, mask_dim]
        cv::Mat mask_small(mask_dim, mask_dim, CV_32F, (void*)(masks + i * mask_dim * mask_dim));
        cv::Mat mask;
        cv::resize(mask_small, mask, cv::Size(box.width, box.height));
        mask = mask > 0.5f;
        mask.convertTo(mask, CV_8UC1, 255);

        Segmentation seg;
        seg.box = box;
        seg.conf = conf;
        seg.classId = classId;
        seg.mask = cv::Mat::zeros(origH, origW, CV_8UC1);
        if (!mask.empty()) {
            mask.copyTo(seg.mask(cv::Rect(box.x, box.y, box.width, box.height)));
        }
        results.push_back(seg);
    }
    return results;
}