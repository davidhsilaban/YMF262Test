#ifndef SYNTHTHREAD_H
#define SYNTHTHREAD_H

#include <QThread>
#include <QBuffer>
#include <QByteArray>
#include <QPair>
#include <QVector>
#include <QHash>
#include "ymf262io.h"
#include "mainwindow.h"

#define OPL_CHANNELS 36

static int NOTES[12] = {0x156, 0x16B, 0x181, 0x198, 0x1B0, 0x1CA, 0x1E5, 0x202, 0x220, 0x241, 0x263, 0x287};
static int OPS[9] = {0x0, 0x1, 0x2, 0x8, 0x9, 0xA, 0x10, 0x11, 0x12};
static int NOTE[128];
static QPair<qint32, qint32> USED[9] = {qMakePair((qint32)-1, (qint32)-1), qMakePair((qint32)-1, (qint32)-1), qMakePair((qint32)-1, (qint32)-1), qMakePair((qint32)-1, (qint32)-1), qMakePair((qint32)-1, (qint32)-1), qMakePair((qint32)-1, (qint32)-1), qMakePair((qint32)-1, (qint32)-1), qMakePair((qint32)-1, (qint32)-1), qMakePair((qint32)-1, (qint32)-1)};
//static int INSTR[16] = {494, 5933, 494, 494, 494, 494, 5933, 494, 494, 0, 494, 494, 494, 494, 494, 494};

class MainWindow;

struct soundbank_hdr {
    quint8 verMajor;
    quint8 verMinor;
    char signature[6];
    quint16 numUsed;
    quint16 numInstr;
    quint32 offsetName;
    quint32 offsetData;
};

struct soundbank_instr_hdr {
    quint16 index;
    quint8 flag;
    char name[9];
};

struct soundbank_instr_opl_regs {
    quint8 ksl;
    quint8 multiple;
    quint8 feedback;
    quint8 attack;
    quint8 sustain;
    quint8 eg;
    quint8 decay;
    quint8 releaseRate;
    quint8 totalLevel;
    quint8 am;
    quint8 vib;
    quint8 ksr;
    quint8 con;
};

struct soundbank_instr_data {
    quint8 iPercussive;
    quint8 iVoiceNum;
    struct soundbank_instr_opl_regs oplModulator;
    struct soundbank_instr_opl_regs oplCarrier;
    quint8 iModWaveSel;
    quint8 iCarWaveSel;
};

struct soundbank_data {
    struct soundbank_instr_hdr instrHeader;
    struct soundbank_instr_data instrData;
};

class SynthThread : public QThread
{
public:
    SynthThread(MainWindow *parent);
    ~SynthThread();
    YMF262IO *ymf262io;
    QBuffer *getSynthBuffer();

    void note_off(int chn, int n, int v);
    void note_on(int chn, int n, int v);
    void program_change(int chn, int c);
    void control_change(int chn, int c, int v);
    void start();
    void stop();
private:
    QBuffer *synthBuffer;
    MainWindow *mainWindow;
    char *audioData;
    int m_pos;
    int next_chn;
    int used_chns;
    int chan;
    int last_force_chn;
    int isActive;
    QVector< QVector<int> > midiCtrlValues;
    QVector< QHash<int, int> > midiSustainNotes;
    QVector< QHash<int, int> > midiActiveNotes;
    QVector<int> drumMap;
    QVector<int> programMap;
    QVector<int> INSTR;

    // QThread interface
    int find_next_chn(int chn, int note);
    int find_note_chn(int chn, int n);
    void testOPL3Sound();
    void opl3_set_instr(int opl_chn, int num);
    void opl3_channel_off(int opl_chn);
    void handleSustain(int chn);
    int force_find_next_chn(int chn, int note);

    // Adlib Soundbank functions
    void loadSoundbank();
    void loadProgramMap();
    void loadDrumMap();

    struct soundbank_hdr soundBankHdr;
    QVector<struct soundbank_data> soundBankData;
protected:
    void run();
};

#endif // SYNTHTHREAD_H
