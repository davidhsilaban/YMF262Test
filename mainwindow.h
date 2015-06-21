#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtMultimedia/QAudioOutput>
#include <QTextStream>
#include <QFile>
#include <QTimer>
#include <QCloseEvent>
#include <QFileDialog>
#include <Windows.h>
#include "ymf262io.h"
#include "playthread.h"
#include "synththread.h"

class PlayThread;
class SynthThread;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QAudioOutput *audioOut;
    QIODevice *audioBuf;
    PlayThread *playThread;
    SynthThread *synthThread;
    QTimer *pullTimer;

private:
    Ui::MainWindow *ui;

private slots:
    void soundOn();
    void soundOff();
    void stateChanged(QAudio::State state);
    void timerTimeout();

private:
    void setupAudioOutput();
    void testOPL3Sound();
    void playMIDISong();
    void stopMIDISong();
    QByteArray audBuf;


    // QWidget interface
protected:
    void closeEvent(QCloseEvent *event);
};

#endif // MAINWINDOW_H
