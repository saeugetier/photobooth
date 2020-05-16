#ifndef COLLAGEIMAGEMODEL_H
#define COLLAGEIMAGEMODEL_H

#include <QAbstractListModel>
#include <QUrl>
#include <QSizeF>
#include <QPointF>
#include <QRectF>
#include <QDomNode>
#include "modelparser.h"

class CollageImage : public QObject, public ModelParser
{
    Q_OBJECT
    Q_PROPERTY(QUrl imagePath READ imagePath NOTIFY imagePathChanged)
    Q_PROPERTY(QRectF imageRect READ imageRect CONSTANT)
    Q_PROPERTY(float rotation READ rotation CONSTANT)
    Q_PROPERTY(QUrl borderImage READ borderImage CONSTANT)
    Q_PROPERTY(QRect borderRect READ borderRect CONSTANT)
    Q_PROPERTY(bool effectSelectable READ effectSelectable CONSTANT)
    Q_PROPERTY(QString effectPreset READ effectPreset CONSTANT)
    Q_PROPERTY(QString effect READ effect NOTIFY effectChanged)
public:
    CollageImage(QObject* parent = nullptr);
    bool parseXml(const QDomNode& node) override;
    QUrl imagePath() const;
    QRectF imageRect() const;
    float rotation() const;
    QUrl borderImage() const;
    QRect borderRect() const;
    bool effectSelectable() const;
    QString effectPreset() const;
    QString effect() const;

    void setImage(QUrl imagePath);
    void setEffect(QString effect);
signals:
    void imagePathChanged(QUrl url);
    void effectChanged(QString effect);
protected:
    bool validateBoundary();
    QUrl mImagePath;
    QRectF mImageRect;
    float mAngle = 0.0f;
    QUrl mBorderImage;
    QRect mBorderRect;
    bool mEffectSelectable = true;
    QString mEffectPreset = "";
    QString mEffect = "";
};

class CollageImageModel : public QAbstractListModel, public ModelParser
{
    Q_OBJECT
    Q_PROPERTY(QUrl backgroundImage READ backgroundImage NOTIFY backgroundImageChanged)
    Q_PROPERTY(int countImagePathSet READ countImagePathSet NOTIFY countImagePatchSetChanged)
    Q_PROPERTY(bool collageFull READ collageFull NOTIFY collageFullChanged)
public:
    enum ImageRoles {
        ImagePathRole = Qt::UserRole + 1,
        ImageRectRole,
        RotationRole,
        BorderImageRole,
        BorderRectRole,
        EffectSelectableRole,
        EffectRole
    };

    explicit CollageImageModel(QObject *parent = nullptr);
    virtual ~CollageImageModel() override;
    bool parseXml(const QDomNode& node) override;
    int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    QUrl backgroundImage() const;
    Q_INVOKABLE bool addImagePath(QUrl source, QString effect = "");
    Q_INVOKABLE void clearImagePathes();
    Q_INVOKABLE bool clearImagePath(int index);
    Q_INVOKABLE bool collageFull();
    Q_INVOKABLE bool nextImageIsEffectSelectable();
    Q_INVOKABLE QString nextImageEffectPreset();
    int countImagePathSet() const;
signals:
    void backgroundImageChanged(const QUrl &image);
    void countImagePatchSetChanged(const int &count);
    void collageFullChanged(bool full);
protected:
    QList<CollageImage*> mImages;
    QUrl mBackgroundImage;
};

#endif // COLLAGEIMAGEMODEL_H
