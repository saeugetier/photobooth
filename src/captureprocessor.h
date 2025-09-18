#ifndef CAPTUREPROCESSOR_H
#define CAPTUREPROCESSOR_H

#include <QObject>
#include <QImage>

class CaptureProcessor : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int rotation READ getRotation WRITE setRotation)
public:
    explicit CaptureProcessor(QObject *parent = nullptr);
    int getRotation() const;
    void setRotation(int rotation);

public slots:
    void saveCapture(QImage preview, const QString &filePath);

signals:
    void captureSaved(QString filePath);
protected:
    int mRotation;
};

#endif // CAPTUREPROCESSOR_H
