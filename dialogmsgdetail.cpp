/************************************************
类名: DialogMsgDetail
功能: 显示信息详情
说明: 无
************************************************/

#include "dialogmsgdetail.h"
#include "ui_dialogmsgdetail.h"

#include "phonebook.h"
#include "setting.h"
#include "msgbox.h"
#include "QtSpeech.h"

#include "MessageIdentification.h"
#include "phonenumberidentification.h"
#include "numbersegmentidentification.h"

#include "dialogwritemsg.h"

DialogMsgDetail::DialogMsgDetail(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogMsgDetail)
{
    ui->setupUi(this);
    init();
}

DialogMsgDetail::~DialogMsgDetail()
{
    delete ui;
}

void DialogMsgDetail::init()
{
    m_isReading = false;
    
    //加载qss
    QFile file(":/qss/Dialog.qss");
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << file.errorString() << __FILE__ << __LINE__;
    } else {
        setStyleSheet(file.readAll());
    } 
    file.close();    

    setWindowFlags(Qt::Dialog | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    activateWindow();
    
    m_actMoveToInbox = new QAction(this);
    connect(m_actMoveToInbox, SIGNAL(triggered()), this, SLOT(slotMoveToInbox()));
    m_actMoveToOutbox = new QAction(this);
    connect(m_actMoveToOutbox, SIGNAL(triggered()), this, SLOT(slotMoveToOutbox()));
    m_actMoveToDraftbox = new QAction(this);
    connect(m_actMoveToDraftbox, SIGNAL(triggered()), this, SLOT(slotMoveToDraftbox()));
    m_actMoveToDustbin = new QAction(this);
    connect(m_actMoveToDustbin, SIGNAL(triggered()), this, SLOT(slotMoveToDustbin()));
    m_menuInMsg = new QMenu(this);
    m_menuInMsg->addAction(m_actMoveToInbox);
    m_menuInMsg->addAction(m_actMoveToOutbox);
    m_menuInMsg->addAction(m_actMoveToDraftbox);
    m_menuInMsg->addAction(m_actMoveToDustbin);
    ui->btnMoveTo->setMenu(m_menuInMsg);
    
    ui->textEditMsgDetail->setReadOnly(true);
        
    retranslateUi();
}

void DialogMsgDetail::changeEvent(QEvent *e)
{
    if (e->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        retranslateUi();
    }
    QDialog::changeEvent(e);    
}

void DialogMsgDetail::retranslateUi()
{
    setWindowTitle(tr("Message Detail"));
    m_actMoveToInbox->setText(tr("MoveToInbox"));    
    m_actMoveToOutbox->setText(tr("MoveToOutbox"));    
    m_actMoveToDraftbox->setText(tr("MoveToDraftbox"));    
    m_actMoveToDustbin->setText(tr("MoveToDustbin"));
    m_strSender     = QT_TR_NOOP("Sender:    %1");
    m_strState      = QT_TR_NOOP("State:     ");
    m_strDateTime   = QT_TR_NOOP("DateTime:  %1");                  
    m_strReceiver   = QT_TR_NOOP("Receiver:  %1");
    if (m_msg.id != 0) {
        loadMsg(m_msg);
    }
}

void DialogMsgDetail::loadMsg(Message msg)
{
    ui->textEditMsgDetail->clear();
    m_msg = msg;

    switch (m_msg.msgtype) {
    case Message::MsgType_InMsg:
        ui->btnReply->show();
        if (m_msg.state == Message::State_Unread) {
            g_msgbox->setRead(m_msg.id, true);
        }
        break;
    case Message::MsgType_OutMsg:
        ui->btnReply->hide();
        break;
    default:
        break;
    }
    updateActionState();    
    
    QTextCursor cursor(ui->textEditMsgDetail->textCursor());
    cursor.movePosition(QTextCursor::Start);
    
    QTextCharFormat charFormat;
    QString strDateTime;
    Contact contact;
    Contact myself;
    QString strSender;
    QString strReceiver;
    QString strContactInfo;
    QString strPersonalInfo;
    if (m_msg.datetime.date().daysTo(QDate::currentDate()) == 0) {
        strDateTime = m_msg.datetime.toString("hh:mm:ss");
    } else {
        strDateTime = m_msg.datetime.toString("yyyy-MM-dd hh:mm:ss");
    }
    contact = g_phoneBook->getContactOfPhoneNum(m_msg.phonenum);
    if (contact.isValid()) {
        strContactInfo = QString("%1<%2>").arg(contact.name).arg(contact.getCurrentPhoneNum());
    } else {
        strContactInfo = m_msg.phonenum;
    }
    myself = g_phoneBook->getMyself();
    strPersonalInfo = QString("%1<%2>").arg(myself.name).arg(myself.getCurrentPhoneNum());
    
    if (m_msg.msgtype == Message::MsgType_InMsg) {
        strSender = strContactInfo;
        strReceiver = strPersonalInfo;
    } else {
        strSender = strPersonalInfo;
        strReceiver = strContactInfo;
    }
    charFormat.setForeground(QBrush(Qt::blue));
    cursor.insertText(tr(m_strSender).arg(strSender), charFormat);
    cursor.insertBlock();
    cursor.insertText(tr(m_strDateTime).arg(strDateTime), charFormat);
    cursor.insertBlock();
    cursor.insertText(tr(m_strReceiver).arg(strReceiver), charFormat);
    cursor.insertBlock();
    cursor.insertText(tr(m_strState));
    cursor.insertText(Message::msgStateToTr((Message::State)msg.state));
    cursor.insertBlock();
    
    charFormat.setForeground(QBrush(Qt::black));
    cursor.insertBlock();
    cursor.insertText(m_msg.content, charFormat);
    cursor.insertBlock();

    //滚动条移至底部
    QScrollBar *scrollBar = ui->textEditMsgDetail->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximumHeight());       
}

void DialogMsgDetail::slotMoveToInbox()
{
    g_msgbox->changeBox(m_msg.id, Message::Box_Inbox);
    m_msg.box = Message::Box_Inbox;
    updateActionState();
}

void DialogMsgDetail::slotMoveToOutbox()
{
    g_msgbox->changeBox(m_msg.id, Message::Box_Outbox);
    m_msg.box = Message::Box_Outbox;
    updateActionState();
}

void DialogMsgDetail::slotMoveToDraftbox()
{
    g_msgbox->changeBox(m_msg.id, Message::Box_Draftbox);
    m_msg.box = Message::Box_Draftbox;
    updateActionState();
}

void DialogMsgDetail::slotMoveToDustbin()
{
    if (QMessageBox::No == QMessageBox::information(
                this, tr("msg"), tr("Are you sure to move message into dustbin?")
                , QMessageBox::Yes | QMessageBox::No, QMessageBox::No)) {
        return;
    }
    g_msgbox->changeBox(m_msg.id, Message::Box_Dustbin);
    m_msg.box = Message::Box_Dustbin;
    //识别
    g_MessageIdentification->MessageIdentificationProcessAndSave(m_msg.content);
    g_PhoneNumberIdentification->NumberIdentificationProcessAndSave(m_msg.phonenum);
    g_NumberSegmentIdentification->NSIProcessAndSave(m_msg.phonenum.toULongLong());
    updateActionState();
}

void DialogMsgDetail::updateActionState()
{
    m_actMoveToInbox->setEnabled(false);
    m_actMoveToOutbox->setEnabled(false);
    m_actMoveToDraftbox->setEnabled(false);
    m_actMoveToDustbin->setEnabled(false);
    switch (m_msg.msgtype) {
    case Message::MsgType_InMsg:
        if (m_msg.box == Message::Box_Inbox) {
            m_actMoveToDustbin->setEnabled(true);            
        } else {
            m_actMoveToInbox->setEnabled(true);                        
        }
        break;
    case Message::MsgType_OutMsg:
        if (m_msg.box == Message::Box_Dustbin) {
            m_actMoveToOutbox->setEnabled(true);
            m_actMoveToDraftbox->setEnabled(true);
        } else {
            m_actMoveToDustbin->setEnabled(true);
        }
        break;
    default:
        break;
    }
}

void DialogMsgDetail::slotFinishRead()
{
    ui->btnRead->setEnabled(true);
}

void DialogMsgDetail::on_btnReply_clicked()
{
    DialogWriteMsg *dwMsg = new DialogWriteMsg;
    dwMsg->initReply(m_msg.phonenum);
    dwMsg->show();    
}

void DialogMsgDetail::on_btnForwarding_clicked()
{
    DialogWriteMsg *dwMsg = new DialogWriteMsg;
    dwMsg->initForwarding(m_msg.content);
    dwMsg->show();        
}

void DialogMsgDetail::on_btnRead_clicked()
{
    if (g_speech->getStatus() == QtSpeech::State_Speaking) {
        QMessageBox::information(this, tr("msg"), tr("This is speaking. Please wait"));
        return;
    }
    g_speech->setVoice(g_setting->getVoiceInfo());
    g_speech->tell(m_msg.content, this, SLOT(slotFinishRead()));
}
