#include "nabasm.h"
#include <qdebug.h>
#include <QFile>
#include <QIODevice>
#include <QRegExp>
#include <QStringList>

#include <iostream>

nabAsm::nabAsm(QString fileIn, QString fileOut)
{
    qDebug() << "Launch for " << fileIn ;
    QFile f( fileIn );
    QFile f2( fileOut );

    if( !f.exists() )
    {
        std::cout << "The file does not exist." << std::endl;
    }
    else if( !f.open( QIODevice::ReadOnly ) )
    {
        std::cout << "Failed to open." << fileIn.toStdString() << std::endl;
    }
    else if( !f2.open( QIODevice::WriteOnly ) )
    {
        std::cout << "Failed to open." << fileOut.toStdString() << std::endl;
    }
    else
    {
        QByteArray bootcode = "amber";
        QByteArray globales = "";
        QByteArray code = "";
        QByteArray cod = "";
        QByteArray offsets = "";
        int params = 0;
        int locals = 0;
        int nbrOffsets = 0;

        QTextStream in(&f);
        QString line = in.readLine();
        while (!line.isNull()) {

            QRegExp rx("^0x[0-9A-F]{8}; Const_[0-9A-F]{4}; (.*);(?: (.*))$");
            rx.setMinimal(true);
            rx.setCaseSensitivity(Qt::CaseInsensitive);
            if ((rx.indexIn(line, 0)) != -1) {
                QRegExp rx2("^(.*)\\((.*)\\)$");

                if(rx.cap(1) == "false")
                {
                    globales += QByteArray::fromHex("FFFFFFFF");
                }
                else if(rx.cap(1) == "integer")
                {
                    globales += QByteArray::fromHex(hexOutput(getUInt32(QByteArray::fromHex(hexOutput(rx.cap(2).right(8).toInt(NULL, 16)*2))), 8));
                }
                else if(rx2.indexIn(rx.cap(1), 0) != -1)
                {
                    if(rx2.cap(1) == "string")
                    {
                        QByteArray data = QByteArray::fromPercentEncoding(rx.cap(2).mid(1, rx.cap(2).size()-2).toAscii());
                        globales += QByteArray::fromHex(hexOutput(getUInt32(QByteArray::fromHex(hexOutput(data.size()*4 + 1))))) + data;
                    }
                    else if(rx2.cap(1) == "bytes")
                    {
                        QByteArray data = QByteArray::fromPercentEncoding(rx.cap(2).mid(1, rx.cap(2).size()-2).toAscii());
                        globales += QByteArray::fromHex(hexOutput(getUInt32(QByteArray::fromHex(hexOutput(data.size()+3))))) + data;
                    }
                    else if(rx2.cap(1) == "skip bytes")
                    {
                        QByteArray data = QByteArray::fromPercentEncoding(rx.cap(2).mid(1, rx.cap(2).size()-2).toAscii());
                        globales += QByteArray::fromHex(hexOutput(getUInt32(QByteArray::fromHex(hexOutput(rx2.cap(2).toInt()+3))))) + data;
                    }
                    else if(rx2.cap(1) == "entry")
                    {
                        globales += QByteArray::fromHex(hexOutput(getUInt32(QByteArray::fromHex(hexOutput(rx2.cap(2).toInt()+3)))));
                    }
                    else if(rx2.cap(1) == "ignore entry")
                    {
                        QStringList size = rx2.cap(2).split(", ");
                        QByteArray data = QByteArray::fromPercentEncoding(rx.cap(2).mid(1, rx.cap(2).size()-2).toAscii());
                        globales += QByteArray::fromHex(hexOutput(getUInt32(QByteArray::fromHex(hexOutput(size[0].toInt()+3)))));
                        globales += QByteArray::fromHex(hexOutput(getUInt32(QByteArray::fromHex(hexOutput(size[1].toInt()*4+1)))));
                        globales += data;
                    }
                }
            }
            else
            {
                rx.setPattern("^0x[0-9A-F]{8} : Sub_[0-9a-f]{4} : 0x([0-9A-F]{4})$");
                if ((rx.indexIn(line, 0)) != -1) {
                    if(cod.size() > 0)
                        code += QByteArray::fromHex(QString::number(params, 16).toAscii()) + QByteArray::fromHex(QString::number(locals, 16).toAscii()) + QByteArray::fromHex("00")+cod;
                    cod = "";
                    params = 0;
                    locals = 0;
                    offsets += QByteArray::fromHex(hexOutput(getUInt32(QByteArray::fromHex(hexOutput(rx.cap(1).toInt(NULL, 16))))));
                    nbrOffsets++;
                }
                else
                {
                    rx.setPattern("^\\.(.*): ([0-9A-F]{2})$");
                    if ((rx.indexIn(line, 0)) != -1) {
                        if(rx.cap(1) == "params")
                        {
                            params = rx.cap(2).toInt(NULL, 16);
                        }
                        if(rx.cap(1) == "locals")
                        {
                            locals = rx.cap(2).toInt(NULL, 16);
                        }
                    }
                    else
                    {
                        rx.setPattern("^[0-9A-F]{4}: (.*)[L/]");
                        rx.setMinimal(true);

                        if ((rx.indexIn(line, 0)) != -1) {
                           cod += QByteArray::fromHex(rx.cap(1).trimmed().replace(" ", "").toAscii());
                        }
                    }
                }
            }


            line = in.readLine();
        }

        if(cod.size() > 0)
            code += QByteArray::fromHex(QString::number(params, 16).toAscii()) + QByteArray::fromHex(QString::number(locals, 16).toAscii()) + QByteArray::fromHex("00")+cod;

        globales = QByteArray::fromHex(hexOutput(getUInt32(QByteArray::fromHex(hexOutput(globales.size()+4))))) + globales;
        code = QByteArray::fromHex(hexOutput(getUInt32(QByteArray::fromHex(hexOutput(code.size()))))) + code;
        offsets = QByteArray::fromHex(hexOutput(getUInt16(QByteArray::fromHex(hexOutput(nbrOffsets, 4))), 4)) + offsets;


        bootcode += hexOutput(globales.size() + code.size() + offsets.size(), 8).toLower() + globales + code + offsets + "Mind";
        f.close();
        f2.write(bootcode);

        f2.close();
    }
}

QByteArray nabAsm::hexOutput(unsigned int i)
{
    return hexOutput(i, 8);
}

QByteArray nabAsm::hexOutput(unsigned int i, int l)
{
    QString s;
    QString f = "";
    s.setNum(i, 16);
    if(l - s.size() > 0)
        f.fill('0', l - s.size());
    return s.prepend(f).toAscii().toUpper();
}

unsigned int nabAsm::getUInt16(QByteArray b)
{
    return (unsigned char)(b.at(0)) + ((unsigned char)(b.at(1)) << 8);
}

unsigned int nabAsm::getUInt32(QByteArray b)
{
    return (unsigned char)(b.at(0)) + ((unsigned char)(b.at(1)) << 8) + ((unsigned char)(b.at(2)) << 16) + ((unsigned char)(b.at(3)) << 24);
}
