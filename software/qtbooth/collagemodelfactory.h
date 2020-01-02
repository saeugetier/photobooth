#ifndef COLLAGEMODELFACTORY_H
#define COLLAGEMODELFACTORY_H

#include <QObject>
#include <QUrl>
#include <exception>
#include <QMap>

#include "collageiconmodel.h"
#include "collageimagemodel.h"
#include "modelparser.h"

class CollageModelFactory : public QObject, public ModelParser
{
    Q_OBJECT

    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString errorMsg READ errorLineMsg NOTIFY errorMsgChanged)
    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged)

public:
    explicit CollageModelFactory(QObject *parent = nullptr);

    enum Status { Null, Ready, Loading, Error };
    Q_ENUM(Status)
    Status status() const;

    QUrl source() const;
    virtual void setSource(const QUrl &url);

    QString errorLineMsg() const;

    Q_INVOKABLE CollageIconModel* getCollageIconModel();
    Q_INVOKABLE CollageImageModel* getCollageImageModel(const QString& name);
signals:
    void sourceChanged(const QUrl &);
    void statusChanged(const Status &);
    void errorMsgChanged(const QString &);

protected:
    void loadModels();
    bool parseXml(const QDomNode &node) override;
    void clearModels();
    Status mStatus;
    QUrl mSource;
    CollageIconModel mIconModel;
    QMap<QString, CollageImageModel*> mCollageModels;
    CollageImageModel mEmptyCollageModel;
};

#endif // COLLAGEMODELFACTORY_H
