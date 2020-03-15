#ifndef FILEIO_H
#define FILEIO_H

#include <QObject>
#include <QUrl>

class FileIO: public QObject
{
    Q_OBJECT

    Q_PROPERTY(QUrl source READ source WRITE setSource)
public:
    Q_INVOKABLE QByteArray read();
    Q_INVOKABLE bool write(const QString &data);
    Q_INVOKABLE bool remove();
    Q_INVOKABLE bool move(const QUrl &filename);
    QUrl source() { return mSource; };

public slots:
    void setSource(const QUrl& source) { mSource = source; };
private:
    QUrl mSource;
};

#endif // FILEIO_H
