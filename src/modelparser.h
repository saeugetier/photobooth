#ifndef MODELPARSER_H
#define MODELPARSER_H

#include <QString>
#include <QDomNode>

class ModelParser
{
public:
    ModelParser();
    QString errorMsg() const;
    int lineNumber();
    virtual bool parseXml(const QDomNode& node) = 0;
protected:
    QString mErrorMsg;
    int mLine;
};

#endif // MODELPARSER_H
