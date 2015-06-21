#ifndef PLAYTHREAD_H
#define PLAYTHREAD_H

#include <QThread>
#include "mainwindow.h"
#include "ymf262io.h"
#include "midifilereader.h"
#include "synththread.h"

class MainWindow;
class MIDIFileReader;
class SynthThread;

class PlayThread : public QThread
{
public:
    PlayThread(QObject *parent, SynthThread *synthThread);
    ~PlayThread();
    void note_on(int chn, int n, int v);
    void note_off(int chn, int n, int v);
    void control_change(int chn, int c, int v);
    void program_change(int chn, int c);
    void stop();
    void start();
    void setMIDIFile(QString midi_file);

    // QThread interface
protected:
    void run();

private:
    MainWindow *mainWnd;
    SynthThread *synthThread;
    YMF262IO *ymf262;
    MIDIFileReader *midiReader;
    QString strMIDIFile;
    void readSong();
    void process_line(QString delta, QString type, QString ch, QString c, QString v);

};

#endif // PLAYTHREAD_H
