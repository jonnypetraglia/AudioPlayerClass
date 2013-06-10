#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

//USAGE: DO NOT INCLUDE THIS FILE! Include 'AudioPlayerFactory' and use its "file()" function

/*NEED:
 *  -Play
 *  -Pause
 *  -Seek (to beginning)/Stop
 *  -Detect ending?
 *  -Get duration
 *  -isPlaying? / getStatus
 *  -Set volume
 */

#include "AudioPlayerCallback.h"
#include <string>

class AudioPlayer {
public:
    //static AudioPlayer* url(const char* url);
    virtual bool play() = 0;
    virtual void stop() = 0;
    virtual void pause() = 0;
    virtual void seek(double t) = 0;

    virtual bool isPlaying() const = 0;
    virtual bool isPaused() const = 0;
    virtual bool isStopped() const = 0;
    virtual double duration() const = 0;
    virtual double progress() const = 0;

    virtual void setVolume(int) = 0;
    virtual int getVolume() const = 0;
    virtual void mute() = 0;
    virtual void unmute() = 0;
    virtual bool isMuted() = 0;
    void toggleMute() { if(isMuted()) unmute(); else mute(); }

    virtual void setBalance(int LR) = 0;
    virtual int getBalance() const = 0;

    virtual void setFinishListener(AudioPlayerCallback* cbo) = 0;

    const static char* FILETYPES[];
};


#endif // AUDIOPLAYER_H
