#include "AudioPlayerGnu.h"

const int AudioPlayerGnu::FILETYPE_COUNT = 8;
const char* AudioPlayerGnu::FILETYPES[AudioPlayerGnu::FILETYPE_COUNT] = {"wav", "mp3", "aif", "mp4", "wma", "b-mtp", "ogg", "spx"};

///////////////////////////Private

gboolean AudioPlayerGnu::bus_callback(GstBus *bus, GstMessage *msg, gpointer callingGnu)
{
    switch(GST_MESSAGE_TYPE(msg))
    {
    case GST_MESSAGE_EOS:
        gst_element_set_state(((AudioPlayerGnu*)callingGnu)->player, GST_STATE_READY);
        if(((AudioPlayerGnu*)callingGnu)->finishListener)
          ((AudioPlayerGnu*)callingGnu)->finishListener->playingFinished();
        g_debug("end-of-stream");
        break;
    case GST_MESSAGE_ERROR:
        gchar *debug;
        GError *err;
        gst_message_parse_error(msg, &err, &debug);
        g_free(debug);
        g_warning("Error: %s", err->message);
        g_error_free(err);
        std::cerr << "ERROR" << std::endl;
        break;
    default:
        break;
    }
    return TRUE;
}

void AudioPlayerGnu::init(int *argc, char **argv[])
{
    gst_init(argc, argv);
}

void AudioPlayerGnu::setWindow(gpointer window_)
{
    window = window_;
}

void AudioPlayerGnu::reset()
{
    gst_element_seek(player, 1.0, GST_FORMAT_TIME, (GstSeekFlags) seek_flags, GST_SEEK_TYPE_SET, 0, GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);
}

///////////////////////////Public

AudioPlayerGnu::~AudioPlayerGnu()
{
    //TODO???
    //delete pipeline;
    //delete videosink;
}

void AudioPlayerGnu::play()
{
    //stop();
    if(GST_IS_X_OVERLAY(videosink))
        gst_x_overlay_set_xwindow_id(GST_X_OVERLAY(videosink), GPOINTER_TO_INT(window));
    gst_element_set_state(player, GST_STATE_PLAYING);
}

void AudioPlayerGnu::stop() //TODO
{
    if(!player)
        return;
    gst_element_set_state(player, GST_STATE_NULL);
    gst_object_unref(GST_OBJECT(player));
    player = NULL;
}

void AudioPlayerGnu::pause()
{
    gst_element_set_state(player, GST_STATE_PAUSED);
}

void AudioPlayerGnu::seek(double t)
{
    gint value = t;
    gst_element_seek(player,          //pipeline element
                     1.0,               //rate
                     GST_FORMAT_TIME,   //GstFormat?
                     (GstSeekFlags) seek_flags,        //flags??
                     GST_SEEK_TYPE_SET, //start_type; if you want it to start
                     value * GST_MSECOND,//Time to seek to
                     GST_SEEK_TYPE_NONE,//
                     GST_CLOCK_TIME_NONE);
}

double AudioPlayerGnu::duration() const
{
    GstFormat format = GST_FORMAT_TIME;
    gint64 cur;
    gboolean result = gst_element_query_duration(player, &format, &cur);
    if(!result || format != GST_FORMAT_TIME)
        return GST_CLOCK_TIME_NONE;
    return cur;
}

double AudioPlayerGnu::progress() const
{
    GstFormat format = GST_FORMAT_TIME;
    gint64 cur;
    gboolean result = gst_element_query_position(player, &format, &cur);
    if(!result || format != GST_FORMAT_TIME)
        return GST_CLOCK_TIME_NONE;
    return cur;
}


bool AudioPlayerGnu::isPlaying() const
{
    GstState current, pending;
    gst_element_get_state(player, &current, &pending,0);
    std::cout << "derp " << current << "==" << GST_STATE_PLAYING << " (" << pending << ")" << std::endl;
    return current==GST_STATE_PLAYING;
}

bool AudioPlayerGnu::isPaused() const
{
    GstState current, pending;
    gst_element_get_state(player, &current, &pending,0);
    std::cout << "derp " << current << "==" << GST_STATE_PAUSED << " (" << pending << ")" << std::endl;
    return current==GST_STATE_PAUSED;
}

bool AudioPlayerGnu::isStopped() const
{
    GstState current, pending;
    gst_element_get_state(player, &current, &pending,0);
    std::cout << "derp " << current << "==" << GST_STATE_NULL << " (" << pending << ")" << std::endl;
    return current==GST_STATE_NULL;
}

    /*GstElement:
     *  GST_STATE_VOID_PENDING = 0
     *  GST_STATE_NULL = 1
     *  GST_STATE_READY = 2
     *  GST_STATE_PAUSED = 3
     *  GST_STATE_PLAYING = 4
     */
AudioPlayerGnu* AudioPlayerGnu::file(const char* fn)
{
    AudioPlayerGnu* a = new AudioPlayerGnu();
    a->init(NULL, NULL);
    a->filename = fn;
    a->player = gst_element_factory_make("playbin", "player");
    a->videosink = gst_element_factory_make("fakesink", "videosink");
    g_object_set(G_OBJECT(a->player), "video-sink", a->videosink, NULL);    //discard video


    {
        GstElement *audiosink = gst_element_factory_make("alsasink",
                                                         "audiosink");
        a->balance = gst_element_factory_make("audiopanorama", "panorama");

        // If panorama is available then construct a new compound audiosink with
        // panorama support.
        if (a->balance) {
          GstElement *audiobin = gst_bin_new("audiobin");
          gst_bin_add_many(GST_BIN(audiobin), a->balance, audiosink, NULL);
          gst_element_link(a->balance, audiosink);
          GstPad *sinkpad = gst_element_get_pad(a->balance, "sink");
          gst_element_add_pad(audiobin, gst_ghost_pad_new("sink", sinkpad));
          gst_object_unref(GST_OBJECT(sinkpad));
          audiosink = audiobin;
        }

        // Set audio-sink to our new audiosink.
        g_object_set(G_OBJECT(a->player), "audio-sink", audiosink, NULL);
    }



    {
        GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(a->player));
        gst_bus_add_watch(bus, bus_callback, gpointer(a));
        gst_object_unref(bus);
    }

    {
        gchar *uri = g_locale_to_utf8(a->filename,-1,NULL,NULL,NULL); //This fixes the Unicode problem
        if(gst_uri_is_valid( uri))
        {
            uri = g_strdup(uri);
        }
        else if(g_path_is_absolute(uri))
        {
            uri = g_filename_to_uri(uri, NULL, NULL);
        }
        else
        {
            gchar *tmp = g_build_filename(g_get_current_dir(), uri, NULL);
            uri = g_filename_to_uri(tmp, NULL, NULL);
            g_free(tmp);
        }

        g_debug("%s", uri); //Printing
        g_object_set(G_OBJECT(a->player), "uri", "file:///mnt/Tera/notbryant/Curmudgeon-build/Yael%20Na%C3%AFm.mp3", NULL);
        g_free(uri);
    }

    /*GST_STATE_CHANGE_
     *              FAILURE = 0
     *              SUCCESS = 1
     *              ASYNC = 2
     *              NO_PREROLL = 3
     */
    gst_element_set_state(a->player, GST_STATE_PAUSED); //Have to pause first to test if it is a valid file
    GstState current, pending;
    GstStateChangeReturn isValidFile = gst_element_get_state(a->player, &current, &pending,0);

    if(!isValidFile)
    {
        delete a; return NULL;
    }

    return a;
}

void AudioPlayerGnu::setFinishListener(AudioPlayerCallback* cbo)
{
    finishListener = cbo;
}

void AudioPlayerGnu::setVolume(int percentage)  //percentage ranges 0 to 100
{
    volumeForMute = percentage;
    g_object_set(G_OBJECT(player), "volume", percentage/10.0/10, NULL); //range from 1.0 to 10.0???
}

int AudioPlayerGnu::getVolume() const
{
    double v;
    g_object_get(G_OBJECT(player),"volume", &v, NULL);
    int i=v+0;
    return i;
}

void AudioPlayerGnu::mute()
{
    if(!_isMuted)
        g_object_set(G_OBJECT(player), "volume", 0, NULL);
    _isMuted=true;
}

void AudioPlayerGnu::unmute()
{
    if(_isMuted)
        setVolume(volumeForMute);
    _isMuted=false;
}

#define kMinBalance -100
#define kMaxBalance 100
template<typename T>
T Clamp(T val, T low, T high) {
    return val > high ? high : (val < low ? low : val);
}
void AudioPlayerGnu::setBalance(int b)
{
    b = Clamp(b, kMinBalance, kMaxBalance);
    gfloat gst_balance =
            (gfloat(b*100 - kMinBalance*100) / (kMaxBalance*100 - kMinBalance*100)) * 2 - 1;
    g_object_set(G_OBJECT(balance), "panorama", gst_balance, NULL);
}

int AudioPlayerGnu::getBalance() const
{
    gfloat bi;
    g_object_get(G_OBJECT(balance), "panorama", &bi, NULL);
    int b = bi/100;
    int gg_balance = static_cast<int>(((b + 1) / 2) *
                                    (kMaxBalance - kMinBalance) +
                                    kMinBalance);
    return Clamp(gg_balance, kMinBalance, kMaxBalance);
}
