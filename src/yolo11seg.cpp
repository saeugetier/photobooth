#include "yolo11seg.h"
#include <QFile>

// Original Author: Abdalrahman M. Amer, www.linkedin.com/in/abdalrahman-m-amer
// Date: 25.01.2025
// Modified for use in photbooth

// ============================================================================
// Utility Namespace
// ============================================================================
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



cv::Mat sigmoid(const cv::Mat& src) {
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

YOLOv11SegDetector::YOLOv11SegDetector(const std::string &modelPath,
                                       const std::string &labelsPath,
                                       bool useGPU)
    : env(ORT_LOGGING_LEVEL_WARNING, "YOLOv11Seg")
{
    sessionOptions.SetIntraOpNumThreads(std::min(6, static_cast<int>(std::thread::hardware_concurrency())));
    sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

    std::vector<std::string> providers = Ort::GetAvailableProviders();
    if (useGPU && std::find(providers.begin(), providers.end(), "CUDAExecutionProvider") != providers.end()) {
        OrtCUDAProviderOptions cudaOptions;
        sessionOptions.AppendExecutionProvider_CUDA(cudaOptions);
        qDebug() << "[INFO] Using GPU (CUDA) for YOLOv11 Seg inference.";
    } else {
        qDebug() << "[INFO] Using CPU for YOLOv11 Seg inference.";
    }

    QFile modelFile(modelPath.c_str());

    if (!modelFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open the model file!";
    }

    QByteArray binaryData = modelFile.readAll();

#ifdef _WIN32
    std::wstring w_modelPath(modelPath.begin(), modelPath.end());
    session = Ort::Session(env, w_modelPath.c_str(), sessionOptions);
#else
    session = Ort::Session(env, binaryData.data(), binaryData.size(), sessionOptions);
#endif

    numInputNodes  = session.GetInputCount();
    numOutputNodes = session.GetOutputCount();

    Ort::AllocatorWithDefaultOptions allocator;

    // Input
    {
        auto inNameAlloc = session.GetInputNameAllocated(0, allocator);
        inputNameAllocs.emplace_back(std::move(inNameAlloc));
        inputNames.push_back(inputNameAllocs.back().get());

        auto inTypeInfo = session.GetInputTypeInfo(0);
        auto inShape    = inTypeInfo.GetTensorTypeAndShapeInfo().GetShape();

        if (inShape.size() == 4) {
            if (inShape[2] == -1 || inShape[3] == -1) {
                isDynamicInputShape = true;
                inputImageShape = cv::Size(640, 640); // Fallback if dynamic
            } else {
                inputImageShape = cv::Size(static_cast<int>(inShape[3]), static_cast<int>(inShape[2]));
            }
        } else {
            throw std::runtime_error("Model input is not 4D! Expect [N, C, H, W].");
        }
    }

    // Outputs
    if (numOutputNodes != 2) {
        throw std::runtime_error("Expected exactly 2 output nodes: output0 and output1.");
    }

    for (size_t i = 0; i < numOutputNodes; ++i) {
        auto outNameAlloc = session.GetOutputNameAllocated(i, allocator);
        outputNameAllocs.emplace_back(std::move(outNameAlloc));
        outputNames.push_back(outputNameAllocs.back().get());
    }

    classNames  = utils::getClassNames(labelsPath);
    classColors = utils::generateColors(classNames);

    qDebug() << "[INFO] YOLOv11Seg loaded: " << modelPath;
    qDebug() << "      Input shape: " << inputImageShape.height << "x" << inputImageShape.width
             << (isDynamicInputShape ? " (dynamic)" : "");
    qDebug() << "      #Outputs   : " << numOutputNodes;
    qDebug() << "      #Classes   : " << classNames.size();
}

cv::Mat YOLOv11SegDetector::preprocess(const cv::Mat &image,
                                       float *&blobPtr,
                                       std::vector<int64_t> &inputTensorShape)
{
    cv::Mat letterboxImage;
    utils::letterBox(image, letterboxImage, inputImageShape,
                     cv::Scalar(114,114,114), /*auto_=*/isDynamicInputShape,
                     /*scaleFill=*/false, /*scaleUp=*/true, /*stride=*/32);

    // Update if dynamic
    inputTensorShape[2] = static_cast<int64_t>(letterboxImage.rows);
    inputTensorShape[3] = static_cast<int64_t>(letterboxImage.cols);

    letterboxImage.convertTo(letterboxImage, CV_32FC3, 1.0f/255.0f);

    size_t size = static_cast<size_t>(letterboxImage.rows) * static_cast<size_t>(letterboxImage.cols) * 3;
    blobPtr = new float[size];

    std::vector<cv::Mat> channels(3);
    for (int c = 0; c < 3; ++c) {
        channels[c] = cv::Mat(letterboxImage.rows, letterboxImage.cols, CV_32FC1,
                              blobPtr + c * (letterboxImage.rows * letterboxImage.cols));
    }
    cv::split(letterboxImage, channels);

    return letterboxImage;
}

std::vector<Segmentation> YOLOv11SegDetector::postprocess(
    const cv::Size &origSize,
    const cv::Size &letterboxSize,
    const std::vector<Ort::Value> &outputs,
    float confThreshold,
    float iouThreshold)
{
    std::vector<Segmentation> results;

    // Validate outputs size
    if (outputs.size() < 2) {
        throw std::runtime_error("Insufficient outputs from the model. Expected at least 2 outputs.");
    }

    // Extract outputs
    const float* output0_ptr = outputs[0].GetTensorData<float>();
    const float* output1_ptr = outputs[1].GetTensorData<float>();

    // Get shapes
    auto shape0 = outputs[0].GetTensorTypeAndShapeInfo().GetShape(); // [1, 116, num_detections]
    auto shape1 = outputs[1].GetTensorTypeAndShapeInfo().GetShape(); // [1, 32, maskH, maskW]

    if (shape1.size() != 4 || shape1[0] != 1 || shape1[1] != 32)
        throw std::runtime_error("Unexpected output1 shape. Expected [1, 32, maskH, maskW].");

    const size_t num_features = shape0[1]; // e.g 80 class + 4 bbox parms + 32 seg masks = 116
    const size_t num_detections = shape0[2];

    // Early exit if no detections
    if (num_detections == 0)
    {
        return results;
    }

    const int numClasses = static_cast<int>(num_features - 4 - 32); // Corrected number of classes

    // Validate numClasses
    if (numClasses <= 0)
    {
        throw std::runtime_error("Invalid number of classes.");
    }

    const int numBoxes = static_cast<int>(num_detections);
    const int maskH = static_cast<int>(shape1[2]);
    const int maskW = static_cast<int>(shape1[3]);

    // Constants from model architecture
    constexpr int BOX_OFFSET = 0;
    constexpr int CLASS_CONF_OFFSET = 4;
    const int MASK_COEFF_OFFSET = numClasses + CLASS_CONF_OFFSET;

    // 1. Process prototype masks
    // Store all prototype masks in a vector for easy access
    std::vector<cv::Mat> prototypeMasks;
    prototypeMasks.reserve(32);
    for (int m = 0; m < 32; ++m) {
        // Each mask is maskH x maskW
        cv::Mat proto(maskH, maskW, CV_32F, const_cast<float*>(output1_ptr + m * maskH * maskW));
        prototypeMasks.emplace_back(proto.clone()); // Clone to ensure data integrity
    }

    // 2. Process detections
    std::vector<BoundingBox> boxes;
    boxes.reserve(numBoxes);
    std::vector<float> confidences;
    confidences.reserve(numBoxes);
    std::vector<int> classIds;
    classIds.reserve(numBoxes);
    std::vector<std::vector<float>> maskCoefficientsList;
    maskCoefficientsList.reserve(numBoxes);

    for (int i = 0; i < numBoxes; ++i) {
        // Extract box coordinates
        float xc = output0_ptr[BOX_OFFSET * numBoxes + i];
        float yc = output0_ptr[(BOX_OFFSET + 1) * numBoxes + i];
        float w = output0_ptr[(BOX_OFFSET + 2) * numBoxes + i];
        float h = output0_ptr[(BOX_OFFSET + 3) * numBoxes + i];

        // Convert to xyxy format
        BoundingBox box{
            static_cast<int>(std::round(xc - w / 2.0f)),
            static_cast<int>(std::round(yc - h / 2.0f)),
            static_cast<int>(std::round(w)),
            static_cast<int>(std::round(h))
        };

        // Get class confidence
        float maxConf = 0.0f;
        int classId = -1;
        for (int c = 0; c < numClasses; ++c) {
            float conf = output0_ptr[(CLASS_CONF_OFFSET + c) * numBoxes + i];
            if (conf > maxConf) {
                maxConf = conf;
                classId = c;
            }
        }

        if (maxConf < confThreshold) continue;

        // Store detection
        boxes.push_back(box);
        confidences.push_back(maxConf);
        classIds.push_back(classId);

        // Store mask coefficients
        std::vector<float> maskCoeffs(32);
        for (int m = 0; m < 32; ++m) {
            maskCoeffs[m] = output0_ptr[(MASK_COEFF_OFFSET + m) * numBoxes + i];
        }
        maskCoefficientsList.emplace_back(std::move(maskCoeffs));
    }

    // Early exit if no boxes after confidence threshold
    if (boxes.empty()) {
        return results;
    }

    // 3. Apply NMS
    std::vector<int> nmsIndices;
    utils::NMSBoxes(boxes, confidences, confThreshold, iouThreshold, nmsIndices);

    if (nmsIndices.empty()) {
        return results;
    }

    // 4. Prepare final results
    results.reserve(nmsIndices.size());

    // Calculate letterbox parameters
    const float gain = std::min(static_cast<float>(letterboxSize.height) / origSize.height,
                                static_cast<float>(letterboxSize.width) / origSize.width);
    const int scaledW = static_cast<int>(origSize.width * gain);
    const int scaledH = static_cast<int>(origSize.height * gain);
    const float padW = (letterboxSize.width - scaledW) / 2.0f;
    const float padH = (letterboxSize.height - scaledH) / 2.0f;

    // Precompute mask scaling factors
    const float maskScaleX = static_cast<float>(maskW) / letterboxSize.width;
    const float maskScaleY = static_cast<float>(maskH) / letterboxSize.height;

    for (const int idx : nmsIndices) {
        Segmentation seg;
        seg.box = boxes[idx];
        seg.conf = confidences[idx];
        seg.classId = classIds[idx];

        // 5. Scale box to original image
        seg.box = utils::scaleCoords(letterboxSize, seg.box, origSize, true);

        // 6. Process mask
        const auto& maskCoeffs = maskCoefficientsList[idx];

        // Linear combination of prototype masks
        cv::Mat finalMask = cv::Mat::zeros(maskH, maskW, CV_32F);
        for (int m = 0; m < 32; ++m) {
            finalMask += maskCoeffs[m] * prototypeMasks[m];
        }

        // Apply sigmoid activation
        finalMask = utils::sigmoid(finalMask);

        // Crop mask to letterbox area with a slight padding to avoid border issues
        int x1 = static_cast<int>(std::round((padW - 0.1f) * maskScaleX));
        int y1 = static_cast<int>(std::round((padH - 0.1f) * maskScaleY));
        int x2 = static_cast<int>(std::round((letterboxSize.width - padW + 0.1f) * maskScaleX));
        int y2 = static_cast<int>(std::round((letterboxSize.height - padH + 0.1f) * maskScaleY));

        // Ensure coordinates are within mask bounds
        x1 = std::max(0, std::min(x1, maskW - 1));
        y1 = std::max(0, std::min(y1, maskH - 1));
        x2 = std::max(x1, std::min(x2, maskW));
        y2 = std::max(y1, std::min(y2, maskH));

        // Handle cases where cropping might result in zero area
        if (x2 <= x1 || y2 <= y1) {
            // Skip this mask as cropping is invalid
            continue;
        }

        cv::Rect cropRect(x1, y1, x2 - x1, y2 - y1);
        cv::Mat croppedMask = finalMask(cropRect).clone(); // Clone to ensure data integrity

        // Resize to original dimensions
        cv::Mat resizedMask;
        cv::resize(croppedMask, resizedMask, origSize, 0, 0, cv::INTER_LINEAR);

        // Threshold and convert to binary
        cv::Mat binaryMask;
        cv::threshold(resizedMask, binaryMask, 0.5, 255.0, cv::THRESH_BINARY);
        binaryMask.convertTo(binaryMask, CV_8U);

        // Crop to bounding box
        cv::Mat finalBinaryMask = cv::Mat::zeros(origSize, CV_8U);
        cv::Rect roi(seg.box.x, seg.box.y, seg.box.width, seg.box.height);
        roi &= cv::Rect(0, 0, binaryMask.cols, binaryMask.rows); // Ensure ROI is within mask
        if (roi.area() > 0) {
            binaryMask(roi).copyTo(finalBinaryMask(roi));
        }

        seg.mask = finalBinaryMask;
        results.push_back(seg);
    }

    return results;
}

void YOLOv11SegDetector::drawSegmentationsAndBoxes(cv::Mat &image,
                                                   const std::vector<Segmentation> &results,
                                                   float maskAlpha) const
{
    for (const auto &seg : results) {
        if (seg.conf < CONFIDENCE_THRESHOLD) {
            continue;
        }
        cv::Scalar color = classColors[seg.classId % classColors.size()];

        // -----------------------------
        // 1. Draw Bounding Box
        // -----------------------------
        cv::rectangle(image,
                      cv::Point(seg.box.x, seg.box.y),
                      cv::Point(seg.box.x + seg.box.width, seg.box.y + seg.box.height),
                      color, 2);

        // -----------------------------
        // 2. Draw Label
        // -----------------------------
        std::string label = classNames[seg.classId] + " " + std::to_string(static_cast<int>(seg.conf * 100)) + "%";
        int baseLine = 0;
        double fontScale = 0.5;
        int thickness = 1;
        cv::Size labelSize = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, fontScale, thickness, &baseLine);
        int top = std::max(seg.box.y, labelSize.height + 5);
        cv::rectangle(image,
                      cv::Point(seg.box.x, top - labelSize.height - 5),
                      cv::Point(seg.box.x + labelSize.width + 5, top),
                      color, cv::FILLED);
        cv::putText(image, label,
                    cv::Point(seg.box.x + 2, top - 2),
                    cv::FONT_HERSHEY_SIMPLEX,
                    fontScale,
                    cv::Scalar(255, 255, 255),
                    thickness);

        // -----------------------------
        // 3. Apply Segmentation Mask
        // -----------------------------
        if (!seg.mask.empty()) {
            // Ensure the mask is single-channel
            cv::Mat mask_gray;
            if (seg.mask.channels() == 3) {
                cv::cvtColor(seg.mask, mask_gray, cv::COLOR_BGR2GRAY);
            } else {
                mask_gray = seg.mask.clone();
            }

            // Threshold the mask to binary (object: 255, background: 0)
            cv::Mat mask_binary;
            cv::threshold(mask_gray, mask_binary, 127, 255, cv::THRESH_BINARY);

            // Create a colored version of the mask
            cv::Mat colored_mask;
            cv::cvtColor(mask_binary, colored_mask, cv::COLOR_GRAY2BGR);
            colored_mask.setTo(color, mask_binary); // Apply color where mask is present

            // Blend the colored mask with the original image
            cv::addWeighted(image, 1.0, colored_mask, maskAlpha, 0, image);
        }
    }
}

void YOLOv11SegDetector::drawSegmentations(cv::Mat &image,
                                           const std::vector<Segmentation> &results,
                                           float maskAlpha) const
{
    for (const auto &seg : results) {
        if (seg.conf < CONFIDENCE_THRESHOLD) {
            continue;
        }
        cv::Scalar color = classColors[seg.classId % classColors.size()];

        // -----------------------------
        // Draw Segmentation Mask Only
        // -----------------------------
        if (!seg.mask.empty()) {
            // Ensure the mask is single-channel
            cv::Mat mask_gray;
            if (seg.mask.channels() == 3) {
                cv::cvtColor(seg.mask, mask_gray, cv::COLOR_BGR2GRAY);
            } else {
                mask_gray = seg.mask.clone();
            }

            // Threshold the mask to binary (object: 255, background: 0)
            cv::Mat mask_binary;
            cv::threshold(mask_gray, mask_binary, 127, 255, cv::THRESH_BINARY);

            // Create a colored version of the mask
            cv::Mat colored_mask;
            cv::cvtColor(mask_binary, colored_mask, cv::COLOR_GRAY2BGR);
            colored_mask.setTo(color, mask_binary); // Apply color where mask is present

            // Blend the colored mask with the original image
            cv::addWeighted(image, 1.0, colored_mask, maskAlpha, 0, image);
        }
    }
}

void YOLOv11SegDetector::drawSegmentationMask(cv::Mat &image,
                                           const std::vector<Segmentation> &results,
                                           const std::vector<int> &classesFilter) const
{
    for (const auto &seg : results) {
        if (seg.conf < CONFIDENCE_THRESHOLD) {
            continue;
        }

        if(!classesFilter.empty())
        {
            if(std::find(classesFilter.begin(), classesFilter.end(), seg.classId) == classesFilter.end())
            {
                // class id not in filter
                continue;
            }
        }

        // -----------------------------
        // Draw Segmentation Mask Only
        // -----------------------------
        if (!seg.mask.empty()) {
            // Ensure the mask is single-channel
            cv::Mat mask_gray;
            if (seg.mask.channels() == 3) {
                cv::cvtColor(seg.mask, mask_gray, cv::COLOR_BGR2GRAY);
            } else {
                mask_gray = seg.mask.clone();
                mask_gray *= 255;
            }

            // Threshold the mask to binary (object: 255, background: 0)
            cv::Mat mask_binary;
            cv::threshold(mask_gray, mask_binary, 127, 255, cv::THRESH_BINARY);

            cv::normalize(mask_binary, mask_binary, 0, 255, cv::NORM_MINMAX, CV_8UC1);

            int image_type = image.type();
            int mask_type = mask_binary.type();

            // Blend the mask together into one single mask
            cv::add(image, mask_binary, image);
        }
    }
}

std::vector<Segmentation> YOLOv11SegDetector::segment(const cv::Mat &image,
                                                      float confThreshold,
                                                      float iouThreshold)
{

    float *blobPtr = nullptr;
    std::vector<int64_t> inputShape = {1, 3, inputImageShape.height, inputImageShape.width};
    cv::Mat letterboxImg = preprocess(image, blobPtr, inputShape);

    size_t inputSize = utils::vectorProduct(inputShape);
    std::vector<float> inputVals(blobPtr, blobPtr + inputSize);
    delete[] blobPtr;

    Ort::MemoryInfo memInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    Ort::Value inputTensor = Ort::Value::CreateTensor<float>(
        memInfo,
        inputVals.data(),
        inputSize,
        inputShape.data(),
        inputShape.size()
        );

    std::vector<Ort::Value> outputs = session.Run(
        Ort::RunOptions{nullptr},
        inputNames.data(),
        &inputTensor,
        numInputNodes,
        outputNames.data(),
        numOutputNodes);

    cv::Size letterboxSize(static_cast<int>(inputShape[3]), static_cast<int>(inputShape[2]));
    return postprocess(image.size(), letterboxSize, outputs, confThreshold, iouThreshold);
}
