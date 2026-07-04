#ifndef SEGMENTATION_H
#define SEGMENTATION_H

#include <opencv2/core.hpp>
#include <vector>
#include <string>

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

class Yolo11Segementation
{
public:
    Yolo11Segementation(const std::string &labelsPath);

    virtual std::vector<Segmentation> segment(const cv::Mat &image,
                                      float confThreshold = CONFIDENCE_THRESHOLD,
                                              float iouThreshold  = IOU_THRESHOLD) = 0;

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
protected:
    std::vector<std::string> classNames;
    std::vector<cv::Scalar>  classColors;

    std::string getModelRessourcePath(const std::string &filename) const;

    static constexpr float CONFIDENCE_THRESHOLD = 0.40f; // Filter boxes below this confidence
    static constexpr float IOU_THRESHOLD        = 0.45f; // NMS IoU threshold
    static constexpr float MASK_THRESHOLD       = 0.40f; // Slightly lower to capture partial objects
};

#endif // SEGMENTATION_H
