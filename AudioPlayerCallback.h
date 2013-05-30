#ifndef AUDIOPLAYERCALLBACK_H
#define AUDIOPLAYERCALLBACK_H

//This is an interface class; implement this to allow a class to have a listener for when a track is finished playing

class AudioPlayerCallback
{
public:
    virtual void playingFinished() = 0;
};

#endif
