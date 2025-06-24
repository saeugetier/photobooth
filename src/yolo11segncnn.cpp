#include "yolo11segncnn.h"
#include <QDir>
#include "utils.h"
#include <QStandardPaths>

// Original Author: Abdalrahman M. Amer, www.linkedin.com/in/abdalrahman-m-amer
// Date: 25.01.2025
// Modified for use in photbooth


YOLOv11SegDetectorNcnn::YOLOv11SegDetectorNcnn(const std::string &modelPath,
                                       const std::string &labelsPath,
                                       bool useGPU)
{
    QString ressourcePathApp = QStandardPaths::locate(QStandardPaths::AppDataLocation, "models", QStandardPaths::LocateDirectory);
    if (ressourcePathApp.isEmpty() && ressourcePathApp.isEmpty()) {
        throw std::runtime_error("Failed to locate the models directory.");
    }
    ressourcePathApp = QDir::cleanPath(ressourcePathApp);
    qDebug() << "[INFO] Using model path: " << ressourcePathApp;
    std::string params_path = ressourcePathApp.toStdString() + "/" + modelPath + "/model.ncnn.param";
    std::string model_path = ressourcePathApp.toStdString() + "/" + modelPath + "/model.ncnn.bin";

    if(0 != net.load_param(params_path.c_str()))
    {
        throw std::runtime_error("Failed to load model parameters from: " + params_path);
    }

    if(0 != net.load_model(model_path.c_str()))
    {
        throw std::runtime_error("Failed to load model binary from: " + model_path);
    }

    // Set options
    net.opt.use_vulkan_compute = useGPU;
    net.opt.num_threads = std::min(6, static_cast<int>(std::thread::hardware_concurrency()));
    net.opt.use_packing_layout = true;


    numInputNodes  = net.input_names().size();
    numOutputNodes = net.output_names().size();

    isDynamicInputShape = false; // Assume static input shape by default. NCNN models typically have fixed input shapes.
    inputImageShape = cv::Size(640, 640); // Default shape. This is fixed for YOLOv11SegNCNN

    
    // Input
    if (numInputNodes != 1) {
        throw std::runtime_error("Expected exactly 1 input node.");
    }

    inputNames = net.input_names();

    // Outputs
    if (numOutputNodes != 2) {
        throw std::runtime_error("Expected exactly 2 output nodes: output0 and output1.");
    }

    outputNames = net.output_names();

    classNames  = utils::getClassNames(labelsPath);
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

std::vector<Segmentation> YOLOv11SegDetectorNcnn::postprocess(
    const cv::Size &origSize,
    const cv::Size &letterboxSize,
    const ncnn::Mat &outputs_boxes,
    const ncnn::Mat &outputs_masks,
    float confThreshold,
    float iouThreshold)
{
    std::vector<Segmentation> results;

    // Extract outputs
    const float* output0_ptr = reinterpret_cast<const float*>(outputs_boxes.data); // [1, 116, num_detections]
    const float* output1_ptr = reinterpret_cast<const float*>(outputs_masks.data); // [1, 32, maskH, maskW]

    //if (outputs_masks.dims != 4)
    //    throw std::runtime_error("Unexpected output1 shape. Expected [1, 32, maskH, maskW].");

    const size_t num_features = outputs_boxes.h; // e.g 80 class + 4 bbox parms + 32 seg masks = 116
    const size_t num_detections = outputs_boxes.w;

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
    const int maskH = static_cast<int>(outputs_masks.h);
    const int maskW = static_cast<int>(outputs_masks.w);

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

void YOLOv11SegDetectorNcnn::drawSegmentationsAndBoxes(cv::Mat &image,
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

void YOLOv11SegDetectorNcnn::drawSegmentations(cv::Mat &image,
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

void YOLOv11SegDetectorNcnn::drawSegmentationMask(cv::Mat &image,
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

std::vector<Segmentation> YOLOv11SegDetectorNcnn::segment(const cv::Mat &image,
                                                      float confThreshold,
                                                      float iouThreshold)
{

    float *blobPtr = nullptr;
    std::vector<int64_t> inputShape = {1, 3, inputImageShape.height, inputImageShape.width};
    cv::Mat letterboxImg = preprocess(image, blobPtr, inputShape);
    
    size_t inputSize = utils::vectorProduct(inputShape);
    ncnn::Mat in = ncnn::Mat::from_pixels(letterboxImg.data, ncnn::Mat::PIXEL_RGB, inputShape[3], inputShape[2]);
    delete[] blobPtr;

    ncnn::Extractor ex = net.create_extractor();
    ex.input("in0", in); // adjust input name as needed
    ncnn::Mat out_boxes, out_masks;
    ex.extract("out0", out_boxes); // [num, 6] (x, y, w, h, conf, class)
    ex.extract("out1", out_masks); // [num, mask_dim, mask_dim]
   
    cv::Size letterboxSize(static_cast<int>(inputShape[3]), static_cast<int>(inputShape[2]));
    return postprocess(image.size(), letterboxSize, out_boxes, out_masks, confThreshold, iouThreshold);
}
