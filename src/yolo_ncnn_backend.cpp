#include "yolo_ncnn_backend.h"
#include <fstream>
#include <sstream>

static std::vector<std::string> loadLabels(const std::string& path) {
    std::vector<std::string> labels;
    std::ifstream file(path);
    std::string line;
    while (std::getline(file, line)) {
        labels.push_back(line);
    }
    return labels;
}

YoloNcnnBackend::YoloNcnnBackend(const std::string& paramPath, const std::string& binPath, const std::string& labelsPath) {
    net.load_param(paramPath.c_str());
    net.load_model(binPath.c_str());
    classNames = loadLabels(labelsPath);
}

std::vector<Segmentation> YoloNcnnBackend::segment(const cv::Mat &image, float confThreshold, float iouThreshold) {
    // Preprocess
    cv::Mat resized;
    cv::resize(image, resized, cv::Size(inputWidth, inputHeight));
    ncnn::Mat in = ncnn::Mat::from_pixels(resized.data, ncnn::Mat::PIXEL_BGR2RGB, inputWidth, inputHeight);

    // Inference
    ncnn::Extractor ex = net.create_extractor();
    ex.input("images", in);

    ncnn::Mat out_boxes, out_masks;
    ex.extract("output0", out_boxes); // shape: [num, 6] (x,y,w,h,conf,class)
    ex.extract("output1", out_masks); // shape: [num, mask_dim, mask_dim]

    // Postprocess
    return postprocess(image, out_boxes, out_masks, confThreshold, iouThreshold);
}

std::vector<Segmentation> YoloNcnnBackend::postprocess(const cv::Mat& origImage, const ncnn::Mat& out_boxes, const ncnn::Mat& out_masks, float confThreshold, float iouThreshold) {
    std::vector<Segmentation> results;
    int origW = origImage.cols;
    int origH = origImage.rows;

    for (int i = 0; i < out_boxes.h; ++i) {
        const float* values = out_boxes.row(i);
        float conf = values[4];
        int classId = static_cast<int>(values[5]);
        if (conf < confThreshold) continue;

        // Bounding box in resized image coordinates
        float x = values[0];
        float y = values[1];
        float w = values[2];
        float h = values[3];

        // Scale bbox to original image
        int x0 = std::max(int((x - w/2) * origW / inputWidth), 0);
        int y0 = std::max(int((y - h/2) * origH / inputHeight), 0);
        int x1 = std::min(int((x + w/2) * origW / inputWidth), origW-1);
        int y1 = std::min(int((y + h/2) * origH / inputHeight), origH-1);

        BoundingBox box(x0, y0, x1-x0, y1-y0);

        // Mask: out_masks[i] is [mask_dim, mask_dim], resize to box size
        cv::Mat mask;
        if (out_masks.dims == 3) {
            ncnn::Mat mask_ncnn = out_masks.channel(i);
            cv::Mat mask_small(mask_ncnn.h, mask_ncnn.w, CV_32FC1, (void*)mask_ncnn.data);
            cv::resize(mask_small, mask, cv::Size(box.width, box.height));
            mask = mask > 0.5f; // binarize
            mask.convertTo(mask, CV_8UC1, 255);
        }

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