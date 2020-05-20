#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <QObject>
#include <memory>
#include <QtConcurrent/QtConcurrent>

class FileSystem : public QObject
{
    Q_OBJECT
public:
    explicit FileSystem(QObject *parent = nullptr);
    Q_INVOKABLE QUrl findFile(QString filename, QList<QUrl> searchPaths, bool searchInResource = true);
    Q_INVOKABLE QString getImagePath();
    Q_INVOKABLE bool removableDriveMounted();
    Q_INVOKABLE void unmountRemoveableDrive();
    Q_INVOKABLE void startCopyFilesToRemovableDrive();
    Q_INVOKABLE void abortCopy();
    Q_INVOKABLE void deleteAllImages();
    Q_INVOKABLE bool layoutFilesOnRemovableDrive();
    Q_INVOKABLE void copyLayoutFiles();
    Q_INVOKABLE QSize getImageSize(QString filename);
protected:
    QString getRemovableDrivePath();
    QFuture<void> m_copyFuture;
signals:
    void copyProgress(int progress);
    void copyFinished();
};

#endif // FILESYSTEM_H
