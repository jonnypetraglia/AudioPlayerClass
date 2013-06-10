#ifndef AUDIOPLAYERGNU_H
#define AUDIOPLAYERGNU_H

#include "AudioPlayer.h"

#include <gst/gst.h>
#include <gst/interfaces/xoverlay.h>
#include <iostream>
#include <string>

class AudioPlayerGnu : public AudioPlayer
{
private:
    const char* filename;
    GstElement *player;
    GstElement *videosink;
    GstElement *balance;
    gpointer window;
#define seek_flags (GST_SEEK_FLAG_FLUSH|GST_SEEK_FLAG_KEY_UNIT)
    AudioPlayerCallback* finishListener;
    int volumeForMute;
    bool _isMuted;
    AudioPlayerGnu() : _isMuted(false) {}

private:
    static gboolean bus_callback(GstBus *bus, GstMessage *msg, gpointer data);//Has to be static???
    void init(int *argc, char **argv[]);
    void setWindow(gpointer window_);
    void reset();

public:
    const static int FILETYPE_COUNT;
    const static char* FILETYPES[];

    ~AudioPlayerGnu();
    bool play();
    void stop();
    void pause();
    void seek(double t);
    double duration() const;
    double progress() const;
    bool isPlaying() const;
    bool isPaused() const;
    bool isStopped() const;

    void setVolume(int percentage);
    int getVolume() const;
    void mute();
    void unmute();
    bool isMuted() { return _isMuted; }

    void setBalance(int LR); //-100 = Left, +100 = Right
    int getBalance() const;

    static AudioPlayerGnu* file(const char *fn);
    void setFinishListener(AudioPlayerCallback* cbo);
};

#endif // AUDIOPLAYERGNU_H
