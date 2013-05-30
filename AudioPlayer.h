#ifndef EPOXY_AUDIOPLAYER_H
#define EPOXY_AUDIOPLAYER_H

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

class AudioPlayer {
public:
    //static AudioPlayer* url(const char* url);
    virtual ~AudioPlayer() {}

    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void seek(double t) = 0 ;

    virtual bool isPlaying() const = 0;
    virtual bool isPaused() const = 0;
    virtual bool isStopped() const = 0;
    virtual double duration() const = 0;
    virtual double progress() const = 0;
    
//    virtual AudioPlayer* file(const char *fn);
    virtual void setFinishListener(AudioPlayerCallback* cbo) = 0;
};


#endif
