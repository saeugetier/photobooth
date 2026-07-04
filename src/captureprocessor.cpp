#include "captureprocessor.h"
#include <QDebug>

CaptureProcessor::CaptureProcessor(QObject *parent)
    : QObject{parent}
{}

int CaptureProcessor::getRotation() const
{
    return mRotation;
}

void CaptureProcessor::setRotation(int rotation)
{
    if (rotation < 0 || rotation >= 360) {
        qWarning() << "Invalid rotation value. Must be between 0 and 359.";
        return;
    }

    if(rotation % 90)
    {
        qWarning() << "Rotation value should be a multiple of 90 degrees. Adjusting to nearest valid value.";
        rotation = (rotation / 90) * 90; // Adjust to nearest multiple of 90
    }

    mRotation = rotation;
}

void CaptureProcessor::saveCapture(QImage preview, const QString &filePath)
{
    if (preview.isNull() || filePath.isEmpty()) {
        qWarning() << "Invalid image or file path.";
        return;
    }

    QTransform transform;
    transform.rotate(mRotation);
    QImage rotated = preview.transformed(transform);

    if (rotated.save(filePath)) {
        emit captureSaved(filePath);
    } else {
        qWarning() << "Failed to save capture to" << filePath;
    }
}
