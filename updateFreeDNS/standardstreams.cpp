#include "standardstreams.h"
#include <iostream>

QTextStream StandardStreams::out(stdout);
QTextStream StandardStreams::err(stderr);

QMutex StandardStreams::mutex;

void StandardStreams::lock()
{
    StandardStreams::mutex.lock();
}

void StandardStreams::unlock()
{
    StandardStreams::mutex.unlock();
}

StandardStreams::StandardStreams(QObject *parent) : QObject(parent)
{

}
