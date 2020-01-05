#ifndef COLLAGEIMAGEMODEL_H
#define COLLAGEIMAGEMODEL_H

#include <QAbstractListModel>
#include <QUrl>
#include <QSizeF>
#include <QPointF>
#include <QDomNode>
#include "modelparser.h"

class CollageImage : public QObject, public ModelParser
{
    Q_OBJECT
    Q_PROPERTY(QUrl imagePath READ imagePath NOTIFY imagePathChanged)
    Q_PROPERTY(QPointF position READ position NOTIFY positionChanged)
    Q_PROPERTY(float rotation READ rotation NOTIFY rotationChanged)
    Q_PROPERTY(QSizeF size READ size NOTIFY sizeChanged)
    Q_PROPERTY(QUrl borderImage READ borderImage NOTIFY borderImageChanged)
public:
    CollageImage(QObject* parent = nullptr);
    bool parseXml(const QDomNode& node) override;
    QUrl imagePath() const;
    QPointF position() const;
    float rotation() const;
    QSizeF size() const;
    QUrl borderImage() const;
    void setImage(QUrl imagePath);
signals:
    void imagePathChanged(QUrl url);
    void positionChanged(QPointF pos);
    void rotationChanged(float angle);
    void sizeChanged(QSizeF size);
    void borderImageChanged(QUrl url);
protected:
    bool validateBoundary();
    QUrl mImagePath;
    QSizeF mSize;
    QPointF mPosition;
    float mAngle;
    QUrl mBorderImage;
};

class CollageImageModel : public QAbstractListModel, public ModelParser
{
    Q_OBJECT
    Q_PROPERTY(QUrl backgroundImage READ backgroundImage NOTIFY backgroundImageChanged)
public:
    enum ImageRoles {
        ImagePathRole = Qt::UserRole + 1,
        PositionRole,
        SizeRole,
        BoarderImageRole
    };

    explicit CollageImageModel(QObject *parent = nullptr);
    virtual ~CollageImageModel();
    bool parseXml(const QDomNode& node) override;
    int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    QUrl backgroundImage() const;
signals:
    void backgroundImageChanged(const QUrl &image);
protected:
    QList<CollageImage*> mImages;
    QUrl mBackgroundImage;
};

#endif // COLLAGEIMAGEMODEL_H
