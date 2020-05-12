#include "collageiconmodel.h"
#include "collagemodelfactory.h"

CollageIcon::CollageIcon()
{
    mPrintable = true;
}

CollageIcon::~CollageIcon()
{

}

bool CollageIcon::parseXml(const QDomNode& node)
{
    QDomElement element = node.toElement();

    if(element.elementsByTagName("name").length() == 0)
    {
        mErrorMsg = "no 'name' element";
        mLine = element.lineNumber();
        return false;
    }

    QDomNode nameNode = element.elementsByTagName("name").item(0);
    mName = nameNode.toElement().text();

    if(element.elementsByTagName("icon").length() == 0)
    {
        mErrorMsg = "no 'icon' element";
        mLine = element.lineNumber();
        return false;
    }

    QDomNode iconNode = element.elementsByTagName("icon").item(0);
    mIcon = iconNode.toElement().text();

    if(element.elementsByTagName("printable").length() != 0)
    {
        QDomNode printableNode = element.elementsByTagName("printable").item(0);
        mPrintable = (iconNode.toElement().text() == "true");
    }

    return true;
}

QString CollageIcon::name() const
{
    return mName;
}

QUrl CollageIcon::icon() const
{
    return mIcon;
}

bool CollageIcon::printable() const
{
    return mPrintable;
}

CollageIconModel::CollageIconModel(QObject *parent) : QAbstractListModel(parent), mShowPrintable(true)
{
}

void CollageIconModel::addIcon(const CollageIcon &icon)
{
    mIcons.append(icon);
}

int CollageIconModel::rowCount(const QModelIndex &parent) const
{
    (void)(parent);
    if(mShowPrintable)
    {
        return mIcons.count();
    }
    else
    {
        int count = 0;
        for(const auto icon : mIcons)
        {
            if(!icon.printable()) count++;
        }
        return count;
    }
}

QVariant CollageIconModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= rowCount(index))
        return QVariant();

    int rowNumber = 0;

    if(!mShowPrintable)
    {
        for(int i = index.row(); ; rowNumber++)
        {
            if(!mIcons[rowNumber].printable() )
            {
                if(i == 0)
                {
                    break;
                }
                i--;
            }
        }
    }
    else
    {
        rowNumber = index.row();
    }

    const CollageIcon &icon = mIcons[rowNumber];
    if (role == NameRole)
        return icon.name();
    else if (role == IconRole)
        return icon.icon();
    return QVariant();
}

QHash<int, QByteArray> CollageIconModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[IconRole] = "icon";
    return roles;
}

void CollageIconModel::clear()
{
    mIcons.clear();
}

QString CollageIconModel::getIconName(int index)
{
     if (index < 0 || index >= mIcons.count())
         return QString();

     const CollageIcon &icon = mIcons[index];
     return icon.name();
}

bool CollageIconModel::showPrintable() const
{
    return mShowPrintable;
}

void CollageIconModel::setShowPrintable(bool printable)
{
    if(printable == mShowPrintable)
        return;
    mShowPrintable = printable;
    rowCountChanged(rowCount());
}
