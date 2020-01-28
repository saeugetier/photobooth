#include "collagemodelfactory.h"
#include <QFile>
#include <QDomDocument>

CollageModelFactory::CollageModelFactory(QObject *parent) : QObject(parent), mStatus(Null)
{

}

CollageModelFactory::Status CollageModelFactory::status() const
{
    return mStatus;
}

QUrl CollageModelFactory::source() const
{
    return mSource;
}

void CollageModelFactory::setSource(const QUrl &url)
{
    if(mSource == url)
        return;

    mSource = url;
    emit sourceChanged(url);
    mStatus = Loading;
    statusChanged(mStatus);
    loadModels();
}

QString CollageModelFactory::errorLineMsg() const
{
    QString temp = mErrorMsg;
    temp.append(" - at line: ");
    temp.append(QString::number(mLine));
    return temp;
}

CollageIconModel *CollageModelFactory::getCollageIconModel()
{
    return &mIconModel;
}

CollageImageModel *CollageModelFactory::getCollageImageModel(const QString &name)
{
    auto iter = mCollageModels.find(name);
    if(iter != mCollageModels.end())
    {
        CollageImageModel* model = *iter;
        model->clearImagePathes();
        return model;
    }
    else
        return &mEmptyCollageModel;
}

void CollageModelFactory::loadModels()
{
    QString fileName;
    if(mSource.isLocalFile())
        fileName = mSource.toLocalFile();
    else if(mSource.toString().contains("qrc:"))
    {
        fileName = mSource.toString().remove(0,3);
    }
    else
    {
        fileName = mSource.toString();
    }

    QFile file(fileName);

    if(file.open(QFile::ReadOnly))
    {
        QString errorMsg;
        int line;

        QDomDocument doc;
        if(!doc.setContent(&file, true, &errorMsg, &line))
        {
            mErrorMsg = errorMsg;
            mLine = line;
            mStatus = Error;
            errorMsgChanged(errorLineMsg());
            statusChanged(mStatus);
        }
        else
        {
            QDomElement rootElement = doc.documentElement();
            if(!parseXml(rootElement))
            {
                clearModels();
                mStatus = Error;
                errorMsgChanged(errorLineMsg());
                statusChanged(mStatus);
            }else
            {
                mStatus = Ready;
                statusChanged(mStatus);
            }
        }
    }
    else
    {
        mErrorMsg = "Could not read file";
        mStatus = Error;
        errorMsgChanged(errorLineMsg());
        statusChanged(mStatus);
    }

    file.close();
}

bool CollageModelFactory::parseXml(const QDomNode &node)
{
    QDomElement rootElement = node.toElement();

    if(rootElement.tagName() != "catalog")
    {
       mErrorMsg = "Excepted 'catalog' root node";
       mLine = rootElement.lineNumber();
       return false;
    }

    if(!rootElement.hasAttribute("version") || rootElement.attribute("version") != "1.0")
    {
        mErrorMsg = "Wrong version information";
        mLine = rootElement.lineNumber();
        return false;
    }

    QDomNodeList collageNodes = rootElement.elementsByTagName("collage");
    if(collageNodes.length() < 1)
    {
        mErrorMsg = "No collage nodes defined";
        mLine = rootElement.lineNumber();
        return false;
    }

    for(int i = 0; i < collageNodes.length(); i++)
    {
        bool result;
        CollageIcon icon;
        result = icon.parseXml(collageNodes.item(i));

        if(!result)
        {
            mErrorMsg = icon.errorMsg();
            mLine = icon.lineNumber();
            return false;
        }

        mIconModel.addIcon(icon);

        CollageImageModel* imageModel = new CollageImageModel();
        result = imageModel->parseXml(collageNodes.item(i));

        if(!result)
        {
            mErrorMsg = imageModel->errorMsg();
            mLine = imageModel->lineNumber();
            return false;
        }

        mCollageModels[icon.name()] = imageModel;
    }

    return true;
}

void CollageModelFactory::clearModels()
{
    mIconModel.clear();

    for(auto iter = mCollageModels.begin(); iter != mCollageModels.end(); iter++)
    {
        delete *iter;
    }
    mCollageModels.clear();
}
