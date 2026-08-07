#include "yolo11segrknn.h"
#include <QFile>
#include <cstring>
#include <format>
#include <stdexcept>
#include "utils.h"

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

    if (shape0.size() < 3 || shape1.size() < 4)
    {
        throw std::runtime_error("Unexpected RKNN output tensor rank.");
    }

    // output0: [1, num_features, num_boxes]
    const int num_features  = static_cast<int>(shape0[1]);
    const int num_boxes     = static_cast<int>(shape0[2]);
    // output1: [1, 32, maskH, maskW]
    const int maskH         = static_cast<int>(shape1[2]);
    const int maskW         = static_cast<int>(shape1[3]);

    if (num_boxes == 0)
    {
        return results;
    }

    const int numClasses        = num_features - 4 - kMaskPrototypeCount;
    constexpr int BOX_OFFSET    = 0;
    constexpr int CLASS_CONF_OFFSET = 4;
    const int MASK_COEFF_OFFSET = numClasses + CLASS_CONF_OFFSET;

    if (numClasses <= 0)
    {
        throw std::runtime_error("Invalid number of classes derived from output0 shape.");
    }

    // Prototype masks
    std::vector<cv::Mat> prototypeMasks;
    prototypeMasks.reserve(kMaskPrototypeCount);
    const size_t maskPlaneSize = static_cast<size_t>(maskH) * static_cast<size_t>(maskW);
    for (int m = 0; m < kMaskPrototypeCount; ++m)
    {
        cv::Mat proto(maskH, maskW, CV_32F);
        std::memcpy(proto.data,
                    output1 + static_cast<size_t>(m) * maskPlaneSize,
                    maskPlaneSize * sizeof(float));
        prototypeMasks.emplace_back(std::move(proto));
    }

    // Parse detections
    std::vector<BoundingBox>              boxes;
    std::vector<float>                    confidences;
    std::vector<int>                      classIds;
    std::vector<std::vector<float>>       maskCoefficientsList;

    for (int i = 0; i < num_boxes; ++i)
    {
        float xc = output0[BOX_OFFSET * num_boxes + i];
        float yc = output0[(BOX_OFFSET + 1) * num_boxes + i];
        float w  = output0[(BOX_OFFSET + 2) * num_boxes + i];
        float h  = output0[(BOX_OFFSET + 3) * num_boxes + i];

        BoundingBox box{
            static_cast<int>(std::round(xc - w / 2.0f)),
            static_cast<int>(std::round(yc - h / 2.0f)),
            static_cast<int>(std::round(w)),
            static_cast<int>(std::round(h))};

        float maxConf = 0.0f;
        int classId   = -1;
        for (int c = 0; c < numClasses; ++c)
        {
            float conf = output0[(CLASS_CONF_OFFSET + c) * num_boxes + i];
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

        boxes.push_back(box);
        confidences.push_back(maxConf);
        classIds.push_back(classId);

        std::vector<float> maskCoeffs(kMaskPrototypeCount);
        for (int m = 0; m < kMaskPrototypeCount; ++m)
        {
            maskCoeffs[m] = output0[(MASK_COEFF_OFFSET + m) * num_boxes + i];
        }
        maskCoefficientsList.emplace_back(std::move(maskCoeffs));
    }

    if (boxes.empty())
    {
        return results;
    }

    std::vector<int> nmsIndices;
    utils::NMSBoxes(boxes, confidences, confThreshold, iouThreshold, nmsIndices);

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

    for (const int idx : nmsIndices)
    {
        Segmentation seg;
        seg.box     = boxes[idx];
        seg.conf    = confidences[idx];
        seg.classId = classIds[idx];

        seg.box = utils::scaleCoords(letterboxSize, seg.box, origSize, true);

        const auto &maskCoeffs = maskCoefficientsList[idx];

        cv::Mat finalMask = cv::Mat::zeros(maskH, maskW, CV_32F);
        for (int m = 0; m < kMaskPrototypeCount; ++m)
        {
            finalMask += maskCoeffs[m] * prototypeMasks[m];
        }

        finalMask = utils::sigmoid(finalMask);

        int x1 = static_cast<int>(std::round((padW - 0.1f) * maskScaleX));
        int y1 = static_cast<int>(std::round((padH - 0.1f) * maskScaleY));
        int x2 = static_cast<int>(std::round((letterboxSize.width  - padW + 0.1f) * maskScaleX));
        int y2 = static_cast<int>(std::round((letterboxSize.height - padH + 0.1f) * maskScaleY));

        x1 = std::max(0, std::min(x1, maskW - 1));
        y1 = std::max(0, std::min(y1, maskH - 1));
        x2 = std::max(x1, std::min(x2, maskW));
        y2 = std::max(y1, std::min(y2, maskH));

        if (x2 <= x1 || y2 <= y1)
        {
            continue;
        }

        cv::Rect cropRect(x1, y1, x2 - x1, y2 - y1);
        cv::Mat  croppedMask = finalMask(cropRect).clone();

        cv::Mat resizedMask;
        cv::resize(croppedMask, resizedMask, origSize, 0, 0, cv::INTER_LINEAR);

        cv::Mat binaryMask;
        cv::threshold(resizedMask, binaryMask, 0.5, 255.0, cv::THRESH_BINARY);
        binaryMask.convertTo(binaryMask, CV_8U);

        cv::Mat finalBinaryMask = cv::Mat::zeros(origSize, CV_8U);
        cv::Rect roi(seg.box.x, seg.box.y, seg.box.width, seg.box.height);
        roi &= cv::Rect(0, 0, binaryMask.cols, binaryMask.rows);
        if (roi.area() > 0)
        {
            binaryMask(roi).copyTo(finalBinaryMask(roi));
        }

        seg.mask = finalBinaryMask;
        results.push_back(seg);
    }

    return results;
}

std::vector<Segmentation> YOLOv11SegDetectorRknn::segment(const cv::Mat &image,
                                                          float confThreshold,
                                                          float iouThreshold)
{
    cv::Mat letterboxImg = preprocess(image);

    rknn_tensor_attr inputAttr{};
    inputAttr.index = 0;
    int ret = rknn_query(ctx, RKNN_QUERY_INPUT_ATTR, &inputAttr, sizeof(inputAttr));
    if (ret < 0)
    {
        throw std::runtime_error("rknn_query INPUT_ATTR failed: " + std::to_string(ret));
    }
    if (inputAttr.fmt != RKNN_TENSOR_NHWC)
    {
        throw std::runtime_error("Unsupported RKNN input format (only NHWC is supported).");
    }
    if (inputAttr.type != RKNN_TENSOR_UINT8)
    {
        throw std::runtime_error("Unsupported RKNN input type (only UINT8 is supported).");
    }

    rknn_input inputs[1]{};
    inputs[0].index        = 0;
    inputs[0].type         = inputAttr.type;
    inputs[0].fmt          = inputAttr.fmt;
    inputs[0].buf          = letterboxImg.data;
    inputs[0].size         = static_cast<uint32_t>(letterboxImg.total() * letterboxImg.elemSize());
    inputs[0].pass_through = 0;

    ret = rknn_inputs_set(ctx, 1, inputs);
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
        outputs[i].want_float = 1;
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

    std::vector<int64_t> shape0, shape1;
    for (uint32_t d = 0; d < outputAttrs[0].n_dims; ++d)
    {
        shape0.push_back(static_cast<int64_t>(outputAttrs[0].dims[d]));
    }
    for (uint32_t d = 0; d < outputAttrs[1].n_dims; ++d)
    {
        shape1.push_back(static_cast<int64_t>(outputAttrs[1].dims[d]));
    }

    if (outputs[0].buf == nullptr || outputs[1].buf == nullptr)
    {
        throw std::runtime_error("RKNN returned null output buffers.");
    }

    return postprocess(image.size(), letterboxImg.size(),
                       reinterpret_cast<const float *>(outputs[0].buf), shape0,
                       reinterpret_cast<const float *>(outputs[1].buf), shape1,
                       confThreshold, iouThreshold);
}
