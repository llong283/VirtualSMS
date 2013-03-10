/************************************************
类名: Setting
功能: 设置管理
说明: 为其他模块提供设置信息
************************************************/

#include "setting.h"

Setting *Setting::setting = NULL;

Setting::Setting(QObject *parent) :
    QObject(parent)
{
    m_settingFileName = QApplication::applicationDirPath() + "/other/setting.ini";
    m_keywordFileName = QApplication::applicationDirPath() + "/other/key.txt";
    updateSetting();
}

Setting *Setting::instance()
{
    if (setting == NULL) {
        setting = new Setting;
    }
    return setting;
}

void Setting::updateSetting()
{  
    QSettings settings(m_settingFileName, QSettings::IniFormat);
    settings.beginGroup("Filter");
    m_filterMode = (FilterMode) settings.value("FilterMode", FM_Unused).toInt();
    switch (m_filterMode) {
    case FM_Unused:
    case FM_Blacklist:
    case FM_Whitelist:
        break;
    default:
        settings.setValue("FilterMode", FM_Unused);
        m_filterMode = FM_Unused;
        break;
    }
    settings.endGroup();
    settings.beginGroup("Identify");
    m_isIdentifyKeyword = settings.value("IdentifyKeyword", true).toBool();
    m_isIdentifyNumber = settings.value("IdentifyNumber", true).toBool();
    m_isIdentifyNumberSegment = settings.value("IdentifyNumberSegment", true).toBool();
    settings.endGroup();
    settings.beginGroup("Other");
    int language = settings.value("Language", Language_Chinese).toInt();
    if (language!=Language_Chinese && language!=Language_English) {
        settings.setValue("Language", Language_Chinese);
        m_language = Language_Chinese;
        emit signLanguageChanged();
    } else if (language!=m_language) {
        m_language = language;        
        emit signLanguageChanged();
    }
    int runMode = settings.value("RunMode", RunMode_Chat).toInt();
    if (runMode!=RunMode_Chat && runMode!=RunMode_Message) {
        settings.setValue("RunMode", RunMode_Chat);
        m_runMode = RunMode_Chat;
        emit signRunModeChanged();
    } else if (runMode != m_runMode) {
        m_runMode = runMode;
        emit signRunModeChanged();
    }
    m_isCloseMin = settings.value("CloseMin", true).toBool();
    
    m_speakLanguage = settings.value("SpeakLanguage", Language_English).toInt();
    VoiceInfo defaultVoice = g_speech->getDefaultVoice();
    m_voiceInfo.id = settings.value("VoiceId", defaultVoice.id).toString();
    m_voiceInfo.name = settings.value("VoiceName", defaultVoice.name).toString();
    settings.endGroup();    

    m_keywords.clear();
    QFile file(m_keywordFileName);
    if (!file.open(QFile::ReadOnly)) {
        qDebug() << file.errorString() << __FILE__ << __LINE__;
    } else {
        while (!file.atEnd()) {
            m_keywords.append(QString(file.readLine()).trimmed());
        }
    }
    file.close();    
}

void Setting::changeMode(RunMode runMode)
{
    if (runMode != m_runMode) {
        m_runMode = runMode;
        QSettings settings(m_settingFileName, QSettings::IniFormat);        
        settings.setValue("Other/RunMode", runMode);  
    }
}
