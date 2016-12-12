//! \file main.cpp
//! \brief Source of main function.
#include "apduutility.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("Maxim Razuev");
    QCoreApplication::setApplicationName("APDU Utility");
    APDUUtility w;
    w.show();
    return a.exec();
}
