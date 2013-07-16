#include "AudioPlayerWin.h"

const int AudioPlayerWin::FILETYPE_COUNT = 2;
const char* AudioPlayerWin::FILETYPES[AudioPlayerWin::FILETYPE_COUNT] = {"mp3", "wav"};

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
    a->mainVolume = 100;
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
    std::string s = "status " + alias + " length";
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
    std::string s =  "status " + alias + " position";
    const char* c = s.c_str();
    mciSendStringA(c, mcidata, 128, 0);
    std::istringstream i(mcidata);
    double x;
    if (!(i >> x))
      return 0;
    return x;
}

bool AudioPlayerWin::play()
{
    std::string s;
    if(isPaused())
    {
        s = "resume " + alias;
        const char* c = s.c_str();
        int err = mciSendStringA(c, NULL, 0, 0);
        if(err>0)
            std::cerr << "MCI ERROR when resuming: " << err << std::endl;
        return err>0;
    }

    s =  "play " + alias + " notify";
    const char* c = s.c_str();
    int err = mciSendStringA(c, NULL, 0, finishListener->getCallbackHWND());
    if(err>0)
        std::cerr << "MCI ERROR when playing: " << err << " [" << alias << "] " << std::endl;
    return err>0;
}

void AudioPlayerWin::pause()
{
    std::string s;
    if(isPaused()||isStopped())
        return;
    else
    {
        s = "pause " + alias;
        const char* c = s.c_str();
        int err = mciSendStringA(c, NULL, 0, 0);
        if(err>0)
            std::cerr << "MCI ERROR when pausing: " << err << std::endl;
    }
}

void AudioPlayerWin::stop()
{
    std::string s;
    if(isStopped())
        return;
    else
    {
        s = "stop " + alias;
        const char* c = s.c_str();
        int err = mciSendStringA(c, NULL, 0, 0);
        if(err>0)
            std::cerr << "MCI ERROR when stopping: " << err << std::endl;
    }
}

void AudioPlayerWin::setVolume(int percentage)
{
    std::string s;
    std::stringstream ss;
    if(isStopped())
        return;
    //Left
    ss << percentage*10;    //Note the *10! MCI ranges from 0 to 1000
    s = "setaudio " + alias + " volume to " + ss.str();

    const char* c = s.c_str();
    int err = mciSendStringA(c, NULL, 0, 0);
    if(err>0)
        std::cerr << "MCI ERROR when voluming: " << err << std::endl;
    mainVolume = percentage;
}

int AudioPlayerWin::getVolume() const
{
    char mcidata[129]; // Data is returned by some MCI requests
    std::string s =  "status " + alias + " volume";
    const char* c = s.c_str();
    mciSendStringA(c, mcidata, 128, 0);
    std::istringstream i(mcidata);
    double x;
    if (!(i >> x))
        return 0;
    return x/10;    //Note the /10! MCI ranges from 0 to 1000
}

void AudioPlayerWin::mute()
{
    if(isMuted())
        return;
    std::string s = "setaudio " + alias + " audio all off";
    const char* c = s.c_str();
    mciSendStringA(c, NULL, 0, 0);
}

void AudioPlayerWin::unmute()
{
    if(!isMuted())
        return;
    std::string s = "setaudio " + alias + " audio all on";
    const char* c = s.c_str();
    mciSendStringA(c, NULL, 0, 0);
}

void AudioPlayerWin::setBalance(int LR) //-100 = Left, +100 = Right
{
    //TODO: I am not sure this is right; right now it only effects the opposite, i.e. <0 turns DOWN the right, but does not change the left
    std::stringstream ss;

    //LEFT
    if(LR < 0)
    {
        ss <<  (100-LR*-1)*10*mainVolume/100;
        std::string s = "setaudio " + alias + " right volume to " + ss.str();
        const char* c = s.c_str();
        mciSendStringA(c, NULL, 0, 0);
        std::cout << ">0  left " << (100-LR*-1)*10*mainVolume/100 << " " << s << std::endl;

        ss.str("");
        ss << 10*mainVolume;
        s = "setaudio " + alias + " left volume to " + ss.str();
        c = s.c_str();
        mciSendStringA(c, NULL, 0, 0);
        std::cout << ">0  left " << 10*mainVolume << " " << s << std::endl;
    }
    //RIGHT
    else if(LR > 0)
    {
        ss << (100-LR)*10*mainVolume/100;
        std::string s = "setaudio " + alias + " left volume to " + ss.str();
        const char* c = s.c_str();
        mciSendStringA(c, NULL, 0, 0);
        std::cout << ">0  left " << (100-LR)*10*mainVolume/100 << " " << s << std::endl;

        ss.str("");
        ss << 10*mainVolume;
        s = "setaudio " + alias + " right volume to " + ss.str();
        c = s.c_str();
        mciSendStringA(c, NULL, 0, 0);
        std::cout << ">0  left " << 10*mainVolume << " " << s << std::endl;
    }
    //CENTER
    else
    {
        std::stringstream ss;
        std::string s;
        const char* c;

        ss << 10*mainVolume;
        s = "setaudio " + alias + " left volume to " + ss.str();
        c = s.c_str();
        mciSendStringA(c, NULL, 0, 0);
        s = "setaudio " + alias + " right volume to " + ss.str();
        c = s.c_str();
        mciSendStringA(c, NULL, 0, 0);
    }
    std::cout << "GETBALANCE " << getBalance() << std::endl;
}

int AudioPlayerWin::getBalance() const
{
    char mcidata[129]; // Data is returned by some MCI requests
    double x;
    std::string s;
    const char* c;

    //LEFT
    {
        s =  "status " + alias + " left volume";
        c = s.c_str();
        mciSendStringA(c, mcidata, 128, 0);
        std::istringstream i;
        i >> x;
        if(x<1000) return (x/-10);
    }

    //RIGHT
    {
        s =  "status " + alias + " right volume";
        c = s.c_str();
        mciSendStringA(c, mcidata, 128, 0);
        std::istringstream i;
        i >> x;
        if(x<1000) return (x/10);
    }

    return 0;
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
    }

    command = "open \"" + command + "\" type mpegvideo alias " + alias;
    std::cout << command << std::endl;
    return mciSendStringA(command.c_str(), NULL, 0, 0)==0;
}

bool AudioPlayerWin::LoadWAV(std::string &command)
{
    command = "open \"" + command + "\" type waveaudio alias " + alias;
    return mciSendStringA(command.c_str(), NULL, 0, 0)==0;
}
