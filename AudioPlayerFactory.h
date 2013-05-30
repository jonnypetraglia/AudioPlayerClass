#ifndef AUDIOPLAYERFACTORY_H
#define AUDIOPLAYERFACTORY_H



#include "AudioPlayer.h"

#ifdef _WIN32 || _WIN64
    #include "AudioPlayerWin.h"
#elif __APPLE__
    #include "AudioPlayerOsx.h"
#else
    #include "AudioPlayerGnu.h"
#endif

class AudioPlayerFactory
{
public:
    static AudioPlayer* createFromFile(const char* fn)
    {
    #ifdef _WIN32 || _WIN64
        return AudioPlayerWin::file(fn);
    #elif __APPLE__
        return AudioPlayerOsx::file(fn);
    #else
        return AudioPlayerGnu::file(fn);
    #endif
    }

private:
    AudioPlayerFactory();
};

#endif // AUDIOPLAYERFACTORY_H
