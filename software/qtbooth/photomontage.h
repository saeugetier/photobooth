#ifndef PHOTOMONTAGE_H
#define PHOTOMONTAGE_H

#include <QObject>
#include <QQmlListProperty>
#include <QFutureWatcher>

class PhotoMontage : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<QString> filenames READ filenames)
public:
    explicit PhotoMontage(QObject *parent = nullptr);

    QQmlListProperty<QString> filenames();

    Q_INVOKABLE bool generate(QString outputFilename);
signals:
    void montageReady(QString filename);
public slots:

protected slots:
    void processed();

protected:
    static void append_filename(QQmlListProperty<QString> *list, QString* filename);
    static void clear_filenames(QQmlListProperty<QString> *list);
    static int count_filenames(QQmlListProperty<QString> *list);

    QStringList m_filenames;
    QFutureWatcher<QString> m_future;
};

#endif // PHOTOMONTAGE_H
