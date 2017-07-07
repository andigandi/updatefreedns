#include <QCoreApplication>

#include "manager.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Manager manager(&a);

    manager.start();

    return a.exec();
}
