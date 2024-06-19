#ifndef REPLACEBACKGROUNDVIDEOFILTER_H
#define REPLACEBACKGROUNDVIDEOFILTER_H

#include <QAbstractVideoFilter>

class ReplaceBackgroundVideoFilter : public QAbstractVideoFilter
{
public:
    QVideoFilterRunnable *createFilterRunnable() override;
};

class ReplaceBackgroundFilterRunable : public QVideoFilterRunnable
{
public:
    QVideoFrame run(QVideoFrame *input, const QVideoSurfaceFormat &surfaceFormat, QVideoFilterRunnable::RunFlags flags) override;
};

#endif // REPLACEBACKGROUNDVIDEOFILTER_H
