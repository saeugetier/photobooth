#ifndef PHOTOMONTAGE_H
#define PHOTOMONTAGE_H

#include <QObject>
#include <QQmlListProperty>
#include <QFutureWatcher>
#include <QImage>

class CollageImageDrawer : public QImage
{
public:
    CollageImageDrawer();
    void paintImage(const QImage &image, QRectF position);
};

class PhotoMontage : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList filenames READ filenames NOTIFY filenamesChanged)
public:
    explicit PhotoMontage(QObject *parent = nullptr);

    QStringList filenames();

    Q_INVOKABLE bool generate();
    Q_INVOKABLE int addFile(QString file);
    Q_INVOKABLE void clearFiles();
    Q_INVOKABLE void saveResult(QString filename);

    static const int COUNT_PHOTOS = 4;
signals:
    void montageReady();
    void filenamesChanged(const QStringList &list);
public slots:


protected:
    QStringList m_filenames;
    QFutureWatcher<CollageImageDrawer> m_future;
};

#endif // PHOTOMONTAGE_H
