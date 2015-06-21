#include "midifilereader.h"

MIDIFileReader::MIDIFileReader(QObject *parent) : QObject(parent), isPlaying(0)
{
    midiFile = new QFile();
    playThread = (PlayThread*)parent;
    midiOutOpen(&hmo, -1, NULL, NULL, 0);
}

MIDIFileReader::~MIDIFileReader() {
    midiOutClose(hmo);
    closeMIDIFile();
    midiTrk.clear();;
    delete midiFile;
}

void MIDIFileReader::openMIDIFile(QString file_name)
{
    midiFile->setFileName(file_name);
    midiFile->open(QFile::ReadOnly);
    while (midiTrk.size() > 0) {
        midiTrk[midiTrk.size()-1].events.clear();
        midiTrk.remove(midiTrk.size()-1);
    }
    parseMIDIFile();
}

void MIDIFileReader::closeMIDIFile()
{
    midiFile->close();
}

void MIDIFileReader::playMIDIFile()
{
    isPlaying = true;
    activeTracks = totalTracks;
    for (trk = 0; trk < totalTracks; trk++) {
        midiTrk[trk].curEventPos = 0;
        midiTrk[trk].nexttrigger = 0.0;
    }
    startProcessing();
}

void MIDIFileReader::stopMIDIFile()
{
    isPlaying = false;
}

void MIDIFileReader::setTempo(QByteArray data)
{
    quint32 usPerQuarterNote = 0;
    double new_time = 0;
    double old_tick_length = currentTickLength;
    quint32 old_tempo = currentTempo;
    double now = getHRTime();
    for (int pos = 0; pos < data.size(); pos++) {
        //qDebug("Tempo: %x", (quint32)((quint8)data.at(pos) << (8*(data.size()-pos-1))));
        usPerQuarterNote |= (quint32)((quint8)data.at(pos) << (8*(data.size()-pos-1)));
    }
    //qDebug("Tempo: %d", usPerQuarterNote);

    // Update timing variables
    currentTempo = usPerQuarterNote;
    currentTickLength = (double) (currentTempo/1000) / timeDivision;

    // Update track trigger times
    for (int mTrk = 0; mTrk < totalTracks; mTrk++) {
        if (midiTrk[mTrk].nexttrigger > 0) {
            new_time = (midiTrk[mTrk].nexttrigger-curTime) / old_tick_length * currentTickLength;
            midiTrk[mTrk].nexttrigger = curTime + new_time;
            //midiTrk[mTrk].nexttrigger += (midiTrk[mTrk].nexttrigger-curTime) / old_tick_length * currentTickLength;
            //midiTrk[mTrk].nexttrigger += (midiTrk[mTrk].nexttrigger-curTime) * (currentTickLength/old_tick_length);
            //midiTrk[mTrk].nexttrigger += (midiTrk[mTrk].nexttrigger-curTime) * (currentTempo/old_tempo);
        }
    }
}

void MIDIFileReader::handleMIDIEvent(quint8 channel, quint8 type, quint8 meta_type, QByteArray data)
{
    switch (type) {
    case 0xFF:
        switch(meta_type) {
        case 0x51:
            setTempo(data);
            break;
        }
        break;
    case 0x90:
        if (channel != 9) {
            //            if (data.at(1) > 0) {
            //                playThread->note_on(channel, data[0]);
            //                midiOutShortMsg(hmo, param1 << 8 | (param2 << 16) | (type | channel));
            //                qDebug("%d", data.at(0));
            //            } else {
            //                playThread->note_off(channel, data[0]);
                            //midiOutShortMsg(hmo, data.at(0) << 8 | (data.at(1) << 16) | (type | channel));
            //            }
            playThread->note_on(channel, data[0], data[1]);
            //qDebug("%d", data.at(0));
        }
        break;
    case 0x80:
        if (channel != 9) {
            playThread->note_off(channel, data[0], data[1]);
            //midiOutShortMsg(hmo, data.at(0) << 8 | (data.at(1) << 16) | (type | channel));
        }
        break;
    case 0xB0:
        playThread->control_change(channel, data[0], data[1]);
        break;
    case 0xC0:
        playThread->program_change(channel, data[0]);
        break;
    }
}

void MIDIFileReader::startProcessing()
{
    while (isPlaying) {
        curTime = getHRTime();
        nearestTimeTrigger = curTime;
        for (trk = 0; trk < totalTracks; trk++) {
            if (midiTrk[trk].curEventPos < midiTrk[trk].events.size()) {
                if (midiTrk[trk].nexttrigger == 0) {
                    midiTrk[trk].nexttrigger = curTime + (double)((double)midiTrk[trk].events.at(midiTrk[trk].curEventPos).delta*currentTickLength);
                    qDebug("Curtime: %lf, Next trigger: %lf", curTime, midiTrk[trk].nexttrigger);
                    //curTime = getHRTime();
                }
                //qDebug("Pos: %d, Track_Size: %d", midiTrk.at(trk).curEventPos, midiTrk.at(trk).events.size());
                //qDebug("Curtime: %lf, Next trigger: %lf, Delta: %d", curTime, midiTrk[trk].nexttrigger, midiTrk[trk].events.at(midiTrk[trk].curEventPos).delta);
                while (curTime >= midiTrk[trk].nexttrigger) {

                    if (midiTrk[trk].curEventPos >= midiTrk[trk].events.size()) {
                        activeTracks--;
                        break;
                    }

                    curEvent = midiTrk[trk].events.at(midiTrk[trk].curEventPos);
                //Sleep(curEvent.delta*0.9542);
                    //curTempoTime = curTime;
//                    qDebug("Curtime: %lf, Next trigger: %lf", curTime, midiTrk[trk].nexttrigger);
                    handleMIDIEvent(curEvent.chn, curEvent.msg, curEvent.meta_type, curEvent.data);
                    midiTrk[trk].curEventPos++;
                    if (midiTrk[trk].curEventPos < midiTrk[trk].events.size()) {
                        midiTrk[trk].nexttrigger += (double)((double)midiTrk[trk].events.at(midiTrk[trk].curEventPos).delta*currentTickLength);
                        //qDebug("Curtime: %lf, Next trigger: %lf", curTime, midiTrk[trk].nexttrigger);
                        //curTime = getHRTime();
                    }
                }
                if (midiTrk[trk].curEventPos >= midiTrk[trk].events.size()) {
                    continue;
                }
                if (midiTrk[trk].nexttrigger < nearestTimeTrigger) {
                    nearestTimeTrigger = midiTrk[trk].nexttrigger;
                }
                //Sleep(10);
            }
        }
        curTime = getHRTime();
        Sleep(curTime - nearestTimeTrigger);
        if (activeTracks <= 0) {
            isPlaying = false;
        }
    }
}

void MIDIFileReader::parseMIDIFile()
{
    qint32 file_pos = 0;
    quint8 cur_byte = 0;
    quint8 cur_event = 0;
    qint32 cur_delta = 0;
    struct midiEvent newEvent;
    qint32 chunk = 0;
    quint32 metaLen = 0;
    quint32 sysexLen = 0;
    quint32 cur_trk = 0;

    // Read MThd Chunk
    chunk = readLong(); //MThd
    readLong(); // MIDI File Length
    midiFormat = readShort(); // Get midi file format
    totalTracks = readShort(); // Get total midi tracks
    timeDivision = readShort(); // Get time division format

    // Set default tempo
    currentTempo = 500000;
    currentTickLength = (double) (currentTempo/1000) / timeDivision;

//    if (midiFormat != 0) { // Only MIDI Format 0 supported now
//        closeMIDIFile();
//        return;
//    }

    // Allocate MIDI Track data vector
    qDebug("Total tracks: %d", totalTracks);
    midiTrk.resize(totalTracks);

    // Read MIDI Tracks (MTrk)
    while (!midiFile->atEnd()) {
        chunk = readLong(); //MTrk
        midiTrk[cur_trk].trkLength = readLong(); // Read track event length
        file_pos = midiFile->pos();
        while (midiFile->pos() < file_pos+midiTrk[cur_trk].trkLength) { // Read events from current track
            cur_delta = readVLQ();
            //readVLQ();
            cur_byte = readChar();
            if (cur_byte > 0x7F) {
                if (cur_byte != 0xF0 && cur_byte != 0xFF) {
                    cur_event = cur_byte;
                }
            } else {
                //qDebug("%s", "Running STATUS");
                midiFile->seek(midiFile->pos()-1);
            }

            //cur_byte = readChar();
            newEvent.delta = cur_delta;
            if (cur_byte == 0xFF){
                newEvent.chn = 0;
                newEvent.msg = 0xFF;
                //newEvent.param1 = readChar();
                // data[0] contains the type of the meta event
                newEvent.meta_type = readChar();
                //newEvent.param2 = readVLQ();
                newEvent.data.resize(readVLQ());
                //midiFile->seek(midiFile->pos()+newEvent.param2);
                midiFile->read(newEvent.data.data(), newEvent.data.size());
                //                for (int i = 0; i < newEvent.param2; i++) {
                //                    qDebug("%c", readChar());
                //                }
                //qDebug("Pos: %lld, Event: 100", midiFile->pos());
            } else if (cur_byte == 0xF0) {
                newEvent.chn = 0;
                newEvent.msg = 0xF0;
                //newEvent.param1 = readVLQ();
                //newEvent.param2 = 0;
                newEvent.meta_type = 0;
                newEvent.data.resize(readVLQ());
                //                for (int i = 0; i < newEvent.param1; i++) {
                //                    qDebug("%c", readChar());
                //                }
                //midiFile->seek(midiFile->pos()+newEvent.param1);
                midiFile->read(newEvent.data.data(), newEvent.data.size());
                //qDebug("Pos: %d, Event: %d", midiFile->pos(), 0xF0);
            } else {
                newEvent.chn = cur_event & 0xF;
                newEvent.msg = cur_event & 0xF0;
                newEvent.meta_type = 0;
                if (newEvent.msg != 0xC0 && newEvent.msg != 0xD0) {
                    newEvent.data.resize(2);
                    newEvent.data[0] = readChar();
                    newEvent.data[1] = readChar();
                } else {
                    newEvent.data.resize(1);
                    newEvent.data[0] = readChar();
                }
            }

            midiTrk[cur_trk].events.append(newEvent);
            //new midiEvent{cur_delta, cur_event, cur_byte, 0};
            //            } else {
            //                if (cur_event == 0xFF) {
            //                    cur_byte = readChar();
            //                    metaLen = readVLQ();
            //                    if (metaLen == 0) {
            //                        metaLen = 1;
            //                    }
            //                    midiFile->seek(midiFile->pos()+metaLen);
            //                } else if (cur_event == 0xF0 || cur_event == 0xF7) {
            //                    sysexLen = readVLQ();
            //                    midiFile->seek(midiFile->pos()+sysexLen);
            //                }
            //                qDebug("Pos: %d, Event: %x", midiFile->pos(), cur_byte);
            //            }
        }
        cur_trk++;
    }
    closeMIDIFile();
}

quint32 MIDIFileReader::readVLQ()
{
    quint32 value;
    quint8 c;

    if((value = readChar()) & 0x80)
    {
        value &= 0x7f;
        do
        {
            value = (value << 7) + ((c = readChar()) & 0x7f);
        } while (c & 0x80);
    }
    return value;
}

quint8 MIDIFileReader::readChar()
{
    quint8 val = 0;
    midiFile->getChar((char*)&val);
    return val;
}

quint16 MIDIFileReader::readShort()
{
    quint16 val = 0;
    quint8 ch = 0;
    for (int i = 8; i >= 0; i-=8) {
        midiFile->getChar((char*)&ch);
        val |= (ch << i);
        //qDebug("%s", "read");
    }
    return val;
}

quint32 MIDIFileReader::readLong()
{
    quint32 val = 0;
    quint8 ch = 0;
    for (int i = 24; i >= 0; i-=8) {
        midiFile->getChar((char*)&ch);
        val |= (ch << i);
        //qDebug("%s", "read");
    }
    return val;
}

double MIDIFileReader::getHRTime()
{
    double cur_time = 0;
    timeBeginPeriod(1);
    cur_time = (double) timeGetTime();
    timeEndPeriod(1);
    return cur_time;
}

