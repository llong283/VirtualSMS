/************************************************
类名: MsgTool
功能: 用于导入导出信息，并修复文件
************************************************/

#include "msgtool.h"
#include "filter.h"
#include "msgbox.h"

MsgTool::MsgTool(QObject *parent) :
    AbstractMsg(parent)
{
}

//导入信息
void MsgTool::importMsg()
{
    int existCount = 0;
    int failCount = 0;
    int successCount = 0;
    int totalCount = 0;
    QList<Message> failMsgs;
    QList<Message> existMsgs;
    QString strDetail;
    
    QString fileName = QFileDialog::getOpenFileName(NULL, 
                                                    tr("Import file"),
                                                    QApplication::applicationDirPath() + "/../Test",
                                                    tr("Xml document(*.xml);;All files(*.*)"));
    if (!fileName.isEmpty()) {        
        if (isFileValid(fileName) && initAbstractMsg(fileName, false) && isFileNormal()) {
            QList<Message> importMsgs = getAllMessages();
            totalCount = importMsgs.count();
            foreach (Message importMsg, importMsgs) {
                if (!isMsgExist(importMsg)) {
                    if (-1 == g_msgbox->addMessage(importMsg.phonenum,
                                                   importMsg.datetime,
                                                   importMsg.content,
                                                   (Message::MsgType)importMsg.msgtype,
                                                   (Message::Box)importMsg.box,
                                                   (Message::State)importMsg.state)) {
                        failCount++;
                        failMsgs.append(importMsg);
                    } else {
                        successCount++;
                    }
                } else {
                    existCount++;
                    existMsgs << importMsg;
                }
            }
        } else {
            strDetail = tr("Import file init fail. It's not valid.");
        }
        
        if (!failMsgs.isEmpty() || !existMsgs.isEmpty()) {
            QString strErrorDetail;
            foreach (Message msg, failMsgs) {
                strErrorDetail += convertMsgToLog(msg);
            }
            QString strExistDetail;
            foreach (Message msg, existMsgs) {
                strExistDetail += convertMsgToLog(msg);
            }
            strDetail = tr("ErrorMsgList: ") + strErrorDetail + "\n"
                    + tr("ExistMsgList: ") + strExistDetail + "\n";
        } else if (totalCount==0 && strDetail.isEmpty()) {
            strDetail = tr("Import file is empty");
        }
        
        QFile file(QApplication::applicationDirPath() + "/ImportMsg.log");
        if (file.open(QFile::WriteOnly | QIODevice::Append | QFile::Text)) {
            QTextStream ts(&file);
            QFileInfo fileInfo(file.fileName());
            if (fileInfo.size() == 0) {
                ts << tr("ImportMsg\n\n");
            }
            ts << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "\n";
            ts << strDetail;
            file.close();
        }
        
        QString strError = tr("Import result: total%1, success%2, fail%3, exist%4. See ImportMsg.log for more detail.")
                .arg(totalCount)
                .arg(successCount)
                .arg(failCount)
                .arg(existCount);
        QMessageBox::information(NULL, tr("msg"), strError);
    }   
}

//导出信息
void MsgTool::exportMsg(const QList<Message> &msgs)
{
    QString fileName = QFileDialog::getSaveFileName(NULL, 
                                                    tr("Export file"),
                                                    QApplication::applicationDirPath(),
                                                    tr("Xml document(*.xml);;All files(*.*)"));
    if (!fileName.isEmpty()) {
        QFileInfo fileInfo(fileName);
        if (fileInfo.exists()
                && QMessageBox::No == QMessageBox::information(NULL, tr("msg"), 
                                                               tr("File is exist. Do you want to overwrite it?"),
                                                               QMessageBox::Yes | QMessageBox::No,
                                                               QMessageBox::No)) {
                return;
        }
        
        initAbstractMsg(fileName, false);
        foreach (Message msg, msgs) {
            addMessage(msg.phonenum,
                       msg.datetime,
                       msg.content,
                       (Message::MsgType)msg.msgtype,
                       (Message::Box)msg.box,
                       (Message::State)msg.state);
        }
        save();
        QString strDetail = tr("Success export %1 messages").arg(msgs.count());
        QFile file(QApplication::applicationDirPath() + "/ExportMsg.log");
        if (file.open(QFile::WriteOnly | QIODevice::Append | QFile::Text)) {
            QTextStream ts(&file);
            QFileInfo fileInfo(file.fileName());
            if (fileInfo.size() == 0) {
                ts << tr("ExportMsg\n\n");
            }
            ts << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "\n";
            ts << strDetail;
            file.close();
        }        
        QMessageBox::information(NULL, tr("msg"), strDetail);
    }
}

//文件是否有效
bool MsgTool::isFileValid(const QString &fileName)
{
    QDomDocument doc;
    QString rootTag("MsgBox");
    bool isSuccess = false;
    QString errorStr;
    int errorLine;
    int errorColumn;
    QFileInfo fileInfo(fileName);
    if (fileInfo.size()==0) {
        QMessageBox::information(NULL, tr("msg"), tr("File is empty"));
        return false;
    }
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qDebug() << file.errorString() << __FILE__ << __LINE__;
        return false;
    }
    isSuccess = doc.setContent(&file, false, &errorStr, &errorLine, &errorColumn);
    if (isSuccess) {
        QDomElement root = doc.documentElement();
        if (root.tagName() != rootTag) {
            qDebug() << QString("Error: Not a %1 file. FileName:%2").arg(rootTag).arg(fileName)
                     << __FILE__ << __LINE__;
            QMessageBox::information(NULL, tr("msg"), 
                                     tr("Error: Not a %1 file. FileName:%2")
                                     .arg(rootTag).arg(fileName));
            isSuccess = false;
        }            
    } else {
        QMessageBox::information(NULL, tr("msg"), tr("Error: Parse error at line %1, column %2.\nError info:%3")
                                 .arg(errorLine).arg(errorColumn).arg(errorStr));
    }
    file.close();    
    return isSuccess;
}

//信息是否存在
bool MsgTool::isMsgExist(const Message &importMsg)
{
    QList<Message> msgs = g_msgbox->getMessagesOfDateTime(importMsg.datetime);
    foreach (Message msg, msgs) {
        if (msg.phonenum==importMsg.phonenum
                && msg.content==importMsg.content
                && msg.msgtype==importMsg.msgtype
                && msg.box==importMsg.box
                && msg.state==importMsg.state) {
            return true;
        }
    }
    return false;
}

//转换为记录信息
QString MsgTool::convertMsgToLog(const Message &msg)
{
    QString strLog;
    QString strMsgtype;
    if (msg.msgtype == Message::MsgType_InMsg) {
        strMsgtype = tr("In message");
    } else if (msg.msgtype == Message::MsgType_OutMsg) {
        strMsgtype = tr("Out message");
    } else {
        strMsgtype = tr("Invalid message type");
    }
    strLog = tr("PhoneNum: %1\nDateTime: %2\nContent: %3\nMessageType: %4 %5\nBox: %6 %7\nState: %8 %9\n\n")
            .arg(msg.phonenum)
            .arg(msg.datetime.toString("yyyy-MM-dd hh:mm:ss.zzz"))
            .arg(msg.content)
            .arg(msg.msgtype)
            .arg(strMsgtype)
            .arg(msg.box)
            .arg(Message::msgBoxToTr((Message::Box)msg.box))
            .arg(msg.state)
            .arg(Message::msgStateToTr((Message::State)msg.state));
    return strLog;
}

//修复
void MsgTool::repairMsg()
{   
    QMessageBox::information(NULL, tr("msg"), tr("Repair may lose some information. Please back it up before repair."));
    QString fileName = QFileDialog::getOpenFileName(NULL, 
                                                    tr("Choose repair file"),
                                                    QApplication::applicationDirPath(),
                                                    tr("Xml document(*.xml);;All files(*.*)"));
    if (!fileName.isEmpty()) {        
        if (isFileValid(fileName) && initAbstractMsg(fileName, false)) {
            if (repair()) {
                QMessageBox::information(NULL, tr("msg"), tr("Repair success"));
            } else {
                QMessageBox::information(NULL, tr("msg"), tr("Repair fail"));                
            }
        } else {
            QMessageBox::information(NULL, tr("msg"), tr("File is not valid. Repair fail"));
        }
    }
}
