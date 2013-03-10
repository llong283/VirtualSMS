/************************************************
类名: DialogChatRoom
功能: 聊天界面，显示聊天记录
说明: 维护了一个发送队列，用于发送消息，如果发送失败，则输出
     发送失败信息
************************************************/

#include "dialogchatroom.h"
#include "ui_dialogchatroom.h"
#include <QNetworkInterface>
#include "phonebook.h"
#include "setting.h"
#include "messagereceiver.h"
#include "msgbox.h"

DialogChatRoom::DialogChatRoom(Contact contact, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogChatRoom),
    m_contact(contact)
{
    ui->setupUi(this);    
    
    //test
    ui->btnFont->hide();
    ui->btnColor->hide();
    
    retranslateUi();
    init();
}

DialogChatRoom::~DialogChatRoom()
{
    delete ui;
}

void DialogChatRoom::changeEvent(QEvent *e)
{
    if (e->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        retranslateUi();
    }
    QDialog::changeEvent(e);
}

void DialogChatRoom::retranslateUi()
{
    setWindowTitle(tr("%1(%2)").arg(m_contact.name, m_contact.phonenum));
    ui->btnClose->setToolTip(tr("Shortcut key: Esc or Ctrl+W"));
    ui->btnSend->setToolTip(tr("Shortcut key: Ctrl+Enter"));
    m_strInfo = QT_TR_NOOP("Name: %1\nNumber: %2\nType: %3\nIp: %4\nState: %5");
}

//初始化
void DialogChatRoom::init()
{
    m_myself = g_phoneBook->getMyself();

    //加载qss
    QFile file(":/qss/Dialog.qss");
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << file.errorString() << __FILE__ << __LINE__;
    } else {
        setStyleSheet(file.readAll());
    } 
    file.close();    

    setWindowFlags(Qt::Dialog | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    setAttribute(Qt::WA_DeleteOnClose);    
    QIcon icon(":/pic/Chat.png");
    setWindowIcon(icon);
    ui->textEditOutput->setReadOnly(true); 
    m_mainSplitter = new QSplitter(Qt::Vertical, this);
    m_mainSplitter->addWidget(ui->groupBoxOutput);
    m_mainSplitter->addWidget(ui->groupBoxInput);
    m_mainSplitter->setStretchFactor(0, 1);
    ui->groupBoxChat->layout()->addWidget(m_mainSplitter);
    
    ui->textEditContactInfo->setReadOnly(true);
    ui->textEditContactInfo->setText(tr(m_strInfo)
                                     .arg(m_contact.name)
                                     .arg(m_contact.getCurrentPhoneNum())
                                     .arg(Contact::contactTypeToString((Contact::Type)m_contact.type))
                                     .arg(g_phoneBook->getContactIpAddr(m_contact.id).toString())
                                     .arg(m_contact.state==Contact::State_Online ? tr("Online") : tr("Offline")));
    ui->textEditMyInfo->setReadOnly(true);
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    QString strIpAddr;
    for (int i = 0; i < ipAddressesList.size(); ++i) {
        QHostAddress ipAddr = ipAddressesList.at(i);
        if (ipAddr!=QHostAddress::LocalHost && ipAddr.toIPv4Address()) {
            strIpAddr += ipAddr.toString() + "\n    ";
        }
    }
    
    ui->textEditMyInfo->setText(tr(m_strInfo)
                                     .arg(m_myself.name)
                                     .arg(m_myself.getCurrentPhoneNum())
                                     .arg(Contact::contactTypeToString((Contact::Type)m_myself.type))
                                     .arg(strIpAddr.trimmed())
                                     .arg(tr("Online")));
    
    ui->btnSend->setShortcut(QKeySequence("Ctrl+Return"));
    ui->btnClose->setShortcut(QKeySequence("Ctrl+W"));
 
//    QList<Message> msgs = g_msgbox->getMessagesOfPhoneNum(m_contact.phonenum);
//    foreach (Message msg, msgs) {
//        if (msg.box==Message::Box_Inbox || msg.box==Message::Box_Outbox) {
//            addMsg(msg);
//        }
//    }
        
    activateWindow();
    ui->textEditInput->setFocus();
}

//添加消息至发送队列
void DialogChatRoom::append(QString strMsg)
{
    if (m_msgQueue.isEmpty()) {
        QTimer::singleShot(0, this, SLOT(slotStartNextSend()));
    }
    m_msgQueue.enqueue(strMsg);
}

//发送下一条消息
void DialogChatRoom::slotStartNextSend()
{
    if (m_msgQueue.isEmpty()) {
        return;
    }
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString strMsg = m_msgQueue.dequeue();
    bool isSend = m_sender.sendMessage(strMsg, m_contact.phonenum);
    g_msgbox->addMessage(m_contact.phonenum,
                         currentDateTime,
                         strMsg,
                         Message::MsgType_OutMsg,
                         isSend ? Message::Box_Outbox : Message::Box_Draftbox,
                         isSend ? Message::State_Sent : Message::State_SendFail);
    if (!isSend) {
        Message msg;
        msg.content = strMsg;
        msg.datetime = currentDateTime;
        msg.msgtype = Message::MsgType_OutMsg;
        addMsg(msg, tr("Send failed. Add to draftbox. Wait for next send."));
    }
    slotStartNextSend();
}

//输出消息并显示
void DialogChatRoom::addMsg(Message msg, QString tip)
{
    QTextCursor cursor(ui->textEditOutput->textCursor());
    cursor.movePosition(QTextCursor::End);
    QTextCharFormat charFormat;
    QString strDateTime;
    if (msg.datetime.date().daysTo(QDate::currentDate()) == 0) {
        strDateTime = msg.datetime.toString("hh:mm:ss");
    } else {
        strDateTime = msg.datetime.toString("yyyy-MM-dd hh:mm:ss");
    }
    if (msg.msgtype == Message::MsgType_InMsg) {
        charFormat.setForeground(QBrush(Qt::blue));
        cursor.insertText(QString("%1 %2")
                          .arg(m_contact.name)
                          .arg(strDateTime),
                          charFormat);
    } else {
        charFormat.setForeground(QBrush(Qt::darkGreen));
        cursor.insertText(QString("%1 %2").arg(m_myself.name).arg(strDateTime), charFormat);
        charFormat.setForeground(QBrush(Qt::red));
        if (!tip.isEmpty()) {
            cursor.insertText(" " + tip, charFormat);
        }
    }
    charFormat.setForeground(QBrush(Qt::black));
    cursor.insertBlock();
    cursor.insertText("  " + msg.content, charFormat);
    cursor.insertBlock();
    cursor.insertBlock();
    
    //滚动条移至底部
    QScrollBar *scrollBar = ui->textEditOutput->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximumHeight());        
}

//发送消息
void DialogChatRoom::on_btnSend_clicked()
{
    QString content = ui->textEditInput->toPlainText();
    if (!content.isEmpty()) {
        Message msg;
        msg.phonenum = m_contact.phonenum;
        msg.datetime = QDateTime::currentDateTime();
        msg.content = content;
        msg.msgtype = Message::MsgType_OutMsg;
        addMsg(msg);
        append(content);
        ui->textEditInput->clear();
    }
    ui->textEditInput->setFocus();
}

//关闭窗口
void DialogChatRoom::on_btnClose_clicked()
{
    close();
}
