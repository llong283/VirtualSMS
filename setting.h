#ifndef SETTING_H
#define SETTING_H

#include <QObject>
#include <QHostAddress>
#include "config.h"
#include "phonebook.h"
#include "QtSpeech.h"

#define g_setting Setting::instance()

class Setting : public QObject
{
    Q_OBJECT
public:
    static Setting *instance();   
    void updateSetting();    
    void changeMode(RunMode runMode);
//    inline QString getMyselfName() {
//        return g_phoneBook->getMyself().name;
//    }    
//    inline QString getMyselfPhoneNum() {
//        return g_phoneBook->getMyself().phonenum;
//    }    
    inline QList<QString> getKeywords() {
        return m_keywords;
    }    
    inline int getRunMode() {
        return m_runMode;
    }    
    inline int getFilterMode() {
        return m_filterMode;
    }
    inline int getLanguage() {
        return m_language;
    }
    inline bool getIsCloseMin() {
        return m_isCloseMin;
    }
    inline VoiceInfo getVoiceInfo() {
        return m_voiceInfo;
    }
    inline bool getIsIdentifyKeyword() {
        return m_isIdentifyKeyword;
    }
    inline bool getIsIdentifyNumber() {
        return m_isIdentifyNumber;
    }
    inline bool getIsIdentifyNumberSegment() {
        return m_isIdentifyNumberSegment;
    }
    inline int getSpeakLanguage() {
        return m_speakLanguage;
    }

private:
    Setting(QObject *parent = 0);
    static Setting *setting;
    QString m_settingFileName;
    QString m_keywordFileName;
    
    QString m_name;
    QString m_phoneNum;
    QList<QString> m_keywords;
    FilterMode m_filterMode;
    int m_runMode;
    int m_language;
    bool m_isCloseMin;
    bool m_isIdentifyKeyword;
    bool m_isIdentifyNumber;
    bool m_isIdentifyNumberSegment;
    VoiceInfo m_voiceInfo;
    int m_speakLanguage;
    
signals:
    void signRunModeChanged();
    void signLanguageChanged();

public slots:

};

#endif // SETTING_H
