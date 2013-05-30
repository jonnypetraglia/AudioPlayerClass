#ifndef AUDIOPLAYERWIN_H
#define AUDIOPLAYERWIN_H

#include "AudioPlayer.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <sstream>
#include <windows.h>


class AudioPlayerWin : public AudioPlayer
{
public:
    ~AudioPlayerWin();
    static AudioPlayerWin* file(const char *fn);
    bool isPlaying() const;
    bool isPaused() const;
    bool isStopped() const;
    double duration() const;
    double progress() const;
    void play();
    void pause();
    void seek(double sec);
    void setFinishListener(AudioPlayerCallback* cbo);
    void setFinishListenerHWND(HWND h);

private:
    enum MediaType { MP3, WAV };
    MediaType mType;
    AudioPlayerCallback* finishListener;
    std::string alias;

    bool LoadMP3(std::string &command);
    bool LoadWAV(std::string &command);

    AudioPlayerWin() : alias("media") {}
};

#endif // AUDIOPLAYERWIN_H
