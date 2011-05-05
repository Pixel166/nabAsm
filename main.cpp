#include <QtCore/QCoreApplication>
#include "nabasm.h"
#include <qfile.h>

#include <iostream>

nabAsm * d;

int main( int argc, char **argv )
{
    if(argc == 3)
    {
        QCoreApplication a( argc, argv );
        d = new nabAsm( argv[1], argv[2]  );
    }
    else
    {
        std::cout << "Usage : nabAsm <input> <output>" << std::endl;
    }
  return 0;
}
