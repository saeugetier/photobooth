#include "photomontage.h"
#include <list>
#include <functional>
#include <QtConcurrent/QtConcurrent>
#include <QtConcurrent/QtConcurrentMap>
#include <QtConcurrent/QtConcurrentFilter>
#include <QDebug>
#include <QPainter>
#include <QImage>
#include <QPair>

#include "myhelper.h"

typedef QPair<int,QString> indexedString;
typedef QPair<int, QImage> indexedImage;

CollageImage::CollageImage() : QImage(MyHelper::instance()->getPrintSize(), QImage::Format_RGB888)
{
    this->fill(Qt::GlobalColor::white);
}

void CollageImage::paintImage(const QImage &image, QRectF position)
{
    qDebug() << "Placing image at position " << position << " Data size " << image.byteCount();

    QPainter painter(this);

    QRect pos;

    pos.setTop(position.top() * this->size().height());
    pos.setLeft(position.left() * this->size().width());
    pos.setBottom(position.bottom() * this->size().height());
    pos.setRight(position.right() * this->size().width());

    painter.drawImage(pos, image);
}

class ImageLoader
{
public:
    typedef indexedImage result_type;

    indexedImage operator()(const indexedString &filename)
    {
        indexedImage result;
        result.first = filename.first;
        result.second.load(filename.second.right(filename.second.length() - QString("file://").length()));

        while(result.second.width() > 1800 || result.second.height() > 1800)  //reduce memory
            result.second = result.second.scaled(QSize(result.second.width() / 2, result.second.height() / 2));

        return result;
    }
};

class CollageReduce
{
public:
    CollageReduce(int elements) : m_cElements(elements), m_cColumns(ceil(sqrt(elements)))
    {

    }

    void operator()(CollageImage &collage, const indexedImage &source)
    {
        //calculate row and column
        int index = source.first;
        int column = index % m_cColumns;
        int row = index / m_cColumns;

        //calculate margins
        float column_size = (1.0f / m_cColumns) - (2 * m_cBorderMargin * (m_cColumns -  1));
        float column_pos = (1.0f / m_cColumns) * column + m_cBorderMargin;
        float row_pos = (1.0f / m_cColumns) * row + m_cBorderMargin;

        QRectF pos(column_pos, row_pos, /*column_pos +*/ column_size, /*row_pos +*/ column_size);
        collage.paintImage(source.second, pos);
    }
protected:
    const int m_cElements;
    const float m_cBorderMargin = 0.02f;
    const int m_cColumns;
};


PhotoMontage::PhotoMontage(QObject *parent) : QObject(parent), m_future(this)//, mMontageImage(NULL)
{
    connect(&m_future, &QFutureWatcher<QString>::finished, this, &PhotoMontage::montageReady);
}


QStringList PhotoMontage::filenames()
{
    return m_filenames;
}

int PhotoMontage::addFile(QString file)
{
   m_filenames.append(file);
   return m_filenames.count();
}

void PhotoMontage::clearFiles()
{
    m_filenames.clear();
}

bool PhotoMontage::generate()
{
    qDebug() << "Generating collage";
    qDebug() << "Input files used: " << m_filenames;

    QList<QPair<int, QString>> file_list;

    for(int i= 0; i < m_filenames.count(); i++ )
    {
        file_list.append(QPair<int, QString>(i, m_filenames[i]));
    }

    QFuture<CollageImage> future
            = QtConcurrent::mappedReduced<CollageImage>(file_list,
                                                        ImageLoader(),
                                                        CollageReduce(file_list.size()),
                                                        QtConcurrent::ReduceOptions(QtConcurrent::OrderedReduce | QtConcurrent::SequentialReduce));

    m_future.setFuture(future);

    return true;
}

void PhotoMontage::saveResult(QString filename)
{
    if(m_future.result().height() != 0)
        m_future.result().save(filename.right(filename.length() - QString("file://").length()));
}
