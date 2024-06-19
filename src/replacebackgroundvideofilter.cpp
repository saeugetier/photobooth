#include "replacebackgroundvideofilter.h"


QVideoFilterRunnable* ReplaceBackgroundVideoFilter::createFilterRunnable()
{
    return new ReplaceBackgroundFilterRunable;
}

QVideoFrame ReplaceBackgroundFilterRunable::run(QVideoFrame *input, const QVideoSurfaceFormat &surfaceFormat, QVideoFilterRunnable::RunFlags flags)
{
    return *input;
}
