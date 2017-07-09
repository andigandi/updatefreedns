#include "output.h"
#include <iostream>

QTextStream Output::out(stdout);
QTextStream Output::err(stderr);

QMutex Output::mutex;

Output::Output(const QString prefix, QObject *parent) : QObject(parent),
    prefix(prefix)
{

}

void Output::writeOut(const QString text)
{
    this->write(&(Output::out), text);
}

void Output::writeErr(const QString text)
{
    this->write(&(Output::err), text);
}

void Output::write(QTextStream *stream, const QString text)
{
    Output::mutex.lock();
    *stream << this->prefix << QStringLiteral(": ") << text << endl;
    Output::mutex.unlock();
}
