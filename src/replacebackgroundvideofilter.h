#ifndef REPLACEBACKGROUNDVIDEOFILTER_H
#define REPLACEBACKGROUNDVIDEOFILTER_H

#include <QImage>
#include <QUrl>
#include <QVideoSink>
#include <QVideoFrame>
#include <QVideoFrameInput>
#include <QThread>
#include <opencv2/core/core.hpp>
#include "yolo11seg.h"

class ReplaceBackgroundFilterRunable;

class ReplaceBackgroundVideoFilter : public QVideoFrameInput
{
    friend ReplaceBackgroundFilterRunable;
    Q_OBJECT
    /* input parameters */
    Q_PROPERTY(QString method READ getMethod WRITE setMethod)
    /* chroma key color 0 green to 1 blue */
    Q_PROPERTY(float keyColor READ getKeyColor WRITE setKeyColor)
    /* background image (only needed for processing the captured photo) */
    Q_PROPERTY(QUrl background WRITE setBackground)
    /* video frame input */
    Q_PROPERTY(QObject* videoSink WRITE setVideoSink)

public:
    ReplaceBackgroundVideoFilter(QObject *parent = nullptr);
    ~ReplaceBackgroundVideoFilter();

    void setMethod(QString method);
    void setKeyColor(float color);
    void setBackground(QImage const& image);
    Q_INVOKABLE void setBackground(QUrl& imagePath)
    {
        QImage img;
        QString fileName;

        if(imagePath.isLocalFile())
        {
            fileName = imagePath.toLocalFile();
        }
        else if(imagePath.toString().contains("qrc:"))
        {
            fileName = imagePath.toString().remove(0,3);
        }
        else
        {
            fileName = imagePath.toString();
        }

        if (img.load(fileName)) {
            setBackground(img);
        }
        else {
            qDebug() << "Failed to load image: " << imagePath;
        }
    }
    QString getMethod() const;
    float getKeyColor() const;

    void setVideoSink(QObject *videoSink);

    Q_INVOKABLE void processCapture(const QString& capture);

protected:
    enum class FilterMethod
    {
        CHROMA,
        NEURAL, //@todo implement neural algo
        NONE
    };

    float mKeyColor = 1.0;

    cv::Mat mBackgroundImage;

    FilterMethod mFilterMethod = FilterMethod::CHROMA;

    QVideoSink *mVideoSink;

    QThread mWorkerThread;

    ReplaceBackgroundFilterRunable* mRunable = nullptr;

    bool mProcessing = false;
    bool mCaptureProcessing = false;
protected slots:
    void processFrame(const QVideoFrame &frame);
    void onProcessingFinished(const QImage& maskImage);
    void onImageSaved(const QString& fileName);

signals:
    void asyncProcessFrame(const QVariant& frame, bool applyBackground, bool highResFilter);

    void captureProcessingFinished(const QString& fileName);
};


class ReplaceBackgroundFilterRunable : public QObject
{
    Q_OBJECT
public:
    ReplaceBackgroundFilterRunable(ReplaceBackgroundVideoFilter* filter);
public slots:
    void run(const QVariant &input, bool highResStill, bool highResFilter);
protected:
    cv::Mat chromaKeyMask(const cv::Mat& img, const cv::Scalar& lower_color, const cv::Scalar& upper_color);

    cv::Mat grabcutChromaKey(const cv::Mat& img, const cv::Scalar& lower_color, const cv::Scalar& upper_color);

    void prepareBackground(cv::Mat &bg, cv::Size size);

    void alphaBlend(const cv::Mat &src, const cv::Mat &bg, const cv::Mat& alpha, cv::Mat &dst);

    /**
     * @brief QImage -> CV_8UC4
     */
    static cv::Mat imageToMat8(const QImage &image);

    /**
     * @brief CV_8UC3|4 -> CV_8UC3
     */
    static void ensureC3(cv::Mat *mat);

    /**
     * @brief CV_8UC1|3|4 -> QImage
     */
    static QImage mat8ToImage(const cv::Mat &mat);

    /**
     * @brief YUV QVideoFrame -> CV_8UC3
     */
    static cv::Mat yuvFrameToMat8(const QVideoFrame &frame);

    /**
     * @brief CV_8UC3|4 -> YUV QVideoFrame
     */
    static QVideoFrame mat8ToYuvFrame(const cv::Mat &mat);

    /**
     * @brief CV_8UC3|4 -> YUV pre-alloced mem
     */
    static void mat8ToYuvFrame(const cv::Mat &mat, uchar *dst);

    /**
     * @brief helper function for getting an image from a video frame
     * @param frame input video frame
     * @return the image
     */
    static QImage imageWrapper(const QVideoFrame &frame);

    cv::Mat mMat;
    bool mYuv;
    ReplaceBackgroundVideoFilter* mFilter;

    YOLOv11SegDetector mYoloSegmentorFast;
    YOLOv11SegDetector mYoloSegmentorSlow;

signals:
    void processingFinished(const QImage& maskImage);
    void imageFileSaved(const QString& fileName);
    void error();
};

#endif // REPLACEBACKGROUNDVIDEOFILTER_H
