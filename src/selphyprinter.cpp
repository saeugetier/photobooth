#include "selphyprinter.h"
#include <QElapsedTimer>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QImageReader>
#include <QImageWriter>
#include <QRegularExpression>
#include <QSet>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QUrl>
#include <QDebug>

SelphyPrinter::SelphyPrinter(const QString &name, QObject *parent)
    : AbstractPrinter(parent), mIp(""), mRemainingCopies(0)
{
    QRegularExpression regex("[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}");
    QRegularExpressionMatch match = regex.match(name);
    if(match.hasMatch())
    {
        mIp = match.capturedTexts().first();
    }

    QObject::connect(&mPrinterProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(finished(int,QProcess::ExitStatus)));
    QObject::connect(&mPrinterProcess, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SIGNAL(failed()));
}

bool SelphyPrinter::busy()
{
    return mPrinterProcess.state() != QProcess::NotRunning;
}

bool SelphyPrinter::printerOnline()
{
    if(mIp.length() == 0)
        return false;

    // Probe common printer/web ports advertised via mDNS (IPP/IPPS/HTTP).
    const QList<quint16> probePorts = {631, 443, 80};
    for(quint16 port : probePorts)
    {
        QTcpSocket socket;
        socket.connectToHost(mIp, port);
        if(socket.waitForConnected(1200))
        {
            socket.disconnectFromHost();
            qDebug() << "SelphyPrinter on IP" << mIp << "is online (port" << port << ").";
            return true;
        }
    }

    qDebug() << "SelphyPrinter on IP" << mIp << "seems to be offline (no printer port reachable).";
    return false;
}

int SelphyPrinter::printImage(const QString &filename, int copyCount)
{
    if(copyCount <= 0)
        return -1;

    if(mIp.length() > 0)
    {
        QString inputPath = filename;
        const QUrl inputUrl(filename);
        if(inputUrl.isValid() && inputUrl.isLocalFile())
            inputPath = inputUrl.toLocalFile();

        QString printFilename;
        if(inputPath.endsWith(".jpg", Qt::CaseInsensitive)
                || inputPath.endsWith(".jpeg", Qt::CaseInsensitive))
        {
            printFilename = inputPath;
        }
        else
        {
            // Convert source image to JPEG in-process to avoid external dependencies.
            printFilename = inputPath + ".jpg";
            QImageReader reader(inputPath);
            const QImage image = reader.read();
            if(image.isNull())
            {
                qDebug() << "Could not read image for Selphy print:" << inputPath << reader.errorString();
                return -1;
            }

            QImageWriter writer(printFilename, "jpg");
            writer.setQuality(100);
            if(!writer.write(image))
            {
                qDebug() << "Could not convert image to JPG for Selphy print:" << printFilename << writer.errorString();
                return -1;
            }
        }

        if(mPrinterProcess.state() == QProcess::NotRunning)
        {
            if(printerOnline())
            {
                mCurrentPrintFilename = printFilename;
                mRemainingCopies = copyCount;

                emit busyChanged(true);
                QStringList selphyParameters;
                selphyParameters << "-printer_ip=" + mIp << mCurrentPrintFilename;

                mPrinterProcess.start("selphy", selphyParameters);
                qDebug() << "Running selphy with parameters:" << selphyParameters;
                return 0;
            }
            else
                return -1;
        }
        else
            return -1;
    }
    else
        qDebug() << "Print failed. No Selphy Printer connected!";

    return -1;
}

void SelphyPrinter::finished(int code, QProcess::ExitStatus status)
{
    if(code != 0)
    {
        mRemainingCopies = 0;
        emit busyChanged(false);
        qDebug() << "Selphy Error: \n" << mPrinterProcess.readAllStandardError();
        qDebug() << "Code: " << code << " - Status: " << status;
        emit failed();
    }
    else
    {
        mRemainingCopies--;
        if(mRemainingCopies > 0)
        {
            QStringList selphyParameters;
            selphyParameters << "-printer_ip=" + mIp << mCurrentPrintFilename;
            mPrinterProcess.start("selphy", selphyParameters);
            qDebug() << "Running selphy with parameters:" << selphyParameters;
            return;
        }

        emit busyChanged(false);
        emit success();
    }
}

QStringList SelphyPrinter::getAvailablePrintersInternal()
{
    QStringList list;

    QStringList mdnsServiceTypes;
    mdnsServiceTypes << "_canon-cpp-disc._udp"
                     << "_ipps._tcp"
                     << "_ipp._tcp"
                     << "_printer._tcp"
                     << "_http._tcp";

    QSet<QString> discoveredIps;
    const auto encodeDnsName = [](const QString &name) {
        QByteArray encoded;
        const QStringList labels = name.split('.', Qt::SkipEmptyParts);
        for(const QString &label : labels)
        {
            const QByteArray asciiLabel = label.toUtf8();
            encoded.append(static_cast<char>(asciiLabel.size()));
            encoded.append(asciiLabel);
        }
        encoded.append('\0');
        return encoded;
    };

    const auto buildPtrQuery = [&](const QString &serviceType) {
        QByteArray query;
        query.reserve(64);

        // DNS header: id=0, flags=0 (query), QDCOUNT=1
        query.append('\0'); query.append('\0');
        query.append('\0'); query.append('\0');
        query.append('\0'); query.append('\1');
        query.append('\0'); query.append('\0');
        query.append('\0'); query.append('\0');
        query.append('\0'); query.append('\0');

        query.append(encodeDnsName(serviceType + ".local"));

        // QTYPE=PTR (12), QCLASS=IN (1)
        query.append('\0'); query.append('\x0c');
        query.append('\0'); query.append('\x01');

        return query;
    };

    const auto readU16 = [](const QByteArray &packet, int offset, quint16 &value) {
        if(offset + 1 >= packet.size())
            return false;

        value = (static_cast<quint16>(static_cast<quint8>(packet.at(offset))) << 8)
                | static_cast<quint16>(static_cast<quint8>(packet.at(offset + 1)));
        return true;
    };

    const auto readU32 = [](const QByteArray &packet, int offset, quint32 &value) {
        if(offset + 3 >= packet.size())
            return false;

        value = (static_cast<quint32>(static_cast<quint8>(packet.at(offset))) << 24)
                | (static_cast<quint32>(static_cast<quint8>(packet.at(offset + 1))) << 16)
                | (static_cast<quint32>(static_cast<quint8>(packet.at(offset + 2))) << 8)
                | static_cast<quint32>(static_cast<quint8>(packet.at(offset + 3)));
        return true;
    };

    const auto decodeDnsName = [](const QByteArray &packet, int &offset, QString &name) {
        QStringList labels;
        int pos = offset;
        bool jumped = false;
        int jumpCount = 0;
        bool terminated = false;

        while(pos < packet.size())
        {
            const quint8 len = static_cast<quint8>(packet.at(pos));
            if(len == 0)
            {
                if(!jumped)
                    offset = pos + 1;
                terminated = true;
                break;
            }

            // DNS name compression pointer.
            if((len & 0xc0) == 0xc0)
            {
                if(pos + 1 >= packet.size())
                    return false;

                const quint8 next = static_cast<quint8>(packet.at(pos + 1));
                const int pointer = ((len & 0x3f) << 8) | next;
                if(pointer < 0 || pointer >= packet.size())
                    return false;

                if(!jumped)
                    offset = pos + 2;
                pos = pointer;
                jumped = true;
                jumpCount++;
                if(jumpCount > 20)
                    return false;
                continue;
            }

            if(len > 63 || pos + 1 + len > packet.size())
                return false;

            labels.append(QString::fromUtf8(packet.mid(pos + 1, len)));
            pos += 1 + len;
        }

        if(!terminated)
            return false;

        name = labels.join('.');
        return true;
    };

    const auto datagramPtrTargetForService = [&](const QByteArray &packet, const QString &serviceFqdn) {
        if(packet.size() < 12)
            return QString();

        quint16 flags = 0;
        quint16 qdcount = 0;
        quint16 ancount = 0;
        quint16 nscount = 0;
        quint16 arcount = 0;
        if(!readU16(packet, 2, flags)
                || !readU16(packet, 4, qdcount)
                || !readU16(packet, 6, ancount)
                || !readU16(packet, 8, nscount)
                || !readU16(packet, 10, arcount))
            return QString();

        // Must be a DNS response.
        if((flags & 0x8000) == 0)
            return QString();

        const QString serviceLower = serviceFqdn.toLower();
        int offset = 12;

        for(int i = 0; i < qdcount; i++)
        {
            QString qname;
            if(!decodeDnsName(packet, offset, qname))
                return QString();

            if(offset + 4 > packet.size())
                return QString();
            offset += 4;
        }

        const int totalRecords = static_cast<int>(ancount) + static_cast<int>(nscount) + static_cast<int>(arcount);
        for(int i = 0; i < totalRecords; i++)
        {
            QString rrName;
            if(!decodeDnsName(packet, offset, rrName))
                return QString();

            quint16 rrType = 0;
            quint16 rrClass = 0;
            quint32 rrTtl = 0;
            quint16 rdLength = 0;
            if(!readU16(packet, offset, rrType)
                    || !readU16(packet, offset + 2, rrClass)
                    || !readU32(packet, offset + 4, rrTtl)
                    || !readU16(packet, offset + 8, rdLength))
                return QString();

            Q_UNUSED(rrClass);
            Q_UNUSED(rrTtl);

            offset += 10;
            if(offset + rdLength > packet.size())
                return QString();

            if(rrType == 12)
            {
                int rdataOffset = offset;
                QString ptrTarget;
                if(!decodeDnsName(packet, rdataOffset, ptrTarget))
                    return QString();

                if(rrName.toLower() == serviceLower)
                    return ptrTarget;
            }

            offset += rdLength;
        }

        return QString();
    };

    QUdpSocket socket;
    const bool bindOk = socket.bind(QHostAddress::AnyIPv4, 5353,
                                    QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);

    if(!bindOk)
    {
        qDebug() << "mDNS socket bind failed.";
        return list;
    }

    QList<QNetworkInterface> mdnsInterfaces;
    const QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    for(const QNetworkInterface &iface : interfaces)
    {
        const auto flags = iface.flags();
        const bool usable = flags.testFlag(QNetworkInterface::IsUp)
                && flags.testFlag(QNetworkInterface::IsRunning)
                && flags.testFlag(QNetworkInterface::CanMulticast)
                && !flags.testFlag(QNetworkInterface::IsLoopBack);
        if(!usable)
            continue;

        if(socket.joinMulticastGroup(QHostAddress("224.0.0.251"), iface))
            mdnsInterfaces.append(iface);
    }

    if(mdnsInterfaces.isEmpty())
        qDebug() << "mDNS multicast join failed on all interfaces; continuing with best-effort discovery.";

    for(const QString &serviceType : mdnsServiceTypes)
    {
        const QByteArray query = buildPtrQuery(serviceType);

        bool sendSucceeded = false;
        if(mdnsInterfaces.isEmpty())
        {
            if(socket.writeDatagram(query, QHostAddress("224.0.0.251"), 5353) >= 0)
                sendSucceeded = true;
        }
        else
        {
            for(const QNetworkInterface &iface : mdnsInterfaces)
            {
                socket.setMulticastInterface(iface);
                if(socket.writeDatagram(query, QHostAddress("224.0.0.251"), 5353) >= 0)
                    sendSucceeded = true;
            }
        }

        if(!sendSucceeded)
        {
            qDebug() << "mDNS query send failed for service" << serviceType;
            continue;
        }

        QElapsedTimer timer;
        timer.start();
        while(timer.elapsed() < 1200)
        {
            if(!socket.waitForReadyRead(200))
                continue;

            while(socket.hasPendingDatagrams())
            {
                QHostAddress sender;
                quint16 senderPort = 0;
                QByteArray datagram;
                datagram.resize(static_cast<int>(socket.pendingDatagramSize()));
                socket.readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

                if(sender.protocol() != QAbstractSocket::IPv4Protocol)
                    continue;

                if(senderPort != 5353)
                    continue;

                const QByteArray upper = datagram.toUpper();
                const QString serviceFqdn = serviceType + ".local";
                const QString ptrTarget = datagramPtrTargetForService(datagram, serviceFqdn);
                const bool hasMatchingPtr = !ptrTarget.isEmpty();
                const bool looksLikeCanon = upper.contains("SELPHY") || upper.contains("CANON");

                if(!hasMatchingPtr)
                    continue;

                if(serviceType != "_canon-cpp-disc._udp" && !looksLikeCanon)
                    continue;

                const QString ip = sender.toString();
                if(ip.isEmpty() || discoveredIps.contains(ip))
                    continue;

                QString displayName;
                const QString lowerTarget = ptrTarget.toLower();
                const QString suffix = "." + serviceFqdn.toLower();
                if(lowerTarget.endsWith(suffix))
                    displayName = ptrTarget.left(ptrTarget.size() - suffix.size());

                if(displayName.isEmpty())
                    displayName = ptrTarget;

                if(displayName.isEmpty())
                    displayName = "Selphy";

                discoveredIps.insert(ip);
                list.append(displayName + " " + ip);
                qDebug() << "Selphy Printer - Found via mDNS:" << serviceType << displayName << ip;
            }
        }
    }

    if(list.isEmpty())
        qDebug() << "No Canon Selphy printer found via mDNS.";

    return list;
}

SelphyPrinter *SelphyPrinter::createInternal(const QString &name)
{
    Q_UNUSED(name);
    return new SelphyPrinter(name);
}
