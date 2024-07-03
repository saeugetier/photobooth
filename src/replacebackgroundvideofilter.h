#ifndef REPLACEBACKGROUNDVIDEOFILTER_H
#define REPLACEBACKGROUNDVIDEOFILTER_H

#include <QAbstractVideoFilter>
#include <opencv2/core/core.hpp>

class ReplaceBackgroundFilterRunable;

class ReplaceBackgroundVideoFilter : public QAbstractVideoFilter
{
    friend ReplaceBackgroundFilterRunable;
    Q_OBJECT
    Q_PROPERTY(QString method READ getMethod WRITE setMethod CONSTANT)
    Q_PROPERTY(float chromaA1 READ getChromaA1 WRITE setChromaA1 CONSTANT)
    Q_PROPERTY(float chromaA2 READ getChromaA2 WRITE setChromaA2 CONSTANT)
    Q_PROPERTY(QImage background WRITE setBackground)
public:
    ReplaceBackgroundVideoFilter(QObject *parent = nullptr) : QAbstractVideoFilter(parent),
        mBackgroundImage(320, 240, CV_8UC3, cv::Scalar(0, 0, 0)) {}
    QVideoFilterRunnable *createFilterRunnable() override;
    void setMethod(QString method);
    void setChromaA1(float a1);
    void setChromaA2(float a2);
    void setBackground(QImage const& image);
    QString getMethod() const;
    float getChromaA1() const;
    float getChromaA2() const;
protected:
    enum class FilterMethod
    {
        CHROMA,
        NEURAL, //@todo implement neural algo
        NONE
    };

    float mChromaA1 = 5.5;
    float mChromaA2 = 1.0;

    cv::Mat mBackgroundImage;

    FilterMethod mFilterMethod = FilterMethod::CHROMA;
};

class ReplaceBackgroundFilterRunable : public QVideoFilterRunnable
{
public:
    ReplaceBackgroundFilterRunable(ReplaceBackgroundVideoFilter* filter);
    QVideoFrame run(QVideoFrame *input, const QVideoSurfaceFormat &surfaceFormat, QVideoFilterRunnable::RunFlags flags) override;
protected:
    cv::Mat grabcutChromaKey(const cv::Mat& img, const cv::Mat& bg_img, const cv::Scalar& lower_color, const cv::Scalar& upper_color);

    void prepareBackground(cv::Mat &bg, cv::Size size);

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
};

#endif // REPLACEBACKGROUNDVIDEOFILTER_H
