#ifndef MIDIFILEREADER_H
#define MIDIFILEREADER_H

#include <QObject>
#include <QFile>
#include <QVector>
#include "playthread.h"

class PlayThread;

class MIDIFileReader : public QObject
{
    Q_OBJECT
public:
    explicit MIDIFileReader(QObject *parent = 0);
    ~MIDIFileReader();
    void openMIDIFile(QString file_name);
    void closeMIDIFile();
    void playMIDIFile();
    void stopMIDIFile();

private:
    void handleMIDIEvent(quint8 channel, quint8 type, quint8 meta_type, QByteArray data);

private:
    PlayThread *playThread;
    bool isPlaying;
    QFile *midiFile;
    quint32 trk;
    struct midiEvent {
        qint32 delta;
        quint8 chn;
        quint8 msg;
        quint8 meta_type;
        QByteArray data;
    };
    struct midiTrkData {
        double nexttrigger;
        quint32 trkLength;
        quint32 curEventPos;
        QVector<struct midiEvent> events;
    };

    QVector<struct midiTrkData> midiTrk;
    qint32 midiFormat;
    qint32 totalTracks;
    quint32 currentTempo;
    double currentTickLength;
    quint16 timeDivision;
    double curTime;
    double curTempoTime;
    double nearestTimeTrigger;
    struct midiEvent curEvent;
    int activeTracks;
    HMIDIOUT hmo;

    void startProcessing();
    void parseMIDIFile();
    quint32 readVLQ();
    quint8 readChar();
    quint16 readShort();
    quint32 readLong();
    double getHRTime();
    void setTempo(QByteArray data);

public slots:
};

#endif // MIDIFILEREADER_H
