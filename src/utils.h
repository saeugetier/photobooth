#ifndef UTILS_H_
#define UTILS_H_

#include <opencv2/opencv.hpp>
#include <algorithm>
#include <string>
#include <vector>
#include <QDebug>

namespace utils {

    template <typename T>
    T clamp(const T &val, const T &low, const T &high) {
        return std::max(low, std::min(val, high));
    }
    
    inline std::vector<std::string> getClassNames(const std::string &path) {
        std::vector<std::string> classNames;
        QFile f(path.c_str());
        if (!f.open(QIODevice::ReadOnly)) {
            qWarning() << "[ERROR] Could not open class names file: " << path.c_str();
            return classNames;
        }
        std::string line;
        while (!(line = f.readLine()).empty()) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            classNames.push_back(line);
        }
        qDebug() << "Loaded " << classNames.size() << " class names from " << path;
        return classNames;
    }
    
    inline size_t vectorProduct(const std::vector<int64_t> &shape) {
        return std::accumulate(shape.begin(), shape.end(), 1ull, std::multiplies<size_t>());
    }
    
    inline void letterBox(const cv::Mat &image,
                          cv::Mat &outImage,
                          const cv::Size &newShape,
                          const cv::Scalar &color     = cv::Scalar(114, 114, 114),
                          bool auto_       = true,
                          bool scaleFill   = false,
                          bool scaleUp     = true,
                          int stride       = 32) {
        float r = std::min((float)newShape.height / (float)image.rows,
                           (float)newShape.width  / (float)image.cols);
        if (!scaleUp) {
            r = std::min(r, 1.0f);
        }
    
        int newW = static_cast<int>(std::round(image.cols * r));
        int newH = static_cast<int>(std::round(image.rows * r));
    
        int dw = newShape.width  - newW;
        int dh = newShape.height - newH;
    
        if (auto_) {
            dw = dw % stride;
            dh = dh % stride;
        }
        else if (scaleFill) {
            newW = newShape.width;
            newH = newShape.height;
            dw = 0;
            dh = 0;
        }
    
        cv::Mat resized;
        cv::resize(image, resized, cv::Size(newW, newH), 0, 0, cv::INTER_LINEAR);
    
        int top = dh / 2;
        int bottom = dh - top;
        int left = dw / 2;
        int right = dw - left;
        cv::copyMakeBorder(resized, outImage, top, bottom, left, right, cv::BORDER_CONSTANT, color);
    }
    
    inline BoundingBox scaleCoords(const cv::Size &letterboxShape,
                                   const BoundingBox &coords,
                                   const cv::Size &originalShape,
                                   bool p_Clip = true) {
        float gain = std::min((float)letterboxShape.height / (float)originalShape.height,
                              (float)letterboxShape.width  / (float)originalShape.width);
    
        int padW = static_cast<int>(std::round(((float)letterboxShape.width  - (float)originalShape.width  * gain) / 2.f));
        int padH = static_cast<int>(std::round(((float)letterboxShape.height - (float)originalShape.height * gain) / 2.f));
    
        BoundingBox ret;
        ret.x      = static_cast<int>(std::round(((float)coords.x      - (float)padW) / gain));
        ret.y      = static_cast<int>(std::round(((float)coords.y      - (float)padH) / gain));
        ret.width  = static_cast<int>(std::round((float)coords.width   / gain));
        ret.height = static_cast<int>(std::round((float)coords.height  / gain));
    
        if (p_Clip) {
            ret.x = clamp(ret.x, 0, originalShape.width);
            ret.y = clamp(ret.y, 0, originalShape.height);
            ret.width  = clamp(ret.width,  0, originalShape.width  - ret.x);
            ret.height = clamp(ret.height, 0, originalShape.height - ret.y);
        }
    
        return ret;
    }
    
    inline std::vector<cv::Scalar> generateColors(const std::vector<std::string> &classNames, int seed = 42) {
        static std::unordered_map<size_t, std::vector<cv::Scalar>> cache;
        size_t key = 0;
        for (const auto &name : classNames) {
            size_t h = std::hash<std::string>{}(name);
            key ^= (h + 0x9e3779b9 + (key << 6) + (key >> 2));
        }
        auto it = cache.find(key);
        if (it != cache.end()) {
            return it->second;
        }
        std::mt19937 rng(seed);
        std::uniform_int_distribution<int> dist(0, 255);
        std::vector<cv::Scalar> colors;
        colors.reserve(classNames.size());
        for (size_t i = 0; i < classNames.size(); ++i) {
            colors.emplace_back(cv::Scalar(dist(rng), dist(rng), dist(rng)));
        }
        cache[key] = colors;
        return colors;
    }
    
    
    
    inline cv::Mat sigmoid(const cv::Mat& src) {
        cv::Mat dst;
        cv::exp(-src, dst);
        dst = 1.0 / (1.0 + dst);
        return dst;
    }

    inline void NMSBoxes(const std::vector<BoundingBox> &boxes,
                         const std::vector<float> &scores,
                         float scoreThreshold,
                         float nmsThreshold,
                         std::vector<int> &indices) {
        indices.clear();
        if (boxes.empty()) {
            return;
        }
    
        std::vector<int> order;
        order.reserve(boxes.size());
        for (size_t i = 0; i < boxes.size(); ++i) {
            if (scores[i] >= scoreThreshold) {
                order.push_back((int)i);
            }
        }
        if (order.empty()) return;
    
        std::sort(order.begin(), order.end(),
                  [&scores](int a, int b) {
                      return scores[a] > scores[b];
                  });
    
        std::vector<float> areas(boxes.size());
        for (size_t i = 0; i < boxes.size(); ++i) {
            areas[i] = (float)(boxes[i].width * boxes[i].height);
        }
    
        std::vector<bool> suppressed(boxes.size(), false);
        for (size_t i = 0; i < order.size(); ++i) {
            int idx = order[i];
            if (suppressed[idx]) continue;
    
            indices.push_back(idx);
    
            for (size_t j = i + 1; j < order.size(); ++j) {
                int idx2 = order[j];
                if (suppressed[idx2]) continue;
    
                const BoundingBox &a = boxes[idx];
                const BoundingBox &b = boxes[idx2];
                int interX1 = std::max(a.x, b.x);
                int interY1 = std::max(a.y, b.y);
                int interX2 = std::min(a.x + a.width,  b.x + b.width);
                int interY2 = std::min(a.y + a.height, b.y + b.height);
    
                int w = interX2 - interX1;
                int h = interY2 - interY1;
                if (w > 0 && h > 0) {
                    float interArea = (float)(w * h);
                    float unionArea = areas[idx] + areas[idx2] - interArea;
                    float iou = (unionArea > 0.f)? (interArea / unionArea) : 0.f;
                    if (iou > nmsThreshold) {
                        suppressed[idx2] = true;
                    }
                }
            }
        }
    }
    
    } // namespace utils
    
#endif // UTILS_H_
