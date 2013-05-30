AudioPlayerLib
==============

# What is this?

A simple C++ "library" to assist in basic media file playing on Mac, Linux, and Windows

The idea is quite simply to provide a class that can be used across any platform without having to worry about it. There are obviously some fantastic libraries such as STFM, or PortAudio, or some of the proprietary ones that don't allow commercial use, but they tend to be (in order): not supportive of all audio types AND missing key abilities like being able to detect while a file stops playing, not at all written for an OO environment, and being difficult to put into commercial $5 apps without paying out the butt.

# Purpose

The idea is that you include one file (AudioPlayerFactory.h), and are able to create an object that acts -to you- the exact same on all platforms. Behind the scenes the object is really one of the subclasses, being Osx, Win, or Gnu.

For my purposes I wanted each object to be able to perform the following tasks:
  * Play
  * Pause
  * (Stop)
  * Seek
  * Detect finish
  * Get Duration
  * (Get Position)
  * getStatus (isPlaying/isPaused/isStopped)
  * Set volume

# How to use

Using this library in your application is quite simple.

1. Include "AudioPlayerFactory.h". Everything necessary will be included along with that file.
2. Declare an AudioPlayer*.
3. Create the object by calling AudioPlayerFactory::createFromFile();
4. If the result is not null it is a valid AudioPlayer object.

# Further usage

If you're going to be using the library with broader frameworks, e.g. the Qt framework, you'll need to take some more steps.

For Qt, you can modify the Pro file to compile conditionally:

    macx {
        SOURCES += AudioPlayerOsx.cpp
        HEADERS += AudioPlayerOsx.h
    }
    win32 {
        SOURCES += AudioPlayerWin.cpp
        HEADERS += AudioPlayerWin.h
    }
    unix {
        SOURCES += AudioPlayerGnu.cpp
        HEADERS += AudioPlayerGnu.h
   }
