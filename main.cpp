#include <QtGui/QApplication>
#include "virtualsms.h"
//#include <QtTest>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTextCodec *codec = QTextCodec::codecForName("gb2312");
    QTextCodec::setCodecForCStrings(codec);
    QTextCodec::setCodecForLocale(codec);
    QTextCodec::setCodecForTr(codec);    
    
//    // ȷ��ֻ����һ��
//    QSystemSemaphore sema("JAMKey",1,QSystemSemaphore::Open);
//    sema.acquire();// ���ٽ������������ڴ�   SharedMemory
//    QSharedMemory mem("VirtualSMS");// ȫ�ֶ�����
//    if (!mem.create(1))// ���ȫ�ֶ����Դ������˳�
//    {
//        QMessageBox::warning(0, QObject::tr("����"), QObject::tr("�����Ѿ�����."));
////        QMessageBox::warning(0, QObject::tr("msg"), QObject::tr("Program is already started"));
//        sema.release();// ����� Unix ϵͳ�����Զ��ͷš�
//        return 0;
//    }
//    sema.release();// �ٽ���

    VirtualSMS w;
    w.show();

    return a.exec();
}
