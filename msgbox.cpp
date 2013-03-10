/************************************************
类名: MsgBox
功能: 管理信息，包括收件箱、发件箱、草稿箱、垃圾箱
说明: 该类继承AbstractMsg类
************************************************/

#include "msgbox.h"

MsgBox* MsgBox::msgbox = NULL;

MsgBox::MsgBox(QObject *parent) :
    AbstractMsg(parent)
{
}

/************************************************
功能：
    MsgBox单例初始化，属静态函数
参数：
    无
返回值:
    返回MsgBox单例指针
************************************************/
MsgBox *MsgBox::instance()
{
    if (msgbox == NULL) {
        msgbox = new MsgBox;
    }
    return msgbox;
}

/************************************************
功能：
    初始化
参数：
    无
返回值:
    初始化成功返回true, 否则返回false
************************************************/
bool MsgBox::init()
{
    qDebug() << "init" << __FILE__ << __LINE__;
    QString fileName(QApplication::applicationDirPath() + "/../Test/msgbox.xml");
    return initAbstractMsg(fileName, true);  
}
