#include "segmentation.h"
#include <opencv2/opencv.hpp>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QFile>
#include "utils.h"

Yolo11Segementation::Yolo11Segementation(const std::string &labelsFile)
{
    classNames = utils::getClassNames(getModelRessourcePath(labelsFile));
    classColors = utils::generateColors(classNames);
}

void Yolo11Segementation::drawSegmentationsAndBoxes(cv::Mat &image,
                                                    const std::vector<Segmentation> &results,
                                                    float maskAlpha) const
{
    for (const auto &seg : results)
    {
        if (seg.conf < CONFIDENCE_THRESHOLD)
        {
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
        if (!seg.mask.empty())
        {
            // Ensure the mask is single-channel
            cv::Mat mask_gray;
            if (seg.mask.channels() == 3)
            {
                cv::cvtColor(seg.mask, mask_gray, cv::COLOR_BGR2GRAY);
            }
            else
            {
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

void Yolo11Segementation::drawSegmentations(cv::Mat &image,
                                            const std::vector<Segmentation> &results,
                                            float maskAlpha) const
{
    for (const auto &seg : results)
    {
        if (seg.conf < CONFIDENCE_THRESHOLD)
        {
            continue;
        }
        cv::Scalar color = classColors[seg.classId % classColors.size()];

        // -----------------------------
        // Draw Segmentation Mask Only
        // -----------------------------
        if (!seg.mask.empty())
        {
            // Ensure the mask is single-channel
            cv::Mat mask_gray;
            if (seg.mask.channels() == 3)
            {
                cv::cvtColor(seg.mask, mask_gray, cv::COLOR_BGR2GRAY);
            }
            else
            {
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

void Yolo11Segementation::drawSegmentationMask(cv::Mat &image,
                                               const std::vector<Segmentation> &results,
                                               const std::vector<int> &classesFilter) const
{
    for (const auto &seg : results)
    {
        if (seg.conf < CONFIDENCE_THRESHOLD)
        {
            continue;
        }

        if (!classesFilter.empty())
        {
            if (std::find(classesFilter.begin(), classesFilter.end(), seg.classId) == classesFilter.end())
            {
                // class id not in filter
                continue;
            }
        }

        // -----------------------------
        // Draw Segmentation Mask Only
        // -----------------------------
        if (!seg.mask.empty())
        {
            // Ensure the mask is single-channel
            cv::Mat mask_gray;
            if (seg.mask.channels() == 3)
            {
                cv::cvtColor(seg.mask, mask_gray, cv::COLOR_BGR2GRAY);
            }
            else
            {
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

std::string Yolo11Segementation::getModelRessourcePath(const std::string &filename) const
{
    QString ressourcePathGeneric = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "models", QStandardPaths::LocateDirectory);
    QString ressourcePathApp = QStandardPaths::locate(QStandardPaths::AppDataLocation, "models", QStandardPaths::LocateDirectory);
    if (ressourcePathApp.isEmpty() && ressourcePathGeneric.isEmpty())
    {
        throw std::runtime_error("Failed to locate the models directory.");
    }
    QString ressourcePath = "";

    if (QFile(ressourcePathApp + "/" + QString::fromStdString(filename)).exists())
    {
        ressourcePath = ressourcePathApp + "/" + QString::fromStdString(filename);
        qDebug() << "Using model from app data path:" << ressourcePath;
    }
    else if (QFile(ressourcePathGeneric + "/" + QString::fromStdString(filename)).exists())
    {
        ressourcePath = ressourcePathGeneric + "/" + QString::fromStdString(filename);
        qDebug() << "Using model from generic data path:" << ressourcePath;
    }
    else
    {
        throw std::runtime_error("Model file not found: " + filename);
    }

    return ressourcePath.toStdString();
}
