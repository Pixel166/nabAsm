#ifndef NABASM_H
#define NABASM_H

#include <QCoreApplication>
#include <QMap>

class nabAsm
{
public:
    nabAsm(QString, QString);
private:
   unsigned int getUInt32(QByteArray);
   unsigned int getUInt16(QByteArray);
   QByteArray hexOutput(unsigned int);
   QByteArray hexOutput(unsigned int, int);
};

#endif // NABASM_H
