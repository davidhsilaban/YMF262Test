#include "ymf262io.h"

YMF262IO::YMF262IO(QObject *parent, int bufsize)
    : bufsize(bufsize)
{
    int ret = YMF262Init(2, 14318180, 44100);
    qDebug("%d", ret);
    enableOPL3();
    baData.resize(1);
    //LocalAlloc();
}

YMF262IO::~YMF262IO()
{
    YMF262Shutdown();
}

/*
void YMF262IO::start()
{
    open(QIODevice::ReadOnly);
}


void YMF262IO::stop()
{
    close();
}
*/

void YMF262IO::writeReg(int chip, int set, int reg, int val)
{
    // Write the register to the chip
    YMF262Write(chip, 0x220+(set*2), reg);

    // Write the value to the chip
    YMF262Write(chip, 0x221+(set*2), val);
    //qDebug("%d %d %x %d", chip, set, reg, val);
}

void YMF262IO::enableOPL3() {
    writeReg(0, 1, 0x05, 0x1);
    writeReg(1, 1, 0x05, 0x1);
}

void YMF262IO::reset()
{
    YMF262ResetChip(0);
    YMF262ResetChip(1);
}

void YMF262IO::updateOne(INT16 *dst, int length)
{
    INT16 sample[2];
    if (baData.size() < (length*4)) {
        qDebug("Resize: %d", length);
        baData.resize(length*4);
    }
    INT16 *out = (INT16*)baData.data();
    YMF262UpdateOneQEMU(0, (INT16*)out, length);
    for (int cp = 1; cp < 2; cp++) {
        for (int p = 0; p < length; p++) {
            YMF262UpdateOneQEMU(cp, (INT16*)sample, 1);
            out[(p*2)] += sample[0];
            out[(p*2)+1] += sample[1];
        }
    }
    //dst = (INT16*)baData.data();
    memcpy(dst, out, length*4);
}

/*
qint64 YMF262IO::readData(char *data, qint64 maxlen)
{
    //YMF262UpdateOneQEMU(0, (INT16*)data, maxlen >> 2);
    qint64 total = 0;
    while (maxlen - total > 0) {
        const qint64 chunk = bufsize;
        updateOne((INT16*)(data + total), maxlen >> 2);
        total += chunk;
    }
    return total;
}

qint64 YMF262IO::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data)
    Q_UNUSED(len)
    return 0;
}
*/
