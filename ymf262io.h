#ifndef YMF262IO_H
#define YMF262IO_H

#include <QIODevice>
#ifdef __cplusplus
extern "C" {
#endif
#include "driver.h"
#include "ymf262.h"
#ifdef __cplusplus
}
#endif

class YMF262IO : public QIODevice
{
	Q_OBJECT

public:
    YMF262IO(QObject *parent, int bufsize);
	~YMF262IO();
    void start();
    void stop();
    void writeReg(int chip, int set, int reg, int val);
    void updateOne(INT16 *dst, int length);
    void enableOPL3();

private:
    int bufsize;
    QByteArray baData;

    // QIODevice interface
protected:
    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);
};
#endif // YMF262IO_H
