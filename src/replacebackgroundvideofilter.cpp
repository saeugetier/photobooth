    #include "replacebackgroundvideofilter.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <QImage>
#include <QAbstractVideoBuffer>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLFramebufferObject>
#include <QtConcurrent>

//QVideoFilterRunnable* ReplaceBackgroundVideoFilter::createFilterRunnable()
//{
//    return new ReplaceBackgroundFilterRunable(this);
//}

ReplaceBackgroundVideoFilter::ReplaceBackgroundVideoFilter(QObject *parent) : QVideoFrameInput(parent),
    mBackgroundImage(320, 240, CV_8UC3, cv::Scalar(0, 0, 0)), mRunable(new ReplaceBackgroundFilterRunable(this))
{
    mRunable->moveToThread(&mWorkerThread);

    connect(&mWorkerThread, &QThread::finished, mRunable, &QObject::deleteLater);
    connect(this, &ReplaceBackgroundVideoFilter::asyncProcessFrame, mRunable, &ReplaceBackgroundFilterRunable::run);
    connect(mRunable, &ReplaceBackgroundFilterRunable::processingFinished, this, &ReplaceBackgroundVideoFilter::onProcessingFinished);

    mWorkerThread.start();
}

ReplaceBackgroundVideoFilter::~ReplaceBackgroundVideoFilter()
{
    mWorkerThread.quit();
    mWorkerThread.wait();
}

void ReplaceBackgroundVideoFilter::setChromaA1(float a1)
{
    mChromaA1 = a1;
}

void ReplaceBackgroundVideoFilter::setChromaA2(float a2)
{
    mChromaA2 = a2;
}

void ReplaceBackgroundVideoFilter::setMethod(QString method)
{
    if(method.contains("Chroma"))
    {
        mFilterMethod = FilterMethod::CHROMA;
    }
    else if(method.contains("Neural Net"))
    {
        mFilterMethod = FilterMethod::NEURAL;
    }
    else
    {
        mFilterMethod = FilterMethod::NONE;
    }
}

void ReplaceBackgroundVideoFilter::setBackground(QImage const& image)
{
    QImage img = image.convertToFormat(QImage::Format_RGB32).rgbSwapped();
    cv::Mat tmp(img.height(), img.width(), CV_8UC4, (void *) img.bits(), img.bytesPerLine());
    mBackgroundImage = tmp.clone();
}

float ReplaceBackgroundVideoFilter::getChromaA1() const
{
    return mChromaA1;
}

float ReplaceBackgroundVideoFilter::getChromaA2() const
{
    return mChromaA2;
}

QString ReplaceBackgroundVideoFilter::getMethod() const
{
    if(mFilterMethod == FilterMethod::CHROMA)
    {
        return QString("Chroma");
    }
    else if(mFilterMethod == FilterMethod::NEURAL)
    {
        return QString("Neural Net");
    }
    else
    {
        return QString("None");
    }
}

void ReplaceBackgroundVideoFilter::setVideoSink(QObject *videoSink)
{
    mVideoSink = qobject_cast<QVideoSink*>(videoSink);

    connect(mVideoSink, &QVideoSink::videoFrameChanged, this, &ReplaceBackgroundVideoFilter::processFrame);
}

void ReplaceBackgroundVideoFilter::processFrame(const QVideoFrame &frame)
{
    if(!mProcessing)
    {
        mProcessing = true;
        emit asyncProcessFrame(frame);
    }
}

void ReplaceBackgroundVideoFilter::onProcessingFinished(const QImage& maskImage)
{
    if(!maskImage.isNull())
    {
        sendVideoFrame(QVideoFrame(maskImage));
    }

    mProcessing = false;
}

ReplaceBackgroundFilterRunable::ReplaceBackgroundFilterRunable(ReplaceBackgroundVideoFilter* filter) : mFilter(filter)
{
}

void ReplaceBackgroundFilterRunable::run(const QVideoFrame& input)
{
    // Supports YUV (I420 and NV12) and RGB. The GL path is readback-based and slow.
    if (!input.isValid()
        || (input.handleType() != QVideoFrame::HandleType::NoHandle && input.handleType() != QVideoFrame::RhiTextureHandle)) {
        qWarning("Invalid input format");
        emit processingFinished(QImage());
    }

    QVideoFrame frame = input;

    if (!frame.map(QVideoFrame::ReadOnly)) {
        qWarning() << "Failed to map QVideoFrame";
        emit processingFinished(QImage());
        return;
    }

    QImage image = frame.toImage();
    mMat = imageToMat8(image);

    frame.unmap();
    ensureC3(&mMat);

    prepareBackground(mFilter->mBackgroundImage, cv::Size(frame.width(), frame.height()));

    switch(mFilter->mFilterMethod)
    {
    case ReplaceBackgroundVideoFilter::FilterMethod::CHROMA:
    {
        cv::Scalar lower_green(0, 80, 80);
        cv::Scalar upper_green(255, 255, 255);
        mMat = chromaKeyMask(mMat, lower_green, upper_green);
    }
    break;

    case ReplaceBackgroundVideoFilter::FilterMethod::NEURAL:
        break;

    case ReplaceBackgroundVideoFilter::FilterMethod::NONE:
        break;
    }

    // Output is an RGB video frame.
    emit processingFinished(mat8ToImage(mMat));
}

void ReplaceBackgroundFilterRunable::prepareBackground(cv::Mat &bg, cv::Size size)
{
    cv::resize(bg, bg, size);
}

cv::Mat ReplaceBackgroundFilterRunable::chromaKeyMask(const cv::Mat& img, const cv::Scalar& lower_color, const cv::Scalar& upper_color)
{
    // Convert the image from BGR to YUV
    cv::Mat yuv_img;
    cv::Mat mask;
    cvtColor(img, yuv_img, cv::COLOR_BGR2YUV);

    // Define the mask based on the color range
    inRange(yuv_img, lower_color, upper_color, mask);

    return mask;
}

cv::Mat ReplaceBackgroundFilterRunable::grabcutChromaKey(const cv::Mat& img, const cv::Mat& bg_img, const cv::Scalar& lower_color, const cv::Scalar& upper_color) {
    // Convert the image from BGR to YUV
    cv::Mat yuv_img;
    cv::Mat result;
    cvtColor(img, yuv_img, cv::COLOR_BGR2YUV);

    // Define the mask based on the color range
    cv::Mat mask;
    inRange(yuv_img, lower_color, upper_color, mask);

    int num_foreground_pixels = countNonZero(mask);
    int num_background_pixels = mask.total() - num_foreground_pixels;

    if(num_background_pixels != 0)
    {
        // Initialize GrabCut mask
        cv::Mat grabcut_mask = cv::Mat::zeros(img.size(), CV_8UC1);

        // Set probable foreground and background areas
        grabcut_mask.setTo(cv::Scalar::all(cv::GC_PR_BGD), mask == 0);
        grabcut_mask.setTo(cv::Scalar::all(cv::GC_PR_FGD), mask == 255);

        // Run GrabCut algorithm
        cv::Mat bgd_model, fgd_model;
        cv::Rect rect(1, 1, img.cols - 2, img.rows - 2);
        //cv::Rect rect(0, 0, img.cols, img.rows); // Use entire image
        cv::grabCut(img, grabcut_mask, rect, bgd_model, fgd_model, 1, cv::GC_INIT_WITH_MASK);

        // Create final mask
        cv::Mat final_mask;
        compare(grabcut_mask, cv::GC_PR_FGD, final_mask, cv::CMP_EQ);
        final_mask = final_mask | (grabcut_mask == cv::GC_FGD);

        // Extract the foreground
        cv::Mat fg, bg;
        img.copyTo(fg, final_mask);
        bg_img.copyTo(bg, 1 - final_mask);

        // Combine the foreground and background
        add(fg, bg, result);
    }
    else
    {
        img.copyTo(result);
    }

    //cv::Mat fg, bg;
    //img.copyTo(fg, mask);
    //bg_img.copyTo(bg, ~mask);

    // Combine the foreground and background
    //add(fg, bg, result);

    return result;
}

cv::Mat ReplaceBackgroundFilterRunable::imageToMat8(const QImage &image)
{
    QImage img = image.convertToFormat(QImage::Format_RGB32).rgbSwapped();
    cv::Mat tmp(img.height(), img.width(), CV_8UC4, (void *) img.bits(), img.bytesPerLine());
    return tmp.clone();
}

void ReplaceBackgroundFilterRunable::ensureC3(cv::Mat *mat)
{
    Q_ASSERT(mat->type() == CV_8UC3 || mat->type() == CV_8UC4);
    if (mat->type() != CV_8UC3) {
        cv::Mat tmp;
        cvtColor(*mat, tmp, cv::COLOR_BGRA2BGR);
        *mat = tmp;
    }
}

QImage ReplaceBackgroundFilterRunable::mat8ToImage(const cv::Mat &mat)
{
    switch (mat.type()) {
    case CV_8UC1:
    {
        QVector<QRgb> ct;
        for (int i = 0; i < 256; ++i)
            ct.append(qRgb(i, i, i));
        QImage result(mat.data, mat.cols, mat.rows, (int) mat.step, QImage::Format_Indexed8);
        result.setColorTable(ct);
        return result.copy();
    }
    case CV_8UC3:
    {
        cv::Mat tmp;
        cvtColor(mat, tmp, cv::COLOR_BGR2BGRA);
        return mat8ToImage(tmp);
    }
    case CV_8UC4:
    {
        QImage result(mat.data, mat.cols, mat.rows, (int) mat.step, QImage::Format_RGB32);
        return result.rgbSwapped();
    }
    default:
        qWarning("Unhandled Mat format %d", mat.type());
        return QImage();
    }
}

cv::Mat ReplaceBackgroundFilterRunable::yuvFrameToMat8(const QVideoFrame &frame)
{
    Q_ASSERT(frame.handleType() == QVideoFrame::HandleType::NoHandle && frame.isReadable());
    Q_ASSERT(frame.pixelFormat() == QVideoFrameFormat::Format_YUV420P || frame.pixelFormat() == QVideoFrameFormat::Format_NV12);

    cv::Mat tmp(frame.height() + frame.height() / 2, frame.width(), CV_8UC1, (uchar *) frame.bits(0));
    cv::Mat result(frame.height(), frame.width(), CV_8UC3);
    cvtColor(tmp, result, frame.pixelFormat() == QVideoFrameFormat::Format_YUV420P ? cv::COLOR_YUV2BGR_YV12 : cv::COLOR_YUV2BGR_NV12);
    return result;
}

class YUVBuffer : public QAbstractVideoBuffer
{
public:
    YUVBuffer(cv::Mat *mat) : m_mode(QVideoFrame::NotMapped) {
        m_yuvMat.reset(mat);
    }
    QVideoFrameFormat format() const Q_DECL_OVERRIDE { return QVideoFrameFormat(QSize(m_yuvMat->cols, m_yuvMat->rows), QVideoFrameFormat::Format_YUV420P); }
    QAbstractVideoBuffer::MapData map(QVideoFrame::MapMode mode) Q_DECL_OVERRIDE {
        QAbstractVideoBuffer::MapData data;

        if (mode != QVideoFrame::NotMapped && m_mode == QVideoFrame::NotMapped) {

            data.planeCount = 1;
            data.dataSize[0] = m_yuvMat->rows * m_yuvMat->cols;

            data.bytesPerLine[0] = m_yuvMat->cols;
            m_mode = mode;

            data.data[0] = m_yuvMat->data;
            return data;
        }
        return data;
    }
    void unmap() Q_DECL_OVERRIDE { m_mode = QVideoFrame::NotMapped; }
private:
    QVideoFrame::MapMode m_mode;
    QScopedPointer<cv::Mat> m_yuvMat;
};

QVideoFrame ReplaceBackgroundFilterRunable::mat8ToYuvFrame(const cv::Mat &mat)
{
    Q_ASSERT(mat.type() == CV_8UC3 || mat.type() == CV_8UC4);

    cv::Mat *m = new cv::Mat;
    cvtColor(mat, *m, mat.type() == CV_8UC4 ? cv::COLOR_BGRA2YUV_YV12 : cv::COLOR_BGR2YUV_YV12);
    return QVideoFrame(std::unique_ptr<QAbstractVideoBuffer>(new YUVBuffer(m)));
}

void ReplaceBackgroundFilterRunable::mat8ToYuvFrame(const cv::Mat &mat, uchar *dst)
{
    cv::Mat m(mat.rows + mat.rows / 2, mat.cols, CV_8UC1, dst);
    cvtColor(mat, m, mat.type() == CV_8UC4 ? cv::COLOR_BGRA2YUV_YV12 : cv::COLOR_BGR2YUV_YV12);
}

/*
  Returns a QImage that wraps the given video frame.

  This is suitable only for QAbstractVideoBuffer::NoHandle frames with RGB (or BGR)
  data. YUV is not supported here.

  The QVideoFrame must be mapped and kept mapped as long as the wrapping QImage
  exists.

  As a convenience the function also supports frames with a handle type of
  QAbstractVideoBuffer::GLTextureHandle. This allows creating a system memory backed
  QVideoFrame containing the image data from an OpenGL texture. However, readback is a
  slow operation and may stall the GPU pipeline and should be avoided in production code.
*/
QImage ReplaceBackgroundFilterRunable::imageWrapper(const QVideoFrame &frame)
{
#ifndef QT_NO_OPENGL
    if (frame.handleType() == QVideoFrame::RhiTextureHandle) {
        // Slow and inefficient path. Ideally what's on the GPU should remain on the GPU, instead of readbacks like this.
        QImage img(frame.width(), frame.height(), QImage::Format_RGBA8888);
        // GLuint textureId = frame.handle().toUInt();
        // QOpenGLContext *ctx = QOpenGLContext::currentContext();
        // QOpenGLFunctions *f = ctx->functions();
        // GLuint fbo;
        // f->glGenFramebuffers(1, &fbo);
        // GLuint prevFbo;
        // f->glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint *) &prevFbo);
        // f->glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        // f->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0);
        // f->glReadPixels(0, 0, frame.width(), frame.height(), GL_RGBA, GL_UNSIGNED_BYTE, img.bits());
        // f->glBindFramebuffer(GL_FRAMEBUFFER, prevFbo);
        return img;
    } else
#endif // QT_NO_OPENGL
    {
        if (!frame.isReadable()) {
            qWarning("imageWrapper: No mapped image data available for read");
            return QImage();
        }

        return frame.toImage();

        qWarning("imageWrapper: No matching QImage format");
    }

    return QImage();
}

#ifndef QT_NO_OPENGL
class TextureBuffer : public QAbstractVideoBuffer
{
public:
    TextureBuffer(uint id) : QAbstractVideoBuffer(), m_id(id) { }
    QVideoFrame::MapMode mapMode() const { return QVideoFrame::NotMapped; }
    QAbstractVideoBuffer::MapData map(QVideoFrame::MapMode) override { return QAbstractVideoBuffer::MapData(); }
    void unmap() override { }
    QVariant handle() const { return QVariant::fromValue<uint>(m_id); }
    QVideoFrameFormat format() const override {return QVideoFrameFormat(); }

private:
    GLuint m_id;
};
#endif // QT_NO_OPENGL

/*
  Creates and returns a new video frame wrapping the OpenGL texture textureId. The size
  must be passed in size, together with the format of the underlying image data in
  format. When the texture originates from a QImage, use
  QVideoFrame::imageFormatFromPixelFormat() to get a suitable format. Ownership is not
  altered, the new QVideoFrame will not destroy the texture.
*/
QVideoFrame frameFromTexture(uint textureId, const QSize &size, QVideoFrameFormat::PixelFormat format)
{
#ifndef QT_NO_OPENGL
    return QVideoFrame(std::unique_ptr<QAbstractVideoBuffer>(new TextureBuffer(textureId)));
#else
    return QVideoFrame();
#endif // QT_NO_OPENGL
}
