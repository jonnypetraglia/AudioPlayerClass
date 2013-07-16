#ifndef AUDIOPLAYERWIN_H
#define AUDIOPLAYERWIN_H

#include "AudioPlayer.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <sstream>
#include <windows.h>
  //Required for GUID
#include <Rpc.h>
  //turning GUID to string
#include <iomanip>

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
    bool play();
    void stop();
    void pause();
    void seek(double sec);
    void setVolume(int percentage);
    int getVolume() const;
    void mute();
    void unmute();
    bool isMuted() { return _isMuted; }
    void setBalance(int LR); //-100 = Left, +100 = Right
    int getBalance() const;
    void setFinishListener(AudioPlayerCallback* cbo);
    void setFinishListenerHWND(HWND h);
    const static int FILETYPE_COUNT;
    const static char* FILETYPES[];

private:
    enum MediaType { MP3, WAV };
    MediaType mType;
    AudioPlayerCallback* finishListener;
    std::string alias;
    bool _isMuted;
    int leftBalance, rightBalance;
    int mainVolume;

    bool LoadMP3(std::string &command);
    bool LoadWAV(std::string &command);


    template <typename T>
    std::string toHex(const T& in, int width=2*sizeof(T))
    {
        std::ostringstream strstr;
        strstr << std::hex << std::setfill('0') << std::setw(width) << in;
        return strstr.str();
    }

    inline std::string toString(const GUID& id)
    {
        return
             toHex(id.Data1)+"-"+toHex(id.Data2)+"-"+toHex(id.Data3)+"-"
            +toHex((int)id.Data4[0],2)+toHex((int)id.Data4[1],2)+"-"
            +toHex((int)id.Data4[2],2)+toHex((int)id.Data4[3],2)
            +toHex((int)id.Data4[4],2)+toHex((int)id.Data4[5],2)
            +toHex((int)id.Data4[6],2)+toHex((int)id.Data4[7],2)
            ;
    }

    AudioPlayerWin(){
        GUID guid;
        UuidCreate(&guid);
        alias = toString(guid);
    }
};

#endif // AUDIOPLAYERWIN_H
