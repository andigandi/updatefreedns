#ifndef OUTPUT_H
#define OUTPUT_H

#include <QObject>
#include <QMutex>
#include <QTextStream>

class Output : public QObject
{
    Q_OBJECT
public:
    explicit Output(const QString prefix, QObject *parent = nullptr);

    void writeOut(const QString text);
    void writeErr(const QString text);

private:
    static QMutex mutex;

    static QTextStream out;
    static QTextStream err;

    const QString prefix;

    void write(QTextStream *stream, const QString text);
};

#endif // OUTPUT_H
