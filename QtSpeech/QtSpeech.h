/*  QtSpeech -- a small cross-platform library to use TTS
    Copyright (C) 2010-2011 LynxLine.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General
    Public License along with this library; if not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301 USA */

#ifndef QtSpeech_H
#define QtSpeech_H

#include <QObject>
#include <QMetaType>

//#if defined(QTSPEECH_STATIC)
//#   define QTSPEECH_API
//#elif defined(QTSPEECH_LIBRARY)
//#   define QTSPEECH_API Q_DECL_EXPORT
//#else
//#   define QTSPEECH_API /*Q_DECL_IMPORT*/
//#endif

#define g_speech QtSpeech::instance()

namespace QtSpeech_v1 { // API v1.0

// types
struct VoiceInfo { QString id; QString name; };
Q_DECLARE_METATYPE(VoiceInfo);

class /*QTSPEECH_API*/ QtSpeech : public QObject {
Q_OBJECT
public:
    // exceptions
    struct Error { QString msg; Error(QString s):msg(s) {} };
    struct InitError : Error { InitError(QString s):Error(s) {} };
    struct LogicError : Error { LogicError(QString s):Error(s) {} };
    struct CloseError : Error { CloseError(QString s):Error(s) {} };

    enum State {
        State_Stopped,
        State_Speaking,
        State_Paused
    };
    
    // api
    static QtSpeech* instance();

    virtual ~QtSpeech();

    const VoiceInfo & name() const; //!< Name of current voice
    static QList<VoiceInfo> voices();     //!< List of available voices in system

    void say(QString) const;                                    //!< Say something, synchronous
    void tell(QString) const;                                   //!< Tell something, asynchronous
    void tell(QString, QObject * obj, const char * slot) const; //!< Tell something, invoke slot at end
    void setVoice(VoiceInfo v);
    VoiceInfo getDefaultVoice();
    State getStatus();
    void pause();
    void resume();
    void stop();
    QList<VoiceInfo> getChineseVoices();
    QList<VoiceInfo> getEnglishVoices();

signals:
    void finished();

protected:
    virtual void timerEvent(QTimerEvent *);

private:
    QtSpeech(QObject * parent);
    QtSpeech(VoiceInfo n = VoiceInfo(), QObject * parent =0L);
    static QtSpeech *speech;    
    class Private;
    Private * d;    
};

}; // namespace QtSpeech_v1

using namespace QtSpeech_v1;

#endif // QtSpeech_H

