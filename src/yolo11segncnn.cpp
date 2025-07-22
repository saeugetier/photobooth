#include "yolo11segncnn.h"
#include <QDir>
#include "utils.h"
#include <QStandardPaths>

// Original Author: Abdalrahman M. Amer, www.linkedin.com/in/abdalrahman-m-amer
// Date: 25.01.2025
// Modified for use in photbooth

YOLOv11SegDetectorNcnn::YOLOv11SegDetectorNcnn(const std::string &modelPath,
                                               const std::string &labelsPath,
                                               bool useGPU) : Yolo11Segementation(labelsPath)
{
    QString ressourcePathGeneric = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "models", QStandardPaths::LocateDirectory);
    QString ressourcePathApp = QStandardPaths::locate(QStandardPaths::AppDataLocation, "models", QStandardPaths::LocateDirectory);
    if (ressourcePathApp.isEmpty() && ressourcePathGeneric.isEmpty())
    {
        throw std::runtime_error("Failed to locate the models directory.");
    }
    QString ressourcePath = "";
    if (!ressourcePathApp.isEmpty())
    {
        ressourcePath = QDir::cleanPath(ressourcePathApp);
    }
    else if (!ressourcePathGeneric.isEmpty())
    {
        ressourcePath = QDir::cleanPath(ressourcePathGeneric);
    }

    qDebug() << "[INFO] Using model path: " << ressourcePath;
    std::string params_path = ressourcePath.toStdString() + "/" + modelPath + "/model.ncnn.param";
    std::string model_path = ressourcePath.toStdString() + "/" + modelPath + "/model.ncnn.bin";

    if (0 != net.load_param(params_path.c_str()))
    {
        throw std::runtime_error("Failed to load model parameters from: " + params_path);
    }

    if (0 != net.load_model(model_path.c_str()))
    {
        throw std::runtime_error("Failed to load model binary from: " + model_path);
    }

    // Set options
    net.opt.use_vulkan_compute = useGPU;

    numInputNodes = net.input_names().size();
    numOutputNodes = net.output_names().size();

    isDynamicInputShape = false;          // Assume static input shape by default. NCNN models typically have fixed input shapes.
    inputImageShape = cv::Size(640, 640); // Default shape. This is fixed for YOLOv11SegNCNN

    // Input
    if (numInputNodes != 1)
    {
        throw std::runtime_error("Expected exactly 1 input node.");
    }

    inputNames = net.input_names();

    // Outputs
    if (numOutputNodes != 2)
    {
        throw std::runtime_error("Expected exactly 2 output nodes: output0 and output1.");
    }

    outputNames = net.output_names();

    classNames = utils::getClassNames(labelsPath);
    classColors = utils::generateColors(classNames);

    qDebug() << "[INFO] YOLOv11Seg loaded: " << modelPath;
    qDebug() << "      Input shape: " << inputImageShape.height << "x" << inputImageShape.width
             << (isDynamicInputShape ? " (dynamic)" : "");
    qDebug() << "      #Outputs   : " << numOutputNodes;
    qDebug() << "      #Classes   : " << classNames.size();
}

cv::Mat YOLOv11SegDetectorNcnn::preprocess(const cv::Mat &image,
                                           float *&blobPtr,
                                           std::vector<int64_t> &inputTensorShape)
{
    cv::Mat letterboxImage;
    utils::letterBox(image, letterboxImage, inputImageShape,
                     cv::Scalar(114, 114, 114), /*auto_=*/false,
                     /*scaleFill=*/false, /*scaleUp=*/true, /*stride=*/32);

    // No dynamic shape in NCNN, so inputTensorShape is not used

    size_t size = static_cast<size_t>(letterboxImage.rows) * static_cast<size_t>(letterboxImage.cols) * 3;
    blobPtr = new float[size];

    return letterboxImage;
}

std::vector<Segmentation> YOLOv11SegDetectorNcnn::postprocess(
    const cv::Size &origSize,
    const cv::Size &letterboxSize,
    const ncnn::Mat &outputs_boxes,
    const ncnn::Mat &outputs_masks,
    float confThreshold,
    float iouThreshold)
{
    std::vector<Segmentation> results;

    // output0: [num_features, num_boxes]
    // output1: [32, maskH, maskW]
    int num_features = outputs_boxes.h;
    int num_boxes = outputs_boxes.w;
    int maskC = outputs_masks.c; // Should be 32
    if (maskC != 32)
    {
        throw std::runtime_error("Expected 32 prototype masks in output1.");
    }
    int maskH = outputs_masks.h;
    int maskW = outputs_masks.w;

    if (num_boxes == 0)
    {
        return results; // Early exit if no boxes
    }

    const int numClasses = num_features - 4 - 32;
    constexpr int BOX_OFFSET = 0;
    constexpr int CLASS_CONF_OFFSET = 4;
    const int MASK_COEFF_OFFSET = numClasses + CLASS_CONF_OFFSET;

    // 1. Process prototype masks
    std::vector<cv::Mat> prototypeMasks;
    prototypeMasks.reserve(32);
    for (int m = 0; m < 32; ++m)
    {
        // Each mask is maskH x maskW
        cv::Mat proto(maskH, maskW, CV_32F, (void *)outputs_masks.channel(m).data);
        prototypeMasks.emplace_back(proto.clone());
    }

    // 2. Process detections
    std::vector<BoundingBox> boxes;
    std::vector<float> confidences;
    std::vector<int> classIds;
    std::vector<std::vector<float>> maskCoefficientsList;

    for (int i = 0; i < num_boxes; ++i)
    {
        // Extract box coordinates
        float xc = outputs_boxes.row(BOX_OFFSET + 0)[i];
        float yc = outputs_boxes.row(BOX_OFFSET + 1)[i];
        float w = outputs_boxes.row(BOX_OFFSET + 2)[i];
        float h = outputs_boxes.row(BOX_OFFSET + 3)[i];

        BoundingBox box{
            static_cast<int>(std::round(xc - w / 2.0f)),
            static_cast<int>(std::round(yc - h / 2.0f)),
            static_cast<int>(std::round(w)),
            static_cast<int>(std::round(h))};

        // Get class confidence
        float maxConf = 0.0f;
        int classId = -1;
        for (int c = 0; c < numClasses; ++c)
        {
            float conf = outputs_boxes.row(CLASS_CONF_OFFSET + c)[i];
            if (conf > maxConf)
            {
                maxConf = conf;
                classId = c;
            }
        }

        if (maxConf < confThreshold)
            continue;

        boxes.push_back(box);
        confidences.push_back(maxConf);
        classIds.push_back(classId);

        // Mask coefficients
        std::vector<float> maskCoeffs(32);
        for (int m = 0; m < 32; ++m)
        {
            maskCoeffs[m] = outputs_boxes.row(MASK_COEFF_OFFSET + m)[i];
        }
        maskCoefficientsList.emplace_back(std::move(maskCoeffs));
    }

    if (boxes.empty())
        return results;

    // 3. Apply NMS
    std::vector<int> nmsIndices;
    utils::NMSBoxes(boxes, confidences, confThreshold, iouThreshold, nmsIndices);

    if (nmsIndices.empty())
        return results;

    // 4. Prepare final results
    results.reserve(nmsIndices.size());

    const float gain = std::min(static_cast<float>(letterboxSize.height) / origSize.height,
                                static_cast<float>(letterboxSize.width) / origSize.width);
    const int scaledW = static_cast<int>(origSize.width * gain);
    const int scaledH = static_cast<int>(origSize.height * gain);
    const float padW = (letterboxSize.width - scaledW) / 2.0f;
    const float padH = (letterboxSize.height - scaledH) / 2.0f;

    const float maskScaleX = static_cast<float>(maskW) / letterboxSize.width;
    const float maskScaleY = static_cast<float>(maskH) / letterboxSize.height;

    for (const int idx : nmsIndices)
    {
        Segmentation seg;
        seg.box = boxes[idx];
        seg.conf = confidences[idx];
        seg.classId = classIds[idx];

        // 5. Scale box to original image
        seg.box = utils::scaleCoords(letterboxSize, seg.box, origSize, true);

        // 6. Process mask
        const auto &maskCoeffs = maskCoefficientsList[idx];

        // Linear combination of prototype masks
        cv::Mat finalMask = cv::Mat::zeros(maskH, maskW, CV_32F);
        for (int m = 0; m < 32; ++m)
        {
            finalMask += maskCoeffs[m] * prototypeMasks[m];
        }

        // Apply sigmoid activation
        finalMask = utils::sigmoid(finalMask);

        // Crop mask to letterbox area with a slight padding to avoid border issues
        int x1 = static_cast<int>(std::round((padW - 0.1f) * maskScaleX));
        int y1 = static_cast<int>(std::round((padH - 0.1f) * maskScaleY));
        int x2 = static_cast<int>(std::round((letterboxSize.width - padW + 0.1f) * maskScaleX));
        int y2 = static_cast<int>(std::round((letterboxSize.height - padH + 0.1f) * maskScaleY));

        x1 = std::max(0, std::min(x1, maskW - 1));
        y1 = std::max(0, std::min(y1, maskH - 1));
        x2 = std::max(x1, std::min(x2, maskW));
        y2 = std::max(y1, std::min(y2, maskH));

        if (x2 <= x1 || y2 <= y1)
            continue;

        cv::Rect cropRect(x1, y1, x2 - x1, y2 - y1);
        cv::Mat croppedMask = finalMask(cropRect).clone();

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

std::vector<Segmentation> YOLOv11SegDetectorNcnn::segment(const cv::Mat &image,
                                                          float confThreshold,
                                                          float iouThreshold)
{
    static int counter = 0;

    cv::Mat letterboxImage;
    utils::letterBox(image, letterboxImage, inputImageShape,
                     cv::Scalar(114, 114, 114), /*auto_=*/false,
                     /*scaleFill=*/false, /*scaleUp=*/true, /*stride=*/32);

    ncnn::Mat in = ncnn::Mat::from_pixels_resize(letterboxImage.data, ncnn::Mat::PIXEL_BGR2RGB, letterboxImage.cols, letterboxImage.rows, 640, 640);

    const float norm_vals[3] = {1 / 255.f, 1 / 255.f, 1 / 255.f};
    in.substract_mean_normalize(0, norm_vals);

    ncnn::Extractor ex = net.create_extractor();
    ex.input("in0", in); // adjust input name as needed

    ncnn::Mat out_boxes, out_masks;
    ex.extract("out0", out_boxes); // [num, 6] (x, y, w, h, conf, class)
    ex.extract("out1", out_masks); // [num, mask_dim, mask_dim]

    return postprocess(image.size(), inputImageShape, out_boxes, out_masks, confThreshold, iouThreshold);
}
