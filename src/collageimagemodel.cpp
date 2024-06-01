#include "collageimagemodel.h"
#include "collagemodelfactory.h"
#include <QTransform>
#include <QDebug>

CollageImageModel::CollageImageModel(QObject *parent) : QAbstractListModel(parent)
{
    mImages.clear();
}

CollageImageModel::~CollageImageModel()
{
    for(int i = 0; i < mImages.length(); i++)
    {
        delete mImages.at(i);
    }
    mImages.clear();
}

bool CollageImageModel::parseXml(const QDomNode& node)
{
    if("collage" != node.nodeName())
    {
        mErrorMsg = "wrong node. Expected node name 'collage'";
        mLine = node.lineNumber();
        return false;
    }

    QDomElement element = node.toElement();

    QDomNodeList backgroundNode = element.elementsByTagName("background");
    if(backgroundNode.count() == 1)
    {
        mBackgroundImage = backgroundNode.item(0).toElement().text();
    }
    else if(backgroundNode.count() > 1)
    {
        mErrorMsg = "multiple background nodes";
        mLine = element.lineNumber();
        return false;
    }
    else
    {
        mErrorMsg = "at least one background nodes required";
        mLine = element.lineNumber();
        return false;
    }

    QDomNodeList foregroundNode = element.elementsByTagName("foreground");
    if(foregroundNode.count() == 1)
    {
        mForegroundImage = foregroundNode.item(0).toElement().text();
    }
    else if(foregroundNode.count() > 1)
    {
        mErrorMsg = "multiple foreground nodes";
        mLine = element.lineNumber();
        return false;
    }

    QDomElement sizeElement = element.firstChildElement("size");
    if(sizeElement.isElement())
    {
        if(sizeElement.hasAttributes() && sizeElement.toElement().hasAttribute("width") && sizeElement.toElement().hasAttribute("height"))
        {
            bool ok;
            QString x = sizeElement.toElement().attribute("width");
            mPixelSize.setWidth(x.toInt(&ok));
            if(!ok)
            {
                mErrorMsg = "size 'width' must be defined as float";
                mLine = sizeElement.lineNumber();
                return false;
            }

            QString y = sizeElement.toElement().attribute("height");
            mPixelSize.setHeight(y.toInt(&ok));
            if(!ok)
            {
                mErrorMsg = "size 'height' must be defined as float";
                mLine = sizeElement.lineNumber();
                return false;
            }
        }
        else
        {
            mErrorMsg = "size must be defined be 'width' and 'height' attributes";
            mLine = sizeElement.lineNumber();
            return false;
        }
    }
    else
    {
        mErrorMsg = "at least one size nodes required";
        mLine = element.lineNumber();
        return false;
    }

    QDomNodeList imagesNode = element.elementsByTagName("images");
    if(imagesNode.length() != 1)
    {
        if(imagesNode.length() == 0)
            mErrorMsg = "no images node";
        else
            mErrorMsg = "multiple images nodes";
        mLine = element.lineNumber();
        return false;
    }

    QDomNodeList imageNodes = imagesNode.item(0).toElement().elementsByTagName("image");
    if(imageNodes.length() < 1)
    {
        mErrorMsg = "at least one image must be defined";
        mLine = imageNodes.item(0).lineNumber();
        return false;
    }

    for(int i = 0; i < imageNodes.length(); i++)
    {
        bool result;
        CollageImage* image = new CollageImage(mPixelSize);
        result = image->parseXml(imageNodes.item(i));
        if(!result)
        {
            mErrorMsg = image->errorMsg();
            mLine = image->lineNumber();
            return false;
        }
        mImages.append(image);
    }

    return true;
}

int CollageImageModel::rowCount(const QModelIndex &parent) const
{
    (void)(parent);
    return mImages.size();
}

QVariant CollageImageModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() > mImages.count())
        return QVariant();

    CollageImage *image = mImages[index.row()];
    if (role == ImagePathRole)
        return image->imagePath();
    else if(role == ImageRectRole)
        return image->imageRect();
    else if(role == RotationRole)
        return image->rotation();
    else if(role == BorderImageRole)
        return image->borderImage();
    else if(role == BorderRectRole)
        return image->borderRect();
    else if(role == EffectSelectableRole)
        return image->effectSelectable();
    else if(role == EffectRole)
        return image->effect();
    return QVariant();
}

QHash<int, QByteArray> CollageImageModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[ImageRectRole] = "imageRect";
    roles[RotationRole] = "imageRotation";
    roles[ImagePathRole] = "imagePath";
    roles[BorderImageRole] = "borderImage";
    roles[BorderRectRole] = "borderRect";
    roles[EffectSelectableRole] = "effectSelectable";
    roles[EffectRole] = "effect";
    return roles;
}

QUrl CollageImageModel::backgroundImage() const
{
    return mBackgroundImage;
}

QUrl CollageImageModel::foregroundImage() const
{
    return mForegroundImage;
}

bool CollageImageModel::addImagePath(QUrl source, QString effect)
{
    if(!collageFull()) {
        int i = 0;
        for(;i < rowCount(); i++)
        {
            if(mImages[i]->imagePath() == QUrl(""))
                break;
        }
        mImages[i]->setImage(source);
        if(!mImages[i]->effectSelectable() || effect == "")
        {
            mImages[i]->setEffect(mImages[i]->effectPreset());
        }
        else
        {
            mImages[i]->setEffect(effect);
        }
        QModelIndex ii = index(i,0);
        emit dataChanged(ii, ii);
        if(collageFull())
        {
            emit collageFullChanged(true);
        }
        return true;
    }
    else {
        return false;
    }
}

void CollageImageModel::clearImagePathes()
{
    for(int i = 0; i < mImages.length(); i++)
    {
        mImages[i]->setImage(QUrl(""));
    }

    emit collageFullChanged(false);

    emit dataChanged(this->index(0,0),this->index(rowCount()-1,0));
}

bool CollageImageModel::clearImagePath(int index)
{
    if(index < rowCount())
    {
        if(collageFull())
        {
            emit collageFullChanged(false);
        }

        if(mImages[index]->imagePath() != QUrl(""))
        {
            mImages[index]->setImage(QUrl(""));

#ifdef SORT_AFTER_DELETE
            for(int i = 0; i < rowCount() - 1; i++)
            {
                if(mImages[i]->imagePath() == QUrl(""))
                {
                    for(int j = i + 1; j < rowCount(); j++)
                    {
                        if(mImages[j]->imagePath() != QUrl(""))
                        {
                            mImages[i]->setImage(mImages[j]->imagePath());
                            mImages[j]->setImage(QUrl(""));
                            break;
                        }
                    }
                }
            }
#endif
            emit dataChanged(this->index(0,0),this->index(rowCount()-1,0));
            return true;
        }
        else {
            return false;
        }
    }
    else {
        return false;
    }
}

bool CollageImageModel::collageFull()
{
    if(rowCount() == countImagePathSet())
        return true;
    else
        return false;
}

QSize CollageImageModel::collagePixelSize() const
{
    return mPixelSize;
}

bool CollageImageModel::nextImageIsEffectSelectable()
{
    if(!collageFull())
    {
        int i = 0;
        for(;i < rowCount(); i++)
        {
            if(mImages[i]->imagePath() == QUrl(""))
                break;
        }
        return mImages[i]->effectSelectable();
    }
    return false;
}

QString CollageImageModel::nextImageEffectPreset()
{
    if(!collageFull())
    {
        int i = 0;
        for(;i < rowCount(); i++)
        {
            if(mImages[i]->imagePath() == QUrl(""))
                break;
        }
        return mImages[i]->effectPreset();
    }
    return "";
}

int CollageImageModel::countImagePathSet() const
{
    int currentCount = 0;
    for(int i = 0; i < rowCount(); i++)
    {
        if(mImages[i]->imagePath() != QUrl(""))
        {
            currentCount++;
        }
    }
    return currentCount;
}

CollageImage::CollageImage(QSize collagePixelSize, QObject *parent) : QObject(parent), mEffect(""), mCollagePixelSize(collagePixelSize)
{

}

bool CollageImage::parseXml(const QDomNode &node)
{
    if("image" != node.nodeName())
    {
        mErrorMsg = "wrong node. Expected node name 'image'";
        mLine = node.lineNumber();
        return false;
    }

    QDomElement element = node.toElement();

    QDomNodeList posNode = element.elementsByTagName("position");

    if(posNode.count() != 1)
    {
        if(posNode.length() == 0)
            mErrorMsg = "no position node";
        else
            mErrorMsg = "multiple position nodes";
        mLine = element.lineNumber();
        return false;
    }

    if(posNode.item(0).hasAttributes() && posNode.item(0).toElement().hasAttribute("x") && posNode.item(0).toElement().hasAttribute("y"))
    {
        bool ok;
        QString x = posNode.item(0).toElement().attribute("x");
        mImageRect.setX(x.toDouble(&ok));
        if(!ok)
        {
            mErrorMsg = "position 'x' must be defined as float";
            mLine = posNode.item(0).lineNumber();
            return false;
        }

        QString y = posNode.item(0).toElement().attribute("y");
        mImageRect.setY(y.toDouble(&ok));
        if(!ok)
        {
            mErrorMsg = "position 'y' must be defined as float";
            mLine = posNode.item(0).lineNumber();
            return false;
        }
    }
    else
    {
        mErrorMsg = "position must be defined be 'x' and 'y' attributes";
        mLine = posNode.item(0).lineNumber();
        return false;
    }

    QDomNodeList sizeNode = element.elementsByTagName("size");

    if(sizeNode.count() != 1)
    {
        if(sizeNode.length() == 0)
            mErrorMsg = "no size node";
        else
            mErrorMsg = "multiple size nodes";
        mLine = element.lineNumber();
        return false;
    }

    if(sizeNode.item(0).hasAttributes() && sizeNode.item(0).toElement().hasAttribute("width") && sizeNode.item(0).toElement().hasAttribute("height"))
    {
        bool ok;
        QString x = sizeNode.item(0).toElement().attribute("width");
        mImageRect.setWidth(x.toDouble(&ok));
        if(!ok)
        {
            mErrorMsg = "size 'width' must be defined as float";
            mLine = sizeNode.item(0).lineNumber();
            return false;
        }

        QString y = sizeNode.item(0).toElement().attribute("height");
        mImageRect.setHeight(y.toDouble(&ok));
        if(!ok)
        {
            mErrorMsg = "size 'height' must be defined as float";
            mLine = sizeNode.item(0).lineNumber();
            return false;
        }
    }
    else
    {
        mErrorMsg = "size must be defined be 'width' and 'height' attributes";
        mLine = sizeNode.item(0).lineNumber();
        return false;
    }

    QDomNodeList rotationNode = element.elementsByTagName("rotation");
    if(rotationNode.count() == 1)
    {
        bool ok;
        mAngle = rotationNode.item(0).toElement().text().toFloat(&ok);
        if(!ok)
        {
            mErrorMsg = "rotation must be defined as float";
            mLine = rotationNode.item(0).lineNumber();
            return false;
        }
    }
    else if(rotationNode.count() > 1)
    {
        mErrorMsg = "multiple rotations are defined";
        mLine = element.lineNumber();
        return false;
    }

    QDomNodeList borderImageNode = element.elementsByTagName("border");
    if(borderImageNode.count() == 1)
    {
        QDomNodeList borderFileNode = borderImageNode.item(0).toElement().elementsByTagName("file");
        if(borderFileNode.count() == 1)
        {
            mBorderImage = borderFileNode.item(0).toElement().text();
        }
        else
        {
            mErrorMsg = "one border image file must be defined";
            mLine = borderImageNode.item(0).toElement().lineNumber();
            return false;
        }

        QDomNodeList borderMarginNode = borderImageNode.item(0).toElement().elementsByTagName("margin");
        if(borderMarginNode.count() == 1)
        {
            if(borderMarginNode.item(0).hasAttributes() &&
                    borderMarginNode.item(0).toElement().hasAttribute("top") &&
                    borderMarginNode.item(0).toElement().hasAttribute("left") &&
                    borderMarginNode.item(0).toElement().hasAttribute("right") &&
                    borderMarginNode.item(0).toElement().hasAttribute("bottom"))
            {
                bool ok;
                QString top = borderMarginNode.item(0).toElement().attribute("top");
                mBorderRect.setTop(top.toInt(&ok));
                if(!ok)
                {
                    mErrorMsg = "margin 'top' must be defined as int";
                    mLine = borderMarginNode.item(0).lineNumber();
                    return false;
                }

                QString left = borderMarginNode.item(0).toElement().attribute("left");
                mBorderRect.setLeft(left.toInt(&ok));
                if(!ok)
                {
                    mErrorMsg = "margin 'left' must be defined as int";
                    mLine = borderMarginNode.item(0).lineNumber();
                    return false;
                }

                QString right = borderMarginNode.item(0).toElement().attribute("right");
                mBorderRect.setRight(right.toInt(&ok));
                if(!ok)
                {
                    mErrorMsg = "margin 'right' must be defined as int";
                    mLine = borderMarginNode.item(0).lineNumber();
                    return false;
                }

                QString bottom = borderMarginNode.item(0).toElement().attribute("bottom");
                mBorderRect.setBottom(bottom.toInt(&ok));
                if(!ok)
                {
                    mErrorMsg = "margin 'bottom' must be defined as int";
                    mLine = borderMarginNode.item(0).lineNumber();
                    return false;
                }
            }
            else
            {
                mErrorMsg = "border margin node must have attributes 'top', 'left', 'right' and 'bottom'";
                mLine = borderMarginNode.item(0).toElement().lineNumber();
                return false;
            }
        }
    }
    else if(borderImageNode.count() > 1)
    {
        mErrorMsg = "multiple border images are defined";
        mLine = element.lineNumber();
        return false;
    }

    QDomNodeList effectPreset = element.elementsByTagName("effectPreset");
    if(effectPreset.count() == 1)
    {
        mEffectPreset = effectPreset.item(0).toElement().text();
    }
    else if(effectPreset.count() > 1)
    {
        mErrorMsg = "multiple effect presets are defined";
        mLine = element.lineNumber();
        return false;
    }

    QDomNodeList effectSelectable = element.elementsByTagName("effectSelectable");
    if(effectSelectable.count() == 1)
    {
        mEffectSelectable = effectSelectable.item(0).toElement().text() == "true" ? true : false;
    }
    else if(effectSelectable.count() > 1)
    {
        mErrorMsg = "multiple effect selectable nodes are defined";
        mLine = element.lineNumber();
        return false;
    }

    if(validateBoundary() == false)
    {
        mErrorMsg = "Validation of size constrains failed.";
        mLine = node.toElement().lineNumber();
        return false;
    }

    return true;
}

QUrl CollageImage::imagePath() const
{
    return mImagePath;
}

QString CollageImage::effect() const
{
    return mEffect;
}

QRectF CollageImage::imageRect() const
{
    return mImageRect;
}

float CollageImage::rotation() const
{
    return mAngle;
}


QUrl CollageImage::borderImage() const
{
    return mBorderImage;
}

void CollageImage::setImage(QUrl imagePath)
{
    if(mImagePath != imagePath)
    {
        mImagePath = imagePath;
        emit imagePathChanged(mImagePath);
    }
}

void CollageImage::setEffect(QString effect)
{
    mEffect = effect;
    effectChanged(effect);
}

QRect CollageImage::borderRect() const
{
    return mBorderRect;
}

bool CollageImage::effectSelectable() const
{
    return mEffectSelectable;
}

QString CollageImage::effectPreset() const
{
    return mEffectPreset;
}

bool CollageImage::validateBoundary()
{
    //test position and size parameter
    bool result = true;

    // scale image rect to pixels
    QRectF imageRect(mImageRect.x()*mCollagePixelSize.width(), mImageRect.y()*mCollagePixelSize.height(), mImageRect.width()*mCollagePixelSize.width(), mImageRect.height()*mCollagePixelSize.height());
    // get center of rect
    QPointF center = imageRect.center();
    // rotate around center
    QTransform transform;
    transform.translate(center.x(), center.y());
    transform.rotate(mAngle);
    transform.translate(-center.x(), -center.y());
    imageRect = transform.mapRect(imageRect);
    qDebug() << "Bounding box of image: " << imageRect;

    // now check if image will fit into collage
    if(imageRect.x() < -std::numeric_limits<qreal>::epsilon()|| imageRect.x() > mCollagePixelSize.width())
    {
        result = false;
    }
    if(imageRect.y() < -std::numeric_limits<qreal>::epsilon() || imageRect.y() > mCollagePixelSize.height())
    {
        result = false;
    }
    if((imageRect.x() + imageRect.width()) > (mCollagePixelSize.width() + std::numeric_limits<qreal>::epsilon() )|| imageRect.width() < 0)
    {
        result = false;
    }
    if((imageRect.y() + imageRect.height()) > (mCollagePixelSize.height() + std::numeric_limits<qreal>::epsilon()) || imageRect.height() < 0)
    {
        result = false;
    }

    //@TODO: Validate Border

    return result;
}
