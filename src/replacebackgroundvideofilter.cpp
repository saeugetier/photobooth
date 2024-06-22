#include "replacebackgroundvideofilter.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <QImage>
#include <QAbstractVideoBuffer>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLFramebufferObject>
#include "private/qvideoframe_p.h"

QVideoFilterRunnable* ReplaceBackgroundVideoFilter::createFilterRunnable()
{
    return new ReplaceBackgroundFilterRunable(this);
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

ReplaceBackgroundFilterRunable::ReplaceBackgroundFilterRunable(ReplaceBackgroundVideoFilter* filter) : mFilter(filter)
{
}

QVideoFrame ReplaceBackgroundFilterRunable::run(QVideoFrame *input, const QVideoSurfaceFormat &surfaceFormat, QVideoFilterRunnable::RunFlags flags)
{
    Q_UNUSED(surfaceFormat);
    Q_UNUSED(flags);

    // Supports YUV (I420 and NV12) and RGB. The GL path is readback-based and slow.
    if (!input->isValid()
        || (input->handleType() != QAbstractVideoBuffer::NoHandle && input->handleType() != QAbstractVideoBuffer::GLTextureHandle)) {
        qWarning("Invalid input format");
        return QVideoFrame();
    }

    input->map(QAbstractVideoBuffer::ReadOnly);
    if (input->pixelFormat() == QVideoFrame::Format_YUV420P || input->pixelFormat() == QVideoFrame::Format_NV12) {
        mYuv = true;
        mMat = yuvFrameToMat8(*input);
    } else {
        mYuv = false;
        QImage wrapper = imageWrapper(*input);
        if (wrapper.isNull()) {
            if (input->handleType() == QAbstractVideoBuffer::NoHandle)
                input->unmap();
            return *input;
        }
        mMat = imageToMat8(wrapper);
    }
    ensureC3(&mMat);
    if (input->handleType() == QAbstractVideoBuffer::NoHandle)
        input->unmap();

    cv::Mat res_front, res_back, res;

    std::vector<cv::Mat> channels, img_channels;

    cv::Mat alpha = cv::Mat::zeros(cv::Size(input->width(), input->height()), CV_32F);

    mMat.convertTo(mMat, CV_32F, 1.0/255);

    split(mMat, channels);

    // use algorithm: https://smirnov-am.github.io/chromakeying/
    const float a1 = mFilter->getChromaA1();
    const float a2 = mFilter->getChromaA2();
    alpha = cv::Scalar::all(1.0) - a1*(channels[1] - a2*channels[0]);
    // keep alpha in [0, 1] range
    threshold(alpha, alpha, 1, 1, cv::THRESH_TRUNC);
    threshold(-1*alpha, alpha, 0, 0, cv::THRESH_TRUNC);
    alpha = -1 * alpha;

    for (int i=0; i < 3; ++i) {
        multiply(alpha, channels[i], channels[i]);
    }

    merge(channels, res_front);
    res_front.convertTo(mMat, CV_8UC3, 255);

    // Output is an RGB video frame.
    return QVideoFrame(mat8ToImage(mMat));
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
    Q_ASSERT(frame.handleType() == QAbstractVideoBuffer::NoHandle && frame.isReadable());
    Q_ASSERT(frame.pixelFormat() == QVideoFrame::Format_YUV420P || frame.pixelFormat() == QVideoFrame::Format_NV12);

    cv::Mat tmp(frame.height() + frame.height() / 2, frame.width(), CV_8UC1, (uchar *) frame.bits());
    cv::Mat result(frame.height(), frame.width(), CV_8UC3);
    cvtColor(tmp, result, frame.pixelFormat() == QVideoFrame::Format_YUV420P ? cv::COLOR_YUV2BGR_YV12 : cv::COLOR_YUV2BGR_NV12);
    return result;
}

class YUVBuffer : public QAbstractVideoBuffer
{
public:
    YUVBuffer(cv::Mat *mat) : QAbstractVideoBuffer(NoHandle), m_mode(NotMapped) {
        m_yuvMat.reset(mat);
    }
    MapMode mapMode() const Q_DECL_OVERRIDE { return m_mode; }
    uchar *map(MapMode mode, int *numBytes, int *bytesPerLine) Q_DECL_OVERRIDE {
        if (mode != NotMapped && m_mode == NotMapped) {
            if (numBytes)
                *numBytes = m_yuvMat->rows * m_yuvMat->cols;
            if (bytesPerLine)
                *bytesPerLine = m_yuvMat->cols;
            m_mode = mode;
            return m_yuvMat->data;
        }
        return 0;
    }
    void unmap() Q_DECL_OVERRIDE { m_mode = NotMapped; }
    QVariant handle() const Q_DECL_OVERRIDE { return 0; }

private:
    MapMode m_mode;
    QScopedPointer<cv::Mat> m_yuvMat;
};

QVideoFrame ReplaceBackgroundFilterRunable::mat8ToYuvFrame(const cv::Mat &mat)
{
    Q_ASSERT(mat.type() == CV_8UC3 || mat.type() == CV_8UC4);

    cv::Mat *m = new cv::Mat;
    cvtColor(mat, *m, mat.type() == CV_8UC4 ? cv::COLOR_BGRA2YUV_YV12 : cv::COLOR_BGR2YUV_YV12);
    return QVideoFrame(new YUVBuffer(m), QSize(mat.cols, mat.rows), QVideoFrame::Format_YUV420P);
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
    if (frame.handleType() == QAbstractVideoBuffer::GLTextureHandle) {
        // Slow and inefficient path. Ideally what's on the GPU should remain on the GPU, instead of readbacks like this.
        QImage img(frame.width(), frame.height(), QImage::Format_RGBA8888);
        GLuint textureId = frame.handle().toUInt();
        QOpenGLContext *ctx = QOpenGLContext::currentContext();
        QOpenGLFunctions *f = ctx->functions();
        GLuint fbo;
        f->glGenFramebuffers(1, &fbo);
        GLuint prevFbo;
        f->glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint *) &prevFbo);
        f->glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        f->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0);
        f->glReadPixels(0, 0, frame.width(), frame.height(), GL_RGBA, GL_UNSIGNED_BYTE, img.bits());
        f->glBindFramebuffer(GL_FRAMEBUFFER, prevFbo);
        return img;
    } else
#endif // QT_NO_OPENGL
    {
        if (!frame.isReadable()) {
            qWarning("imageWrapper: No mapped image data available for read");
            return QImage();
        }

        QImage::Format fmt = QVideoFrame::imageFormatFromPixelFormat(frame.pixelFormat());
        if (fmt != QImage::Format_Invalid)
            return QImage(frame.bits(), frame.width(), frame.height(), fmt);
        else
            return qt_imageFromVideoFrame(frame);

        qWarning("imageWrapper: No matching QImage format");
    }

    return QImage();
}

#ifndef QT_NO_OPENGL
class TextureBuffer : public QAbstractVideoBuffer
{
public:
    TextureBuffer(uint id) : QAbstractVideoBuffer(GLTextureHandle), m_id(id) { }
    MapMode mapMode() const { return NotMapped; }
    uchar *map(MapMode, int *, int *) { return 0; }
    void unmap() { }
    QVariant handle() const { return QVariant::fromValue<uint>(m_id); }

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
QVideoFrame frameFromTexture(uint textureId, const QSize &size, QVideoFrame::PixelFormat format)
{
#ifndef QT_NO_OPENGL
    return QVideoFrame(new TextureBuffer(textureId), size, format);
#else
    return QVideoFrame();
#endif // QT_NO_OPENGL
}
