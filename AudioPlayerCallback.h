#ifndef AUDIOPLAYERCALLBACK_H
#define AUDIOPLAYERCALLBACK_H

//This is an interface class; implement this to allow a class to have a listener for when a track is finished playing

#ifdef _WIN32
#include <windows.h>
class AudioPlayerCallback
{
    //typedef HWND int;
protected:
    HWND callbackHWND;
public:
    HWND getCallbackHWND() { return callbackHWND; }
    void setCallbackHWND(const HWND &h) { callbackHWND = h; }
};
#else
class AudioPlayerCallback
{
public:
    virtual void playingFinished();
};
#endif

#endif
