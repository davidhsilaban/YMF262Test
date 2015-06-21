#include "playthread.h"

//static int NOTES[12] = {0x156, 0x16B, 0x181, 0x198, 0x1B0, 0x1CA, 0x1E5, 0x202, 0x220, 0x241, 0x263, 0x287};
//static int OPS[9] = {0x0, 0x1, 0x2, 0x8, 0x9, 0xA, 0x10, 0x11, 0x12};
//static int NOTE[128];
//static int USED[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

PlayThread::PlayThread(QObject *parent, SynthThread *synthThread)
{
    //mainWnd = parent;
    this->synthThread = synthThread;
    ymf262 = synthThread->ymf262io;
    midiReader = new MIDIFileReader(this);
    //connect(midiReader, SIGNAL(handleMIDIEvent(quint8,quint8,qint32,qint32)), this, SLOT(handleMIDIEvent(quint8,quint8,qint32,qint32)));
    //midiReader->openMIDIFile("C:\\Cakewalk Projects\\Sustain Test\\Sustain Test.mid");
}

PlayThread::~PlayThread()
{
    delete midiReader;
}

void PlayThread::run()
{
    //readSong();
    midiReader->playMIDIFile();
}
void PlayThread::readSong()
{
    QString delta, ch, c, v, type;
    QStringList line_params;
    QFile file("g.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    while (!in.atEnd()) {
        //QString line = in.readLine();
        line_params = in.readLine().split(" ");
        //in >> delta >> type >> ch >> c >> v;
        delta = line_params.at(0);
        if (line_params.length() > 1) {
            type = line_params.at(1);
            ch = line_params.at(2);
            if (line_params.length() > 3) {
                c = line_params.at(3);
                if (line_params.length() > 4) {
                    v = line_params.at(4);
                }
            }
        }
        process_line(delta, type, ch, c, v);
    }
    synthThread->quit();
    file.close();
}

void PlayThread::process_line(QString delta, QString type, QString ch, QString c, QString v)
{
    //qDebug("%s", type.toLatin1().data());
    Sleep(delta.toInt()*0.9542);
    if (type == "On" && ch.split("=").at(1).toInt() != 10) {
        if (v.split("=").at(1).toInt() > 0) {
            //note_on(c.split("=").at(1).toInt());
        } else {
            //note_off(c.split("=").at(1).toInt());
        }
        //qDebug("%d", c.split("=").at(1).toInt());
    } else if (type == "Off" && ch.split("=").at(1).toInt() != 10) {
        //note_off(c.split("=").at(1).toInt());
    }
}

void PlayThread::note_on(int chn, int n, int v)
{
    synthThread->note_on(chn, n, v);
}

void PlayThread::note_off(int chn, int n, int v)
{
    synthThread->note_off(chn, n, v);
}

void PlayThread::control_change(int chn, int c, int v)
{
    synthThread->control_change(chn, c, v);
}

void PlayThread::program_change(int chn, int c)
{
    synthThread->program_change(chn, c);
}

void PlayThread::stop()
{
    midiReader->stopMIDIFile();
}

void PlayThread::start()
{
    midiReader->openMIDIFile(strMIDIFile);
    QThread::start();
}

void PlayThread::setMIDIFile(QString midi_file)
{
    strMIDIFile = midi_file;
}
