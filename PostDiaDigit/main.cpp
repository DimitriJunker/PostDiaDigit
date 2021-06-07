#include "postdiadigit.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    PostDiaDigit w;
    w.show();

    return a.exec();
}
