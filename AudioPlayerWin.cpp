#include "AudioPlayerWin.h"

/////////////////////////PUBLIC/////////////////////////

AudioPlayerWin::~AudioPlayerWin()
{
    std::string s ="close " + alias;
    const char* c = s.c_str();
    mciSendStringA(c, NULL, 0, 0);
}

//static
AudioPlayerWin* AudioPlayerWin::file(const char *fn)
{
    AudioPlayerWin* a = new AudioPlayerWin();
    std::string command = fn;
    std::string ext = command.substr(command.find_last_of(".")+1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    if(ext=="mp3")
    {
        a->mType = MP3;
        if(a->LoadMP3(command))
            return a;
    }
    else if(ext=="wav" || ext=="wave")
    {
        a->mType = WAV;
        if(a->LoadWAV(command))
            return a;
    }
    delete a;
    return NULL;
}

//Super();
bool AudioPlayerWin::isPlaying() const
{
    char mcidata[129]; // Data is returned by some MCI requests
    int mcidatalen=sizeof(mcidata)-1;

    std::string s = "status " + alias + " mode";
    const char* c = s.c_str();
    mciSendStringA(c,mcidata,mcidatalen,NULL);
    if (stricmp(mcidata,"playing") == 0)
        return TRUE;
    else
        return FALSE;
}

bool AudioPlayerWin::isPaused() const
{
    char mcidata[129]; // Data is returned by some MCI requests
    int mcidatalen=sizeof(mcidata)-1;

    std::string s = "status " + alias + " mode";
    const char* c = s.c_str();
    mciSendStringA(c,mcidata,mcidatalen,NULL);
    if (stricmp(mcidata,"paused") == 0)
        return TRUE;
    else
        return FALSE;
}

bool AudioPlayerWin::isStopped() const
{
    char mcidata[129]; // Data is returned by some MCI requests
    int mcidatalen=sizeof(mcidata)-1;

    std::string s = "status " + alias + " mode";
    const char* c = s.c_str();
    mciSendStringA(c,mcidata,mcidatalen,NULL);
    if (stricmp(mcidata,"stopped") == 0)
        return TRUE;
    else
        return FALSE;
}

double AudioPlayerWin::duration() const
{
    char mcidata[129]; // Data is returned by some MCI requests
    std::string s ="status " + alias + " length";
    const char* c = s.c_str();
    mciSendStringA(c, mcidata, 128, 0);
    std::istringstream i(mcidata);
    double x;
    if (!(i >> x))
      return 0;
    return x;
}

double AudioPlayerWin::progress() const
{
    char mcidata[129]; // Data is returned by some MCI requests
    std::string s = "status " + alias + " position";
    const char* c = s.c_str();
    mciSendStringA(c, mcidata, 128, 0);
    std::istringstream i(mcidata);
    double x;
    if (!(i >> x))
      return 0;
    return x;
}

void AudioPlayerWin::play()
{
    std::string s = "play " + alias + " notify";
    const char* c = s.c_str();
    int err = mciSendStringA(c, NULL, 0, finishListener->getCallbackHWND());
    if(err>0)
        std::cerr << "MCI ERROR when playing: " << err << std::endl;
}

void AudioPlayerWin::pause()
{
    std::string s;
    if(isPaused())
    {
        s = "resume " + alias;
        const char* c = s.c_str();
        int err = mciSendStringA(c, NULL, 0, 0);
        if(err>0)
            std::cerr << "MCI ERROR when resuming: " << err << std::endl;
    }
    else
    {
        s = "pause " + alias;
        const char* c = s.c_str();
        int err = mciSendStringA(c, NULL, 0, 0);
        if(err>0)
            std::cerr << "MCI ERROR when pausing: " << err << std::endl;
    }
}
void AudioPlayerWin::seek(double sec)
{
    std::stringstream ss;
    ss << sec*1000;
    std::string temp = "seek " + alias + " to " + ss.str();
    int err = mciSendStringA(temp.c_str(), NULL, 0, 0);
    if(err>0)
        std::cerr << "MCI ERROR when seeking: " << err << std::endl;
    std::cout <<  "seek to  " << ss.str() << std::endl;
}

void AudioPlayerWin::setFinishListener(AudioPlayerCallback* cbo)
{
    finishListener = cbo;
}

void AudioPlayerWin::setFinishListenerHWND(HWND h)
{
    if(finishListener)
        finishListener->setCallbackHWND(h);
    else
        std::cerr << "ERROR: callback not yet registered" << std::endl;
}


/////////////////////////PRIVATE/////////////////////////

bool AudioPlayerWin::LoadMP3(std::string &command)
{
    int pos=command.find('~');
    while(pos!=std::string::npos && command[pos-1]!='\\')
    {
        command.replace(pos,1,"\\~");
        pos = command.find('~', pos+2);
    }//*/
    command = "open \"" + command + "\" type mpegvideo alias " + alias;
    std::cout << command << std::endl;
    return mciSendStringA(command.c_str(), NULL, 0, 0)==0;
}

bool AudioPlayerWin::LoadWAV(std::string &command)
{
    command = "open \"" + command + "\" type waveaudio alias " + alias;
    return mciSendStringA(command.c_str(), NULL, 0, 0)==0;
}
