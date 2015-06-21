#include "mainwindow.h"
#include "ui_mainwindow.h"

//static int OPS[9] = {0x0, 0x1, 0x2, 0x8, 0x9, 0xA, 0x10, 0x11, 0x12};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    audBuf(32768, 0)
{
    ui->setupUi(this);
    setupAudioOutput();
    pullTimer = new QTimer(this);
    connect(pullTimer, SIGNAL(timeout()), SLOT(timerTimeout()));
    playThread = new PlayThread(this, synthThread);
    //pullTimer->start(15);
    //playThread = new PlayThread(this, synthThread);
    //playThread->start();
}

MainWindow::~MainWindow()
{
    audioOut->stop();
    playThread->quit();
    synthThread->quit();
    delete ui;
    //delete txtStream;
    delete audioOut;
    delete playThread;
    delete synthThread;
    //delete ymf262;
}

void MainWindow::soundOn()
{
    //synthThread->ymf262io->writeReg(0xB0, 0x32);
    //audioOut->start(synthThread->getSynthBuffer());
    //audioOut->resume();
    stopMIDISong();
    playThread->setMIDIFile("gmstri00.mid");
    playMIDISong();
}

void MainWindow::soundOff()
{
    //synthThread->ymf262io->writeReg(0xB0, 0x12);
}

void MainWindow::setupAudioOutput()
{
    QAudioFormat fmt;
    fmt.setFrequency(44100);
    fmt.setChannels(2);
    fmt.setSampleSize(16);
    fmt.setCodec("audio/pcm");
    fmt.setByteOrder(QAudioFormat::LittleEndian);
    fmt.setSampleType(QAudioFormat::SignedInt);

    synthThread = new SynthThread(this);
    audioOut = new QAudioOutput(fmt);
    //audioOut->setNotifyInterval(100);
    connect(audioOut,SIGNAL(stateChanged(QAudio::State)),SLOT(stateChanged(QAudio::State)));
    //connect(audioOut, SIGNAL(notify()), SLOT(notify()));
    //ymf262 = new YMF262IO(this, audioOut->periodSize());
    //ymf262->start();
    //audioOut->setBufferSize(4410);
    //pullTimer->start();
    //audioOut->start(synthThread->getSynthBuffer());
}

void MainWindow::playMIDISong()
{
    audioBuf = audioOut->start();
    synthThread->start();
    pullTimer->start(15);
    playThread->start();
}

void MainWindow::stopMIDISong()
{
    playThread->stop();
    pullTimer->stop();
    synthThread->stop();
    audioOut->stop();
}

void MainWindow::stateChanged(QAudio::State state)
{
    qDebug("Audio State: %d\nError State: %d", state, audioOut->error());
    if (state == QAudio::IdleState) {
        //audioOut->stop();
        //audioOut->start(synthThread->getSynthBuffer());
        //audioOut->resume();
        //audioBuf->write(synthThread->getSynthBuffer()->data());
    }
}

void MainWindow::timerTimeout()
{
    //audioBuf->write(synthThread->getSynthBuffer()->data());
    //qDebug("%d", audioOut->bytesFree());
    //audioBuf->write(synthThread->getSynthBuffer()->read(audioOut->bytesFree()));
    //qDebug("%d", audioBuf->write(synthThread->getSynthBuffer()->read(audioOut->periodSize())));
    //qDebug("%d", synthThread->getSynthBuffer()->read(1024).size());
    int chunks = audioOut->bytesFree()/audioOut->periodSize();
    while (chunks) {
        const qint64 len = synthThread->getSynthBuffer()->read(audBuf.data(), audioOut->periodSize());
        if (len)
            audioBuf->write(audBuf.data(), len);
        if (len != audioOut->periodSize())
            break;
        --chunks;
    }
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    stopMIDISong();
    event->accept();
}
