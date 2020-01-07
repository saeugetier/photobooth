#include "collageimagemodel.h"
#include "collagemodelfactory.h"

CollageImageModel::CollageImageModel(QObject *parent) : QAbstractListModel(parent)
{

}

CollageImageModel::~CollageImageModel()
{
    for(int i = 0; i < mImages.length(); i++)
    {
        delete mImages.at(i);
    }
    mImages.clear();

    QAbstractListModel::~QAbstractListModel();
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
        CollageImage* image = new CollageImage();
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
    return mImages.count();
}

QVariant CollageImageModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() > mImages.count())
        return QVariant();

    const CollageImage &image = mImages[index.row()];
    if (role == ImagePathRole)
        return image.imagePath();
    else if (role == PositionRole)
        return image.position();
    else if(role == SizeRole)
        return image.size();
    else if(role == BoarderImageRole)
        return image.borderImage();
    return QVariant();
}

QHash<int, QByteArray> CollageImageModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[PositionRole] = "position";
    roles[SizeRole] = "size";
    roles[ImagePathRole] = "imagePath";
    roles[BoarderImageRole] = "borderImage";
    return roles;
}

QUrl CollageImageModel::backgroundImage() const
{
    return mBackgroundImage;
}

bool CollageImageModel::addImagePath(QUrl source)
{
    if(rowCount() > countImagePathSet()) {
        mImages[countImagePathSet()]->setImage(source);
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
}

bool CollageImageModel::clearImagePath(int index)
{
    if(index < rowCount())
    {
        if(mImages[index]->imagePath() != QUrl(""))
        {
            mImages[index]->setImage(QUrl(""));

            for(int i = 0; i < rowCount() - 1; i++)
            {
                if(mImages[i]->imagePath() == QUrl(""))
                {
                    for(int j = i + 1; i < rowCount(); j++)
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

int CollageImageModel::countImagePathSet() const
{
    int i = 0;

    for(;i < rowCount(); i++)
    {
        if(mImages[i]->imagePath() == QUrl(""))
            break;
    }
    return i;
}

CollageImage::CollageImage(QObject *parent) : QObject(parent)
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
        mPosition.setX(x.toDouble(&ok));
        if(!ok)
        {
            mErrorMsg = "position 'x' must be defined as float";
            mLine = posNode.item(0).lineNumber();
            return false;
        }

        QString y = posNode.item(0).toElement().attribute("y");
        mPosition.setY(y.toDouble(&ok));
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
        mSize.setWidth(x.toDouble(&ok));
        if(!ok)
        {
            mErrorMsg = "size 'width' must be defined as float";
            mLine = sizeNode.item(0).lineNumber();
            return false;
        }

        QString y = sizeNode.item(0).toElement().attribute("height");
        mSize.setHeight(y.toDouble(&ok));
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

    QDomNodeList borderImageNode = element.elementsByTagName("border");
    if(borderImageNode.count() == 1)
    {
        mBorderImage = borderImageNode.item(0).toElement().text();
    }
    else if(borderImageNode.count() > 1)
    {
        mErrorMsg = "multiple border images are defined";
        mLine = element.lineNumber();
        return false;
    }

    return true;
}

QUrl CollageImage::imagePath() const
{
    return mImagePath;
}

QPointF CollageImage::position() const
{
    return mPosition;
}

float CollageImage::rotation() const
{
    return mAngle;
}

QSizeF CollageImage::size() const
{
    return mSize;
}

QUrl CollageImage::borderImage() const
{
    return mBorderImage;
}

void CollageImage::setImage(QUrl imagePath)
{
    mImagePath = imagePath;
}

bool CollageImage::validateBoundary()
{
    return true;
}
