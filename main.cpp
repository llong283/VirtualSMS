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
    
//    // 确保只运行一次
//    QSystemSemaphore sema("JAMKey",1,QSystemSemaphore::Open);
//    sema.acquire();// 在临界区操作共享内存   SharedMemory
//    QSharedMemory mem("VirtualSMS");// 全局对象名
//    if (!mem.create(1))// 如果全局对象以存在则退出
//    {
//        QMessageBox::warning(0, QObject::tr("警告"), QObject::tr("程序已经启动."));
////        QMessageBox::warning(0, QObject::tr("msg"), QObject::tr("Program is already started"));
//        sema.release();// 如果是 Unix 系统，会自动释放。
//        return 0;
//    }
//    sema.release();// 临界区

    VirtualSMS w;
    w.show();

    return a.exec();
}
