/************************************************
����: MsgBox
����: ������Ϣ�������ռ��䡢�����䡢�ݸ��䡢������
˵��: ����̳�AbstractMsg��
************************************************/

#include "msgbox.h"

MsgBox* MsgBox::msgbox = NULL;

MsgBox::MsgBox(QObject *parent) :
    AbstractMsg(parent)
{
}

/************************************************
���ܣ�
    MsgBox������ʼ��������̬����
������
    ��
����ֵ:
    ����MsgBox����ָ��
************************************************/
MsgBox *MsgBox::instance()
{
    if (msgbox == NULL) {
        msgbox = new MsgBox;
    }
    return msgbox;
}

/************************************************
���ܣ�
    ��ʼ��
������
    ��
����ֵ:
    ��ʼ���ɹ�����true, ���򷵻�false
************************************************/
bool MsgBox::init()
{
    qDebug() << "init" << __FILE__ << __LINE__;
    QString fileName(QApplication::applicationDirPath() + "/../Test/msgbox.xml");
    return initAbstractMsg(fileName, true);  
}
