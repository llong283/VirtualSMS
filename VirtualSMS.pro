#-------------------------------------------------
#
# Project created by QtCreator 2012-06-10T18:43:27
#
#-------------------------------------------------

QT       += core gui network xml

TARGET = VirtualSMS
TEMPLATE = app

TRANSLATIONS = VirtualSMS.ts
RC_FILE = myapp.rc
#CONFIG += qtestlib

include(QtSpeech/QtSpeech.pri)

SOURCES += main.cpp\
        virtualsms.cpp \
    phonebook.cpp \
    setting.cpp \
    dialogsetting.cpp \
    dialogchatroom.cpp \
    dialogwritemsg.cpp \
    commonxmlprocess.cpp \
    itemdelegate.cpp \
    dialogmsgdetail.cpp \
    dialogcontact.cpp \
    filter.cpp \
    messagesender.cpp \
    messagereceiver.cpp \
    dialogchoosecontact.cpp \
    dialogmsgmanage.cpp \
    udpbroadcast.cpp \
    dialogaddidentification.cpp \
    msgmanageitemdelegate.cpp \
    phonenumberidentification.cpp \
    MessageIdentification.cpp \
    DistinguishPhrase.cpp \
    dialogdeletemsg.cpp \
    abstractmsg.cpp \
    msgbox.cpp \
    numbersegmentidentification.cpp \
    msgtool.cpp

HEADERS  += virtualsms.h \
    phonebook.h \
    setting.h \
    config.h \
    dialogsetting.h \
    dialogchatroom.h \
    dialogwritemsg.h \
    commonxmlprocess.h \
    itemdelegate.h \
    dialogmsgdetail.h \
    dialogcontact.h \
    messagesender.h \
    filter.h \
    messagereceiver.h \
    dialogchoosecontact.h \
    dialogmsgmanage.h \
    udpbroadcast.h \
    dialogaddidentification.h \
    msgmanageitemdelegate.h \
    MessageIdentification.h \
    DistinguishPhrase.h \
    phonenumberidentification.h \
    dialogdeletemsg.h \
    abstractmsg.h \
    msgbox.h \
    numbersegmentidentification.h \
    msgtool.h

FORMS    += virtualsms.ui \
    dialogsetting.ui \
    dialogchatroom.ui \
    dialogwritemsg.ui \
    dialogmsgdetail.ui \
    dialogcontact.ui \
    dialogchoosecontact.ui \
    dialogmsgmanage.ui \
    dialogaddidentification.ui \
    dialogdeletemsg.ui

RESOURCES += \
    resource.qrc
