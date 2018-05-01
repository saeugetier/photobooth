#ifndef PHOTOMONTAGE_H
#define PHOTOMONTAGE_H

#include <QObject>
#include <QQmlListProperty>
#include <QFutureWatcher>

class PhotoMontage : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList filenames READ filenames NOTIFY filenamesChanged)
public:
    explicit PhotoMontage(QObject *parent = nullptr);

    QStringList filenames();

    Q_INVOKABLE bool generate(QString outputFilename);
    Q_INVOKABLE int addFile(QString file);
    Q_INVOKABLE void clearFiles();
signals:
    void montageReady(QString filename);
    void filenamesChanged(const QStringList &list);
public slots:

protected slots:
    void processed();

protected:
    QStringList m_filenames;
    QFutureWatcher<QString> m_future;
};

#endif // PHOTOMONTAGE_H
