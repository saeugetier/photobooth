#include "modelparser.h"

ModelParser::ModelParser()
{
    mErrorMsg = "no error";
    mLine = -1;
}

int ModelParser::lineNumber()
{
    return mLine;
}

QString ModelParser::errorMsg() const
{
    QString result(mErrorMsg);
    result.append(" - at line: ");
    result.append(QString::number(mLine));
    return mErrorMsg;
}
