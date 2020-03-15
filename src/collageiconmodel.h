#ifndef COLLAGEICONMODEL_H
#define COLLAGEICONMODEL_H

#include <QUrl>
#include <QDomNode>
#include <QAbstractListModel>
#include "modelparser.h"

class CollageIcon : public ModelParser
{
public:
    CollageIcon();
    virtual ~CollageIcon();

    bool parseXml(const QDomNode& node) override;

    QString name() const;
    QUrl icon() const;
    bool printable() const;
protected:
    QString mName;
    QUrl mIcon;
    bool mPrintable;
};

class CollageIconModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY rowCountChanged)
    Q_PROPERTY(bool showPrintable READ showPrintable WRITE setShowPrintable NOTIFY showPrintableChanged)
public:
    enum IconRoles {
       NameRole = Qt::UserRole + 1,
       IconRole,
       PrintableRole
    };

    CollageIconModel(QObject *parent = 0);
    void addIcon(const CollageIcon& icon);
    int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    void clear();
    Q_INVOKABLE QString getIconName(int index);
    bool showPrintable() const;
    void setShowPrintable(bool printable);
signals:
    void rowCountChanged(int count);
    void showPrintableChanged(bool printable);
private:
    QList<CollageIcon> mIcons;
    bool mShowPrintable;
};

#endif // COLLAGEICON_H
