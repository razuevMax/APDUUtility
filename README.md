# APDUUtility
Program to work with smart cards in accordance with ISO 7816 protocol by sending an APDU command.

This program is built on top of the QWinSCard library (https://github.com/razuevMax/QWinSCard).

Documentation available on GitHub Pages: https://razuevmax.github.io/APDUUtility/

This program allows you to transmit and receive application protocol data units (APDUs) specified by ISO/IEC 7816-3 to a smart card in Qt5.

Compilation of verified -
Windows:
MS Visual Studio 2015/Qt 5.7

Qt Creator 4.1.0/Qt 5.7/mingw 5.3 32

Linux (tested on SUSE Linux 14):

Qt Creator 4.1.0/Qt 5.7/gcc 4

# Requirements
c++11 compiler (desirable с++14)

Qt 5

pcsc-lite library for linux/mac
