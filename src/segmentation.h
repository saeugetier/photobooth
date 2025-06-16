#ifndef SEGMENTATION_H
#define SEGMENTATION_H

#include <opencv2/core.hpp>
#include <vector>

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

#endif // SEGMENTATION_H
