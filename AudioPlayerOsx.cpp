#include "AudioPlayerOsx.h"

const int AudioPlayerOsx::FILETYPE_COUNT = 13;
const char* AudioPlayerOsx::FILETYPES[AudioPlayerOsx::FILETYPE_COUNT] = {"aac", "adts", "ac3", "aif", "aiff", "aifc", "caf", "mp3", "m4a", "snd", "au", "sd2", "wav"};

#define checkStatus(status) checkStatus_(status, __FILE__, __LINE__)

//Inherited

bool AudioPlayerOsx::isPlaying() const
{
    return aqData.mIsRunning && !isPaused();
}

bool AudioPlayerOsx::isPaused() const
{
    return aqData.mIsRunning && paoosay;
}

bool AudioPlayerOsx::isStopped() const
{
    return !aqData.mIsRunning;
}

bool AudioPlayerOsx::play()
{
    if(isPlaying())
        return false;
    OSStatus status;
    if(isStopped())// || !isPaused())
    {
        aqData.mIsRunning = true;                          // 1
        aqData.mCurrentPacket = 0;                                // 1

        primeBuffer();

        Float32 gain = volume_level/100.0;
            // Optionally, allow user to override gain setting here
        status = AudioQueueSetParameter (                                  // 2
            aqData.mQueue,                                        // 3
            kAudioQueueParam_Volume,                              // 4
            gain                                                  // 5
        );
        checkStatus(status);
    }

    paoosay = false;
    status = AudioQueueStart (                                  // 2
        aqData.mQueue,                                 // 3
        NULL                                           // 4
    );
    checkStatus(status);
    return status<0;
}

void AudioPlayerOsx::pause()
{
    OSStatus status;
    if(isPaused() || isStopped())
        return;
    paoosay = true;

    status = AudioQueuePause(aqData.mQueue);
    checkStatus(status);
}

double AudioPlayerOsx::duration() const
{
    double dur = 0;
    unsigned int sz = sizeof(dur);
    OSStatus status = AudioFileGetProperty(aqData.mAudioFile, kAudioFilePropertyEstimatedDuration, (UInt32*)&sz, &dur);
    checkStatus(status);
    return dur;
}

void AudioPlayerOsx::seek(double sec)
{
    double frame = sec * aqData.mDataFormat.mSampleRate;

    AudioFramePacketTranslation trans;
    trans.mFrame = frame;

    unsigned int sz = sizeof(trans);
    OSStatus status = AudioFileGetProperty(aqData.mAudioFile, kAudioFilePropertyFrameToPacket, (UInt32*)&sz, &trans);

    seekToPacket(trans.mPacket);
    trans.mFrameOffsetInPacket = 0; // Don't support sub packet seeking..

    status = AudioFileGetProperty(aqData.mAudioFile, kAudioFilePropertyPacketToFrame, (UInt32*)&sz, &trans);

    timeBase = trans.mFrame / aqData.mDataFormat.mSampleRate;

}

double AudioPlayerOsx::progress() const
{
    double p = 0;

    AudioTimeStamp timeStamp;
    OSStatus status = AudioQueueGetCurrentTime (
       aqData.mQueue,
       NULL,
       &timeStamp,
       NULL
    );

//        checkStatus(status);

    p = timeStamp.mSampleTime/aqData.mDataFormat.mSampleRate + timeBase;
    return p;
}

AudioPlayerOsx* AudioPlayerOsx::file(const char *fn) {

    CFURLRef url = CFURLCreateFromFileSystemRepresentation(NULL, (UInt8*)fn, strlen(fn), false );

    AudioPlayerOsx* ap = new AudioPlayerOsx();
    ap->paoosay= false;

    if(!ap->load(url)) {
        delete ap;
        ap = NULL;
    }

    CFRelease(url);

    return ap;
}


void AudioPlayerOsx::stop()
{
    AudioQueueStop(aqData.mQueue, true);
    AudioQueueFlush(aqData.mQueue);
}

void AudioPlayerOsx::setVolume(int i)
{
    //Between 0.0 and 1.0
    volume_level = i;
    float macLevel = volume_level/100.0;
    AudioQueueSetParameter(aqData.mQueue,
                           kAudioQueueParam_Volume,
                           macLevel
                           );
}

int AudioPlayerOsx::getVolume() const
{
    AudioQueueParameterValue x;
    AudioQueueGetParameter(aqData.mQueue,kAudioQueueParam_Volume, &x);
    return x*100;
}

void AudioPlayerOsx::mute()
{
    if(!_isMuted)
        AudioQueueSetParameter(aqData.mQueue, kAudioQueueParam_Volume, 0);
    _isMuted=true;
}

void AudioPlayerOsx::unmute()
{
    if(_isMuted)
        setVolume(volume_level);
    _isMuted=false;
}

bool AudioPlayerOsx::isMuted()
{
    return _isMuted;
}

void AudioPlayerOsx::setBalance(int LR)
{
    float macPan = LR/100.0;
    AudioQueueSetParameter(aqData.mQueue,
                           kAudioQueueParam_Pan,
                           macPan);
}

int AudioPlayerOsx::getBalance() const
{
    AudioQueueParameterValue x;
    AudioQueueGetParameter(aqData.mQueue,kAudioQueueParam_Pan, &x);
    return x*100;
}

//static
void AudioPlayerOsx::HandleOutputBuffer (
    void                *aqData,
    AudioQueueRef       inAQ,
    AudioQueueBufferRef inBuffer
) {

//    std::cout << "cb" << std::endl;

    OSStatus status;

    AQPlayerState *pAqData = (AQPlayerState *) aqData;        // 1
    if (pAqData->mIsRunning == 0) return;                     // 2
    UInt32 numBytesReadFromFile;                              // 3
    UInt32 numPackets = pAqData->mNumPacketsToRead;           // 4
    status = AudioFileReadPackets (
        pAqData->mAudioFile,
        false,
        &numBytesReadFromFile,
        pAqData->mPacketDescs,
        pAqData->mCurrentPacket,
        &numPackets,
        inBuffer->mAudioData
    );
//    checkStatus(status);
    if (numPackets > 0) {                                     // 5
        inBuffer->mAudioDataByteSize = numBytesReadFromFile;  // 6
       status = AudioQueueEnqueueBuffer (
            pAqData->mQueue,
            inBuffer,
            (pAqData->mPacketDescs ? numPackets : 0),
            pAqData->mPacketDescs
        );
//        checkStatus(status);
        pAqData->mCurrentPacket += numPackets;                // 7
    } else {
        status = AudioQueueStop (
            pAqData->mQueue,
            false
        );
//        checkStatus(status);
        pAqData->mIsRunning = false;
        if(pAqData->callback)
        {
            std::cout<< "CALLBACK" << std::endl;
            pAqData->callback->playingFinished();
        }
    }
}


bool AudioPlayerOsx::load(CFURLRef url)
{
    OSStatus status;
    memset(&aqData,0,sizeof(aqData));
    timeBase = 0;

    status = AudioFileOpenURL(url,kAudioFileReadPermission,0,&aqData.mAudioFile);
    checkStatus(status);
    if( status != noErr ) return false;


    UInt32 dataFormatSize = sizeof (aqData.mDataFormat);    // 1

    status = AudioFileGetProperty (                                  // 2
        aqData.mAudioFile,                                  // 3
        kAudioFilePropertyDataFormat,                       // 4
        &dataFormatSize,                                    // 5
        &aqData.mDataFormat                                 // 6
    );
    checkStatus(status);


    status = AudioQueueNewOutput (                                // 1
        &aqData.mDataFormat,                             // 2
        HandleOutputBuffer,                              // 3
        &aqData,                                         // 4
        CFRunLoopGetCurrent (),                          // 5
        kCFRunLoopCommonModes,                           // 6
        0,                                               // 7
        &aqData.mQueue                                   // 8
    );
    checkStatus(status);

    UInt32 maxPacketSize;
    UInt32 propertySize = sizeof (maxPacketSize);
    status = AudioFileGetProperty (                               // 1
        aqData.mAudioFile,                               // 2
        kAudioFilePropertyPacketSizeUpperBound,          // 3
        &propertySize,                                   // 4
        &maxPacketSize                                   // 5
    );
    checkStatus(status);



    deriveBufferSize (                                   // 6
        aqData.mDataFormat,                              // 7
        maxPacketSize,                                   // 8
        0.5,                                             // 9
        &aqData.bufferByteSize,                          // 10
        &aqData.mNumPacketsToRead                        // 11
    );



    bool isFormatVBR = (                                       // 1
        aqData.mDataFormat.mBytesPerPacket == 0 ||
        aqData.mDataFormat.mFramesPerPacket == 0
    );

    if (isFormatVBR) {                                         // 2
        aqData.mPacketDescs =
          (AudioStreamPacketDescription*) malloc (
            aqData.mNumPacketsToRead * sizeof (AudioStreamPacketDescription)
          );
    } else {                                                   // 3
        aqData.mPacketDescs = NULL;
    }


    UInt32 cookieSize = sizeof (UInt32);                   // 1
    OSStatus couldNotGetProperty =                             // 2
        AudioFileGetPropertyInfo (                         // 3
            aqData.mAudioFile,                             // 4
            kAudioFilePropertyMagicCookieData,             // 5
            &cookieSize,                                   // 6
            NULL                                           // 7
        );
//    checkStatus(couldNotGetProperty);

    if (!couldNotGetProperty && cookieSize) {              // 8
        char* magicCookie =
            (char *) malloc (cookieSize);

        status = AudioFileGetProperty (                             // 9
            aqData.mAudioFile,                             // 10
            kAudioFilePropertyMagicCookieData,             // 11
            &cookieSize,                                   // 12
            magicCookie                                    // 13
        );
    checkStatus(status);

        status = AudioQueueSetProperty (                            // 14
            aqData.mQueue,                                 // 15
            kAudioQueueProperty_MagicCookie,               // 16
            magicCookie,                                   // 17
            cookieSize                                     // 18
        );
    checkStatus(status);

        free (magicCookie);                                // 19
    }
    return true;
}





void AudioPlayerOsx::primeBuffer()
{
    OSStatus status;

    for (int i = 0; i < kNumberBuffers; ++i) {                // 2
        status = AudioQueueAllocateBuffer (                            // 3
            aqData.mQueue,                                    // 4
            aqData.bufferByteSize,                            // 5
            &aqData.mBuffers[i]                               // 6
        );
        checkStatus(status);

        HandleOutputBuffer (                                  // 7
            &aqData,                                          // 8
            aqData.mQueue,                                    // 9
            aqData.mBuffers[i]                                // 10
        );
    }

    #if 1
    status = AudioQueuePrime (
       aqData.mQueue,
       kNumberBuffers,
       NULL
    );
    checkStatus(status);
    #endif
}



void AudioPlayerOsx::seekToPacket(uint64_t packet)
{
    AudioQueueStop(aqData.mQueue, true);
    AudioQueueFlush(aqData.mQueue);
    aqData.mCurrentPacket = packet;//rand()%1000;
    primeBuffer();
}


//static
void AudioPlayerOsx::checkStatus_(OSStatus status, const char* file, int line) {
    if(status != noErr) {
        std::cerr << file << ":" << line << ": ";
        char cc[5];
        *((unsigned int*)cc) = status;
        cc[4] = 0;
        std::cerr << "Error status " << status << ": " << cc << std::endl;
    } else {
//        std::cerr << "OK" << std::endl;
    }
}

AudioPlayerOsx::~AudioPlayerOsx()
{
    OSStatus status;

    status = AudioQueueDispose (                            // 1
        aqData.mQueue,                             // 2
        true                                       // 3
    );
    checkStatus(status);


    status = AudioFileClose(aqData.mAudioFile);
    checkStatus(status);

    free (aqData.mPacketDescs);                    // 5
}

//static
void AudioPlayerOsx::deriveBufferSize (
    AudioStreamBasicDescription &ASBDesc,                            // 1
    UInt32                      maxPacketSize,                       // 2
    Float64                     seconds,                             // 3
    UInt32                      *outBufferSize,                      // 4
    UInt32                      *outNumPacketsToRead                 // 5
) {
    static const int maxBufferSize = 0x50000;                        // 6
    static const int minBufferSize = 0x4000;                         // 7

    if (ASBDesc.mFramesPerPacket != 0) {                             // 8
        Float64 numPacketsForTime =
            ASBDesc.mSampleRate / ASBDesc.mFramesPerPacket * seconds;
        *outBufferSize = numPacketsForTime * maxPacketSize;
    } else {                                                         // 9
        *outBufferSize =
            maxBufferSize > maxPacketSize ?
                maxBufferSize : maxPacketSize;
    }

    if (                                                             // 10
        *outBufferSize > maxBufferSize &&
        *outBufferSize > maxPacketSize
    )
        *outBufferSize = maxBufferSize;
    else {                                                           // 11
        if (*outBufferSize < minBufferSize)
            *outBufferSize = minBufferSize;
    }

    *outNumPacketsToRead = *outBufferSize / maxPacketSize;           // 12
}
