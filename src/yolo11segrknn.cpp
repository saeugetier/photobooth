#include "yolo11segrknn.h"
#include <QFile>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <format>
#include <limits>
#include <optional>
#include <stdexcept>
#include "utils.h"

namespace {

constexpr int kBoxOffset = 0;
constexpr int kClassConfOffset = 4;

bool isSegDebugEnabled()
{
    static const bool enabled = [] {
        const char *env = std::getenv("PHOTOBOOTH_RKNN_SEG_DEBUG");
        return env != nullptr && std::strcmp(env, "0") != 0;
    }();
    return enabled;
}

float sigmoidScalar(float x)
{
    return 1.0f / (1.0f + std::exp(-x));
}

const char *tensorTypeToString(rknn_tensor_type type)
{
    switch (type)
    {
        case RKNN_TENSOR_UINT8:
            return "UINT8";
        case RKNN_TENSOR_INT8:
            return "INT8";
        case RKNN_TENSOR_FLOAT16:
            return "FLOAT16";
        default:
            return "OTHER";
    }
}

const char *tensorFormatToString(rknn_tensor_format fmt)
{
    switch (fmt)
    {
        case RKNN_TENSOR_NCHW:
            return "NCHW";
        case RKNN_TENSOR_NHWC:
            return "NHWC";
        default:
            return "OTHER";
    }
}

std::string shapeToString(const std::vector<int64_t> &shape)
{
    std::string text = "[";
    for (size_t i = 0; i < shape.size(); ++i)
    {
        text += std::to_string(shape[i]);
        if (i + 1 < shape.size())
        {
            text += ", ";
        }
    }
    text += "]";
    return text;
}

std::string shapeFromAttrToString(const rknn_tensor_attr &attr)
{
    std::vector<int64_t> shape;
    shape.reserve(attr.n_dims);
    for (uint32_t d = 0; d < attr.n_dims; ++d)
    {
        shape.push_back(static_cast<int64_t>(attr.dims[d]));
    }
    return shapeToString(shape);
}

void logTensorAttr(const char *name, const rknn_tensor_attr &attr)
{
    if (!isSegDebugEnabled())
    {
        return;
    }

    qDebug().noquote()
        << "[RKNN-SEG-DEBUG]"
        << name
        << "idx=" << attr.index
        << "dims=" << QString::fromStdString(shapeFromAttrToString(attr))
        << "fmt=" << tensorFormatToString(attr.fmt)
        << "type=" << tensorTypeToString(attr.type);
}

void logFloatBufferStats(const char *name, const float *data, size_t count)
{
    if (!isSegDebugEnabled())
    {
        return;
    }
    if (data == nullptr)
    {
        qWarning() << "[RKNN-SEG-DEBUG]" << name << "buffer is null.";
        return;
    }
    if (count == 0)
    {
        qWarning() << "[RKNN-SEG-DEBUG]" << name << "buffer has zero elements.";
        return;
    }

    float minVal = std::numeric_limits<float>::max();
    float maxVal = std::numeric_limits<float>::lowest();
    double sum = 0.0;
    size_t finiteCount = 0;
    size_t nonFiniteCount = 0;
    size_t nearZeroCount = 0;
    constexpr float kNearZeroEps = 1e-6f;

    for (size_t i = 0; i < count; ++i)
    {
        const float v = data[i];
        if (!std::isfinite(v))
        {
            ++nonFiniteCount;
            continue;
        }
        minVal = std::min(minVal, v);
        maxVal = std::max(maxVal, v);
        sum += static_cast<double>(v);
        ++finiteCount;
        if (std::fabs(v) <= kNearZeroEps)
        {
            ++nearZeroCount;
        }
    }

    if (finiteCount == 0)
    {
        minVal = 0.0f;
        maxVal = 0.0f;
    }

    const double mean = sum / static_cast<double>(finiteCount == 0 ? 1 : finiteCount);
    qDebug().noquote()
        << "[RKNN-SEG-DEBUG]"
        << name
        << "count=" << static_cast<qulonglong>(count)
        << "min=" << minVal
        << "max=" << maxVal
        << "mean=" << mean
        << "nonFinite=" << static_cast<qulonglong>(nonFiniteCount)
        << "nearZero=" << static_cast<qulonglong>(nearZeroCount);

    const size_t sampleCount = std::min<size_t>(count, 8);
    std::string samples;
    for (size_t i = 0; i < sampleCount; ++i)
    {
        if (i > 0)
        {
            samples += ", ";
        }
        samples += std::format("{:g}", data[i]);
    }
    qDebug().noquote() << "[RKNN-SEG-DEBUG]" << name << "first_values=" << QString::fromStdString(samples);
}

struct ParsedDetections
{
    std::vector<BoundingBox> boxes;
    std::vector<float> confidences;
    std::vector<int> classIds;
    std::vector<std::vector<float>> maskCoefficients;
};

void validatePostprocessShapes(const std::vector<int64_t> &shape0,
                               const std::vector<int64_t> &shape1,
                               int maskPrototypeCount)
{
    if (shape0.size() < 3 || shape1.size() < 4)
    {
        throw std::runtime_error("Unexpected RKNN output tensor rank.");
    }
    if (shape0[0] != 1)
    {
        throw std::runtime_error("Unexpected output0 shape. Expected batch size 1.");
    }
    if (shape1[0] != 1 || shape1[1] != maskPrototypeCount)
    {
        throw std::runtime_error("Unexpected output1 shape. Expected [1, 32, maskH, maskW].");
    }
}

std::vector<cv::Mat> buildPrototypeMasks(const float *output1,
                                         int maskH,
                                         int maskW,
                                         int maskPrototypeCount)
{
    std::vector<cv::Mat> prototypeMasks;
    prototypeMasks.reserve(maskPrototypeCount);

    const size_t maskPlaneSize = static_cast<size_t>(maskH) * static_cast<size_t>(maskW);
    for (int m = 0; m < maskPrototypeCount; ++m)
    {
        cv::Mat proto(maskH, maskW, CV_32F);
        std::memcpy(proto.data,
                    output1 + static_cast<size_t>(m) * maskPlaneSize,
                    maskPlaneSize * sizeof(float));
        prototypeMasks.emplace_back(std::move(proto));
    }

    return prototypeMasks;
}

ParsedDetections parseDetections(const float *output0,
                                 int numBoxes,
                                 int numClasses,
                                 int maskCoeffOffset,
                                 int maskPrototypeCount,
                                 bool boxesMajorLayout,
                                 float confThreshold)
{
    ParsedDetections parsed;

    for (int i = 0; i < numBoxes; ++i)
    {
        const auto readValue = [&](int featureIndex) -> float {
            if (boxesMajorLayout)
            {
                return output0[static_cast<size_t>(i) * static_cast<size_t>(maskCoeffOffset + maskPrototypeCount) + featureIndex];
            }
            return output0[static_cast<size_t>(featureIndex) * static_cast<size_t>(numBoxes) + i];
        };

        const float xc = readValue(kBoxOffset);
        const float yc = readValue(kBoxOffset + 1);
        const float w  = readValue(kBoxOffset + 2);
        const float h  = readValue(kBoxOffset + 3);

        const BoundingBox box{
            static_cast<int>(std::round(xc - w / 2.0f)),
            static_cast<int>(std::round(yc - h / 2.0f)),
            static_cast<int>(std::round(w)),
            static_cast<int>(std::round(h))};

        float maxConf = 0.0f;
        int classId   = -1;
        for (int c = 0; c < numClasses; ++c)
        {
            const float conf = readValue(kClassConfOffset + c);
            if (conf > maxConf)
            {
                maxConf = conf;
                classId = c;
            }
        }

        if (maxConf < confThreshold)
        {
            continue;
        }

        parsed.boxes.push_back(box);
        parsed.confidences.push_back(maxConf);
        parsed.classIds.push_back(classId);

        std::vector<float> maskCoeffs(maskPrototypeCount);
        for (int m = 0; m < maskPrototypeCount; ++m)
        {
            maskCoeffs[m] = readValue(maskCoeffOffset + m);
        }
        parsed.maskCoefficients.emplace_back(std::move(maskCoeffs));
    }

    return parsed;
}

cv::Rect computeMaskCropRect(const cv::Size &letterboxSize,
                             int maskW,
                             int maskH,
                             float padW,
                             float padH,
                             float maskScaleX,
                             float maskScaleY)
{
    int x1 = static_cast<int>(std::round((padW - 0.1f) * maskScaleX));
    int y1 = static_cast<int>(std::round((padH - 0.1f) * maskScaleY));
    int x2 = static_cast<int>(std::round((letterboxSize.width  - padW + 0.1f) * maskScaleX));
    int y2 = static_cast<int>(std::round((letterboxSize.height - padH + 0.1f) * maskScaleY));

    x1 = std::max(0, std::min(x1, maskW - 1));
    y1 = std::max(0, std::min(y1, maskH - 1));
    x2 = std::max(x1, std::min(x2, maskW));
    y2 = std::max(y1, std::min(y2, maskH));

    return cv::Rect(x1, y1, x2 - x1, y2 - y1);
}

std::optional<Segmentation> buildSegmentation(const cv::Size &origSize,
                                              const cv::Size &letterboxSize,
                                              const std::vector<BoundingBox> &boxes,
                                              const std::vector<float> &confidences,
                                              const std::vector<int> &classIds,
                                              const std::vector<std::vector<float>> &maskCoefficients,
                                              const std::vector<cv::Mat> &prototypeMasks,
                                              int idx,
                                              int maskH,
                                              int maskW,
                                              int maskPrototypeCount,
                                              float padW,
                                              float padH,
                                              float maskScaleX,
                                              float maskScaleY,
                                              int debugIndex)
{
    Segmentation seg;
    seg.box     = boxes[idx];
    seg.conf    = confidences[idx];
    seg.classId = classIds[idx];
    seg.box = utils::scaleCoords(letterboxSize, seg.box, origSize, true);

    const auto &coeffs = maskCoefficients[idx];
    cv::Mat finalMask = cv::Mat::zeros(maskH, maskW, CV_32F);
    for (int m = 0; m < maskPrototypeCount; ++m)
    {
        finalMask += coeffs[m] * prototypeMasks[m];
    }
    finalMask = utils::sigmoid(finalMask);

    const cv::Rect cropRect = computeMaskCropRect(letterboxSize, maskW, maskH, padW, padH, maskScaleX, maskScaleY);
    if (cropRect.width <= 0 || cropRect.height <= 0)
    {
        return std::nullopt;
    }

    cv::Mat resizedMask;
    cv::resize(finalMask(cropRect).clone(), resizedMask, origSize, 0, 0, cv::INTER_LINEAR);

    cv::Mat binaryMask;
    cv::threshold(resizedMask, binaryMask, 0.5, 255.0, cv::THRESH_BINARY);
    binaryMask.convertTo(binaryMask, CV_8U);
    const int binaryNonZero = cv::countNonZero(binaryMask);

    cv::Mat finalBinaryMask = cv::Mat::zeros(origSize, CV_8U);
    cv::Rect roi(seg.box.x, seg.box.y, seg.box.width, seg.box.height);
    roi &= cv::Rect(0, 0, binaryMask.cols, binaryMask.rows);
    if (roi.area() > 0)
    {
        binaryMask(roi).copyTo(finalBinaryMask(roi));
    }

    if (isSegDebugEnabled() && debugIndex < 5)
    {
        const int finalNonZero = cv::countNonZero(finalBinaryMask);
        qDebug() << "[RKNN-SEG-DEBUG] mask idx=" << debugIndex
                 << "class=" << seg.classId
                 << "conf=" << seg.conf
                 << "roi=" << roi.x << roi.y << roi.width << roi.height
                 << "cropRect=" << cropRect.x << cropRect.y << cropRect.width << cropRect.height
                 << "binaryNonZero=" << binaryNonZero
                 << "finalNonZero=" << finalNonZero;
    }

    seg.mask = finalBinaryMask;
    return seg;
}

rknn_tensor_attr queryInputAttr(rknn_context ctx)
{
    rknn_tensor_attr inputAttr{};
    inputAttr.index = 0;
    const int ret = rknn_query(ctx, RKNN_QUERY_INPUT_ATTR, &inputAttr, sizeof(inputAttr));
    if (ret < 0)
    {
        throw std::runtime_error(std::format("rknn_query INPUT_ATTR failed: {}", ret));
    }
    return inputAttr;
}

void validateInputAttr(const rknn_tensor_attr &inputAttr)
{
    if (inputAttr.fmt != RKNN_TENSOR_NHWC)
    {
        throw std::runtime_error("Unsupported RKNN input format (only NHWC is supported).");
    }
    if (inputAttr.type != RKNN_TENSOR_FLOAT16 && inputAttr.type != RKNN_TENSOR_UINT8 && inputAttr.type != RKNN_TENSOR_INT8)
    {
        throw std::runtime_error("Unsupported RKNN input type (supported: UINT8, INT8, FLOAT16).");
    }
}

cv::Mat prepareInputTensor(const cv::Mat &letterboxImage,
                           const rknn_tensor_attr &inputAttr)
{
    if (inputAttr.type == RKNN_TENSOR_UINT8 || inputAttr.type == RKNN_TENSOR_INT8)
    {
        return letterboxImage;
    }

    cv::Mat floatInput;
    letterboxImage.convertTo(floatInput, CV_32FC3);

    cv::Mat fp16Input;
    floatInput.convertTo(fp16Input, CV_16FC3);
    return fp16Input;
}

std::vector<int64_t> shapeFromAttr(const rknn_tensor_attr &attr)
{
    std::vector<int64_t> shape;
    shape.reserve(attr.n_dims);
    for (uint32_t d = 0; d < attr.n_dims; ++d)
    {
        shape.push_back(static_cast<int64_t>(attr.dims[d]));
    }
    return shape;
}

bool shouldRequestFloatOutput(const rknn_tensor_attr &outputAttr)
{
    (void)outputAttr;
    // Keep postprocess in float space regardless of model quantization.
    // RKNN runtime dequantizes when want_float is set.
    return true;
}

} // namespace

YOLOv11SegDetectorRknn::YOLOv11SegDetectorRknn(const std::string &modelPath,
                                               const std::string &labelsPath)
    : Yolo11Segementation(labelsPath)
{
    const std::string fullPath = getModelRessourcePath(modelPath);

    QFile modelFile(QString::fromStdString(fullPath));
    if (!modelFile.open(QIODevice::ReadOnly))
    {
        throw std::runtime_error(std::format("Failed to open RKNN model: {}", fullPath));
    }
    QByteArray modelData = modelFile.readAll();

    int ret = rknn_init(&ctx, modelData.data(), static_cast<uint32_t>(modelData.size()), 0, nullptr);
    if (ret < 0)
    {
        throw std::runtime_error(std::format("rknn_init failed, error: {}", ret));
    }

    try
    {
        rknn_input_output_num io_num{};
        ret = rknn_query(ctx, RKNN_QUERY_IN_OUT_NUM, &io_num, sizeof(io_num));
        if (ret < 0)
        {
            throw std::runtime_error(std::format("rknn_query IN_OUT_NUM failed: {}", ret));
        }
        numInputNodes  = io_num.n_input;
        numOutputNodes = io_num.n_output;

        if (numInputNodes != 1)
        {
            throw std::runtime_error("Expected exactly 1 input node.");
        }
        if (numOutputNodes != 2)
        {
            throw std::runtime_error("Expected exactly 2 output nodes: output0 and output1.");
        }

        // Determine input spatial dimensions from the queried tensor attribute
        rknn_tensor_attr inputAttr{};
        inputAttr.index = 0;
        ret = rknn_query(ctx, RKNN_QUERY_INPUT_ATTR, &inputAttr, sizeof(inputAttr));
        if (ret < 0)
        {
            throw std::runtime_error(std::format("rknn_query INPUT_ATTR failed: {}", ret));
        }

        if (inputAttr.n_dims == 4)
        {
            if (inputAttr.fmt == RKNN_TENSOR_NHWC)
            {
                inputImageShape = cv::Size(inputAttr.dims[2], inputAttr.dims[1]); // W=dim[2], H=dim[1]
            }
            else // NCHW
            {
                inputImageShape = cv::Size(inputAttr.dims[3], inputAttr.dims[2]); // W=dim[3], H=dim[2]
            }
        }
        else
        {
            throw std::runtime_error("Expected 4 dimensions for RKNN input tensor.");
        }

        logTensorAttr("input", inputAttr);

        outputAttrs.resize(numOutputNodes);
        for (uint32_t i = 0; i < numOutputNodes; ++i)
        {
            outputAttrs[i]       = {};
            outputAttrs[i].index = i;
            ret = rknn_query(ctx, RKNN_QUERY_OUTPUT_ATTR, &outputAttrs[i], sizeof(rknn_tensor_attr));
            if (ret < 0)
            {
                throw std::runtime_error(std::format("rknn_query OUTPUT_ATTR failed for output {}: {}", i, ret));
            }
            logTensorAttr((std::string("output") + std::to_string(i)).c_str(), outputAttrs[i]);
        }

        qDebug() << "[INFO] YOLOv11Seg RKNN loaded: " << modelPath;
        qDebug() << "      Input shape: " << inputImageShape.height << "x" << inputImageShape.width;
        qDebug() << "      #Outputs   : " << numOutputNodes;
        qDebug() << "      #Classes   : " << classNames.size();
    }
    catch (...)
    {
        if (ctx)
        {
            rknn_destroy(ctx);
            ctx = 0;
        }
        throw;
    }
}

YOLOv11SegDetectorRknn::~YOLOv11SegDetectorRknn()
{
    if (ctx)
    {
        rknn_destroy(ctx);
        ctx = 0;
    }
}

cv::Mat YOLOv11SegDetectorRknn::preprocess(const cv::Mat &image)
{
    cv::Mat letterboxImage;
    utils::letterBox(image, letterboxImage, inputImageShape,
                     cv::Scalar(114, 114, 114), /*auto_=*/false,
                     /*scaleFill=*/false, /*scaleUp=*/true, /*stride=*/32);
    // RKNN models are typically converted from RGB-trained ONNX models
    cv::cvtColor(letterboxImage, letterboxImage, cv::COLOR_BGR2RGB);
    return letterboxImage;
}

std::vector<Segmentation> YOLOv11SegDetectorRknn::postprocess(
    const cv::Size     &origSize,
    const cv::Size     &letterboxSize,
    const float        *output0,
    const std::vector<int64_t> &shape0,
    const float        *output1,
    const std::vector<int64_t> &shape1,
    float               confThreshold,
    float               iouThreshold)
{
    std::vector<Segmentation> results;

    validatePostprocessShapes(shape0, shape1, kMaskPrototypeCount);

    const int expectedFeatureCount = static_cast<int>(classNames.size()) + 4 + kMaskPrototypeCount;

    // output0 is typically [1, num_features, num_boxes], but some RKNN exports
    // preserve the detection tensor as [1, num_boxes, num_features].
    const bool featuresMajorLayout = (shape0[1] == expectedFeatureCount);
    const bool boxesMajorLayout = (shape0[2] == expectedFeatureCount);
    if (!featuresMajorLayout && !boxesMajorLayout)
    {
        throw std::runtime_error(std::format("Unexpected RKNN output0 shape. Expected one dimension to equal {}.", expectedFeatureCount));
    }

    const int num_features = featuresMajorLayout ? static_cast<int>(shape0[1]) : static_cast<int>(shape0[2]);
    const int num_boxes    = featuresMajorLayout ? static_cast<int>(shape0[2]) : static_cast<int>(shape0[1]);
    // output1: [1, 32, maskH, maskW]
    const int maskH         = static_cast<int>(shape1[2]);
    const int maskW         = static_cast<int>(shape1[3]);

    if (num_boxes == 0)
    {
        return results;
    }

    const int numClasses        = num_features - 4 - kMaskPrototypeCount;
    const int maskCoeffOffset = numClasses + kClassConfOffset;

    if (isSegDebugEnabled())
    {
        qDebug().noquote()
            << "[RKNN-SEG-DEBUG] postprocess shape0=" << QString::fromStdString(shapeToString(shape0))
            << "shape1=" << QString::fromStdString(shapeToString(shape1))
            << "layout=" << (featuresMajorLayout ? "features-major" : "boxes-major")
            << "num_boxes=" << num_boxes
            << "num_features=" << num_features
            << "num_classes=" << numClasses
            << "maskCoeffOffset=" << maskCoeffOffset
            << "classSigmoid=" << "off";
    }

    if (numClasses <= 0)
    {
        throw std::runtime_error("Invalid number of classes derived from output0 shape.");
    }

    const auto prototypeMasks = buildPrototypeMasks(output1, maskH, maskW, kMaskPrototypeCount);
    const auto parsed = parseDetections(output0,
                                        num_boxes,
                                        numClasses,
                                        maskCoeffOffset,
                                        kMaskPrototypeCount,
                                        boxesMajorLayout,
                                        confThreshold);

    if (isSegDebugEnabled())
    {
        float maxDetConf = 0.0f;
        float maxDetConfSigmoid = 0.0f;
        for (int i = 0; i < num_boxes; ++i)
        {
            float detMaxConf = 0.0f;
            float detMaxConfSigmoid = 0.0f;
            for (int c = 0; c < numClasses; ++c)
            {
                const int featureIndex = kClassConfOffset + c;
                const float conf = boxesMajorLayout
                    ? output0[static_cast<size_t>(i) * static_cast<size_t>(maskCoeffOffset + kMaskPrototypeCount) + featureIndex]
                    : output0[static_cast<size_t>(featureIndex) * static_cast<size_t>(num_boxes) + i];
                detMaxConf = std::max(detMaxConf, conf);
                detMaxConfSigmoid = std::max(detMaxConfSigmoid, sigmoidScalar(conf));
            }
            maxDetConf = std::max(maxDetConf, detMaxConf);
            maxDetConfSigmoid = std::max(maxDetConfSigmoid, detMaxConfSigmoid);
        }
        qDebug() << "[RKNN-SEG-DEBUG] detections above threshold=" << parsed.boxes.size()
                 << "of" << num_boxes
                 << "max raw class confidence=" << maxDetConf
                 << "max sigmoid class confidence=" << maxDetConfSigmoid
                 << "active score mode=raw"
                 << "threshold=" << confThreshold;
    }

    if (parsed.boxes.empty())
    {
        return results;
    }

    std::vector<int> nmsIndices;
    utils::NMSBoxes(parsed.boxes, parsed.confidences, confThreshold, iouThreshold, nmsIndices);

    if (isSegDebugEnabled())
    {
        qDebug() << "[RKNN-SEG-DEBUG] NMS kept" << nmsIndices.size()
                 << "of" << parsed.boxes.size()
                 << "candidates with IoU threshold" << iouThreshold;
    }

    if (nmsIndices.empty())
    {
        return results;
    }

    results.reserve(nmsIndices.size());

    const float gain   = std::min(static_cast<float>(letterboxSize.height) / origSize.height,
                                  static_cast<float>(letterboxSize.width)  / origSize.width);
    const int scaledW  = static_cast<int>(origSize.width  * gain);
    const int scaledH  = static_cast<int>(origSize.height * gain);
    const float padW   = (letterboxSize.width  - scaledW) / 2.0f;
    const float padH   = (letterboxSize.height - scaledH) / 2.0f;

    const float maskScaleX = static_cast<float>(maskW) / letterboxSize.width;
    const float maskScaleY = static_cast<float>(maskH) / letterboxSize.height;

    int debugMaskCounter = 0;
    for (const int idx : nmsIndices)
    {
        auto seg = buildSegmentation(origSize,
                                     letterboxSize,
                                     parsed.boxes,
                                     parsed.confidences,
                                     parsed.classIds,
                                     parsed.maskCoefficients,
                                     prototypeMasks,
                                     idx,
                                     maskH,
                                     maskW,
                                     kMaskPrototypeCount,
                                     padW,
                                     padH,
                                     maskScaleX,
                                     maskScaleY,
                                     debugMaskCounter);
        ++debugMaskCounter;
        if (!seg.has_value())
        {
            continue;
        }
        results.push_back(*seg);
    }

    return results;
}

std::vector<Segmentation> YOLOv11SegDetectorRknn::segment(const cv::Mat &image,
                                                          float confThreshold,
                                                          float iouThreshold)
{
    const cv::Mat letterboxImg = preprocess(image);

    const rknn_tensor_attr inputAttr = queryInputAttr(ctx);
    validateInputAttr(inputAttr);
    const cv::Mat inputTensor = prepareInputTensor(letterboxImg, inputAttr);

    if (isSegDebugEnabled())
    {
        logTensorAttr("input-runtime", inputAttr);
        qDebug() << "[RKNN-SEG-DEBUG] image=" << image.cols << "x" << image.rows
                 << "letterbox=" << letterboxImg.cols << "x" << letterboxImg.rows
                 << "inputTensorType=" << inputTensor.type()
                 << "floatInputMode=raw_0_255"
                 << "inputTensorBytes=" << static_cast<qulonglong>(inputTensor.total() * inputTensor.elemSize());
    }

    rknn_input inputs[1]{};
    inputs[0].index        = 0;
    inputs[0].type         = inputAttr.type;
    inputs[0].fmt          = inputAttr.fmt;
    inputs[0].buf          = inputTensor.data;
    inputs[0].size         = static_cast<uint32_t>(inputTensor.total() * inputTensor.elemSize());
    inputs[0].pass_through = 0;

    int ret = rknn_inputs_set(ctx, 1, inputs);
    if (ret < 0)
    {
        throw std::runtime_error(std::format("rknn_inputs_set failed: {}", ret));
    }

    ret = rknn_run(ctx, nullptr);
    if (ret < 0)
    {
        throw std::runtime_error(std::format("rknn_run failed: {}", ret));
    }

    std::vector<rknn_output> outputs(numOutputNodes);
    for (uint32_t i = 0; i < numOutputNodes; ++i)
    {
        outputs[i]            = {};
        outputs[i].index      = i;
        outputs[i].want_float = shouldRequestFloatOutput(outputAttrs[i]) ? 1 : 0;
        outputs[i].is_prealloc = 0;
    }

    ret = rknn_outputs_get(ctx, numOutputNodes, outputs.data(), nullptr);
    if (ret < 0)
    {
        throw std::runtime_error(std::format("rknn_outputs_get failed: {}", ret));
    }

    struct OutputReleaseGuard
    {
        rknn_context context;
        uint32_t outputCount;
        rknn_output *outputData;

        ~OutputReleaseGuard()
        {
            rknn_outputs_release(context, outputCount, outputData);
        }
    } guard{ctx, numOutputNodes, outputs.data()};

    const std::vector<int64_t> shape0 = shapeFromAttr(outputAttrs[0]);
    const std::vector<int64_t> shape1 = shapeFromAttr(outputAttrs[1]);

    if (outputs[0].buf == nullptr || outputs[1].buf == nullptr)
    {
        throw std::runtime_error("RKNN returned null output buffers.");
    }

    if (isSegDebugEnabled())
    {
        const size_t output0Count = utils::vectorProduct(shape0);
        const size_t output1Count = utils::vectorProduct(shape1);
        qDebug().noquote()
            << "[RKNN-SEG-DEBUG] output0 shape=" << QString::fromStdString(shapeToString(shape0))
            << "output1 shape=" << QString::fromStdString(shapeToString(shape1));
        logFloatBufferStats("output0", reinterpret_cast<const float *>(outputs[0].buf), output0Count);
        logFloatBufferStats("output1", reinterpret_cast<const float *>(outputs[1].buf), output1Count);
    }

    return postprocess(image.size(), letterboxImg.size(),
                       reinterpret_cast<const float *>(outputs[0].buf), shape0,
                       reinterpret_cast<const float *>(outputs[1].buf), shape1,
                       confThreshold, iouThreshold);
}
