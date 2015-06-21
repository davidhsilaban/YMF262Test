#include "synththread.h"

SynthThread::SynthThread(MainWindow *parent)
    : QThread(parent), m_pos(0)
{
    memset(NOTE, 0, sizeof(NOTE));
    mainWindow = parent;

    INSTR.resize(16);

    ymf262io = new YMF262IO(this, 0);
    synthBuffer = new QBuffer(this);
    //audioData = new QByteArray(1024, ' ');
    audioData = new char[4096];
    //QByteArray data(10240, ' ');
    //synthBuffer->setData(data);
    //synthBuffer->write(audioData, 4096);
    testOPL3Sound();
    loadSoundbank();
    loadProgramMap();
//    for (int c = 0; c < soundBankData.size(); c++) {
//        qDebug("Name: %s, Index: %d", soundBankData.at(c).instrHeader.name, c);
//    }
}

SynthThread::~SynthThread()
{
    synthBuffer->close();
    delete synthBuffer;
    delete audioData;
    delete ymf262io;
}

QBuffer *SynthThread::getSynthBuffer()
{
    return synthBuffer;
}

void SynthThread::run()
{
    //ymf262io->start();
    while (isActive) {
        //qDebug("bytesAvailable %ld", synthBuffer->bytesAvailable());
        while (synthBuffer->atEnd()) {
            m_pos = synthBuffer->pos();
            ymf262io->updateOne((INT16*)audioData, 4096>>2);
            synthBuffer->write(audioData, 4096);
            synthBuffer->seek(m_pos);
            //mainWindow->audioBuf->write(audioData, 1024);
            //qDebug("%d", m_pos);
            //qDebug("%s", synthBuffer->buffer().data());
        }
        Sleep(10);
    }
}

void SynthThread::testOPL3Sound()
{
    for (int i = 0; i < OPL_CHANNELS; i++) {
        ymf262io->writeReg((i/OPL_CHANNELS), (i/9), 0x20+OPS[i%9], 0x1);
        ymf262io->writeReg((i/OPL_CHANNELS), (i/9), 0x40+OPS[i%9], 0x10);
        ymf262io->writeReg((i/OPL_CHANNELS), (i/9), 0x60+OPS[i%9], 0xF0);
        ymf262io->writeReg((i/OPL_CHANNELS), (i/9), 0x80+OPS[i%9], 0x77);
        ymf262io->writeReg((i/OPL_CHANNELS), (i/9), 0x20+OPS[i%9]+3, 0x1);
        ymf262io->writeReg((i/OPL_CHANNELS), (i/9), 0x40+OPS[i%9]+3, 0x0);
        ymf262io->writeReg((i/OPL_CHANNELS), (i/9), 0x60+OPS[i%9]+3, 0xF0);
        ymf262io->writeReg((i/OPL_CHANNELS), (i/9), 0x80+OPS[i%9]+3, 0x77);
    }
    ymf262io->writeReg((0/OPL_CHANNELS), (0/9), 0xA0, 0x41);
    ymf262io->writeReg((0/OPL_CHANNELS), (0/9), 0xB0, 0x12);
}

void SynthThread::opl3_set_instr(int opl_chn, int num)
{
    struct soundbank_instr_data instr = soundBankData.at(num).instrData;
    int add = 0; // addition to operator's multiple to prevent lower octave

    // Validation
    //qDebug("%d %d", instr.oplModulator.multiple, instr.oplCarrier.multiple);
    if (instr.oplCarrier.multiple == 0 || instr.oplModulator.multiple == 0) {
        add = 1;
    }

    qDebug("%d", opl_chn);

    // Modulator Parameters
    ymf262io->writeReg((opl_chn/18), ((opl_chn/9)%2), 0x20+OPS[opl_chn%9],
                       (instr.oplModulator.am << 7) |
                       (instr.oplModulator.vib << 6) |
                       (instr.oplModulator.eg << 5) |
                       (instr.oplModulator.ksr << 4) |
                       (instr.oplModulator.multiple+add));
    ymf262io->writeReg((opl_chn/18), ((opl_chn/9)%2), 0x40+OPS[opl_chn%9], (instr.oplModulator.ksl << 6) |
                       instr.oplModulator.totalLevel);
    ymf262io->writeReg((opl_chn/18), ((opl_chn/9)%2), 0x60+OPS[opl_chn%9], (instr.oplModulator.attack << 4) |
                       instr.oplModulator.decay);
    ymf262io->writeReg((opl_chn/18), ((opl_chn/9)%2), 0x80+OPS[opl_chn%9], (instr.oplModulator.sustain << 4) |
                       instr.oplModulator.releaseRate);
    ymf262io->writeReg((opl_chn/18), ((opl_chn/9)%2), 0xE0+OPS[opl_chn%9], instr.iModWaveSel);

    // Carrier Parameters
    ymf262io->writeReg((opl_chn/18), ((opl_chn/9)%2), 0x20+OPS[opl_chn%9]+3, (instr.oplCarrier.am << 7) |
                       (instr.oplCarrier.vib << 6) |
                       (instr.oplCarrier.eg << 5) |
                       (instr.oplCarrier.ksr << 4) |
                       (instr.oplCarrier.multiple+add));
    ymf262io->writeReg((opl_chn/18), ((opl_chn/9)%2), 0x40+OPS[opl_chn%9]+3, (instr.oplCarrier.ksl << 6) |
                       instr.oplCarrier.totalLevel);
    ymf262io->writeReg((opl_chn/18), ((opl_chn/9)%2), 0x60+OPS[opl_chn%9]+3, (instr.oplCarrier.attack << 4) |
                       instr.oplCarrier.decay);
    ymf262io->writeReg((opl_chn/18), ((opl_chn/9)%2), 0x80+OPS[opl_chn%9]+3, (instr.oplCarrier.sustain << 4) |
                       instr.oplCarrier.releaseRate);
    ymf262io->writeReg((opl_chn/18), ((opl_chn/9)%2), 0xE0+OPS[opl_chn%9]+3, instr.iCarWaveSel);

    // Other Instrument Parameters
    ymf262io->writeReg((opl_chn/18), ((opl_chn/9)%2), 0xC0+(opl_chn%9), 0x30 | (instr.oplModulator.feedback << 1) | (!instr.oplModulator.con));
}

void SynthThread::opl3_channel_off(int opl_chn)
{
    int fnum = NOTES[USED[opl_chn].second%12];
    int oct = (USED[opl_chn].second/12)-1;
    //qDebug("%d", chan);
    ymf262io->writeReg((opl_chn/18), (opl_chn/9)%2, 0xA0+(opl_chn%9), fnum&0xFF);
    ymf262io->writeReg((opl_chn/18), (opl_chn/9)%2, 0xB0+(opl_chn%9), (oct<<2) | fnum>>8);
    USED[opl_chn] = QPair<qint32, qint32>(-1, -1);
    //chn--;
}

void SynthThread::loadSoundbank()
{
    struct soundbank_instr_hdr *nameRecords;
    struct soundbank_instr_data *dataRecords;
    struct soundbank_data *data;
    // Load soundbank file
    QFile soundBank("STANDARD.BNK");

    // Open soundbank file
    soundBank.open(QFile::ReadOnly);

    // Read header file
    soundBank.read((char*)&soundBankHdr, sizeof(struct soundbank_hdr));

    if (QString(soundBankHdr.signature).left(6) != "ADLIB-") {
        soundBank.close();
        return; // Invalid soundbank format
    }

    // Read instrument name records from the file
    nameRecords = new struct soundbank_instr_hdr[soundBankHdr.numInstr];
    soundBank.seek(soundBankHdr.offsetName);
    soundBank.read((char*)nameRecords, sizeof(struct soundbank_instr_hdr) * soundBankHdr.numInstr);

    // Read instrument name records from the file
    dataRecords = new struct soundbank_instr_data[soundBankHdr.numInstr];
    soundBank.seek(soundBankHdr.offsetData);
    soundBank.read((char*)dataRecords, sizeof(struct soundbank_instr_data) * soundBankHdr.numInstr);

    // Copy data from array to the instrument data QVector
    soundBankData.resize(soundBankHdr.numInstr);
    for (int i = 0; i < soundBankData.size(); i++) {
        data = soundBankData.data();
        memcpy((void*)&data[i].instrHeader, &nameRecords[i], sizeof(struct soundbank_instr_hdr));
        memcpy((void*)&data[i].instrData, &dataRecords[i], sizeof(struct soundbank_instr_data));
    }

    soundBank.close();
    delete[] nameRecords;
    delete[] dataRecords;
}

void SynthThread::loadProgramMap()
{
    QFile programsMapFile("program_map.txt");
    QTextStream programsMapStream(&programsMapFile);

    // Open the file
    programsMapFile.open(QFile::ReadOnly);

    // Read the file into the vector
    while (!programsMapStream.atEnd()) {
        programMap.append(programsMapStream.readLine().toInt());
    }

    // Close the file
    programsMapFile.close();
}

void SynthThread::note_on(int chn, int n, int v)
{
    int fnum = NOTES[n%12];
    int oct = (n/12)-1;
    //qDebug ("%x %d %x", fnum, oct, fnum&0xFF);
    //NOTE[n] = chn+1;

    if (v == 0) {
        note_off(chn, n, v);
        return;
    }

//    if (midiCtrlValues[chn][64] > 0) {
//        midiSustainNotes[chn].insert(n, 0);
//    }

    int chan = find_note_chn(chn, n);
    if (chan == -1) chan = find_next_chn(chn, n);
    //chan = (chan + 1) % 9;
    qDebug("%d", chan);
    opl3_channel_off(chan);
    opl3_set_instr(chan, INSTR[chn]);
    ymf262io->writeReg((chan/18), (chan/9)%2, 0xA0+(chan%9), fnum&0xFF);
    ymf262io->writeReg((chan/18), (chan/9)%2, 0xB0+(chan%9), 0x20 | (oct<<2) | fnum>>8);
    USED[chan] = QPair<qint32, qint32>(chn, n);
    midiActiveNotes[chn].insert(n, 1);
    used_chns = (used_chns+1) > OPL_CHANNELS ? OPL_CHANNELS : (used_chns+1);
}

void SynthThread::program_change(int chn, int c)
{
    if (programMap[c] == -1) {
        INSTR[chn] = 494; // $-PIANO6
    } else {
        INSTR[chn] = programMap[c];
    }
}

void SynthThread::handleSustain(int chn)
{
    int note;
    if (midiCtrlValues[chn][64] == 0) {
        foreach (note, midiSustainNotes[chn].keys()) {
            //qDebug("Note: %d", note);
            if (midiActiveNotes[chn].value(note) == 0) {
                //note_off(chn, note, 64);
                opl3_channel_off(find_note_chn(chn, note));
                midiSustainNotes[chn].remove(note);
            }
        }
        midiSustainNotes[chn].clear();
    }
}

void SynthThread::control_change(int chn, int c, int v)
{
    midiCtrlValues[chn][c] = v;

    switch (c) {
        case 0x40:
        qDebug("Sustain: %d %d", c, v);
        handleSustain(chn);
        break;
    }
}

void SynthThread::start()
{
    next_chn = -1;
    used_chns = 0;
    chan = 0;
    last_force_chn = 0;

    // Resize MIDI controller vectors to number of MIDI channels
    midiCtrlValues.clear();
    midiCtrlValues.resize(16);

    // Resize sustain notes vector to number of MIDI channels
    midiSustainNotes.clear();
    midiSustainNotes.resize(16);
    midiActiveNotes.clear();
    midiActiveNotes.resize(16);
    for (int c = 0; c < 16; c++) {
        midiSustainNotes[c] = QHash<int, int>();
        midiActiveNotes[c] = QHash<int, int>();
        // Set size to 128 controller values
        midiCtrlValues[c].clear();
        midiCtrlValues[c].resize(128);
    }

    USED.clear();
    USED.resize(OPL_CHANNELS);
    for (int opl_chn = 0; opl_chn < OPL_CHANNELS; opl_chn++) {
        USED[opl_chn] = qMakePair((qint32)-1, (qint32)-1);
    }
    ymf262io->reset();
    synthBuffer->open(QIODevice::ReadWrite);
    synthBuffer->seek(0);
    isActive = true;
    QThread::start();
}

void SynthThread::stop()
{
    isActive = false;
    while (isRunning()) {
        msleep(10);
    }
    synthBuffer->buffer().clear();
    synthBuffer->close();
    while (synthBuffer->isOpen()) {
        msleep(10);
    }
}

void SynthThread::note_off(int chn, int n, int v)
{
    if (midiCtrlValues[chn][64] > 0) {
        midiSustainNotes[chn].insert(n, 1);
        midiActiveNotes[chn].insert(n, 0);
        return;
    }

    int chan = find_note_chn(chn, n);
    if (chan == -1) return;
    opl3_channel_off(chan);
    midiActiveNotes[chn].insert(n, 0);
    used_chns--;
}

int SynthThread::force_find_next_chn(int chn, int note)
{
    int new_chn = ((++last_force_chn) % OPL_CHANNELS);
    if (USED[new_chn] != QPair<qint32, qint32>(-1, -1)) {
        //note_off(USED[new_chn].first, USED[new_chn].second, 64);
        opl3_channel_off(new_chn);
        //USED[new_chn] = QPair<qint32, qint32>(-1, -1);
    }
    return new_chn;
}

int SynthThread::find_next_chn(int chn, int note)
{
    int new_chn = -1;
    for (int i = 0; i < OPL_CHANNELS; i++) {
        if (USED[i] == QPair<qint32, qint32>(-1, -1)) {
            new_chn = i;
            break;
        }
    }
    if (new_chn == -1) {
        new_chn = force_find_next_chn(chn, note);
    }
    return new_chn;
}

int SynthThread::find_note_chn(int chn, int n)
{
    int ch = -1;
    for (int i = 0; i < OPL_CHANNELS; i++) {
        if (USED[i] == QPair<qint32, qint32>(chn, n)) {
            ch = i;
            break;
        }
    }
    return ch;
}
