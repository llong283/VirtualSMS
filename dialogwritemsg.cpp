/************************************************
类名: DialogWriteMsg
功能: 写信息、转发信息、回复信息界面
说明: 无
************************************************/

#include "dialogwritemsg.h"
#include "ui_dialogwritemsg.h"

#include "phonebook.h"
#include "msgbox.h"

const int cons_textedit_min_height = 22;

DialogWriteMsg::DialogWriteMsg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogWriteMsg)
{
    ui->setupUi(this);
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
    m_type = Type_Invalid;
    ui->textEditTip->setReadOnly(true);
    ui->textEditTip->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->textEditTip->hide();
    
    ui->textEditReceiver->setReadOnly(true);
    ui->textEditReceiver->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    ui->btnSend->setShortcut(QKeySequence("Ctrl+Return"));
    ui->btnClose->setShortcut(QKeySequence("Ctrl+W"));
    
    ui->treeWidget->header()->hide();
    initContactList();
    
    retranslateUi();
}

DialogWriteMsg::~DialogWriteMsg()
{
    delete ui;
}

void DialogWriteMsg::changeEvent(QEvent *e)
{
    if (e->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        retranslateUi();
    }
    QDialog::changeEvent(e);    
}

void DialogWriteMsg::closeEvent(QCloseEvent *ce)
{
    on_btnClose_clicked();
    QDialog::closeEvent(ce);
}

void DialogWriteMsg::keyPressEvent(QKeyEvent *ke)
{
    if (ke->key() == Qt::Key_Escape) {
        on_btnClose_clicked();
    }
    QDialog::keyPressEvent(ke);
}

void DialogWriteMsg::showEvent(QShowEvent *se)
{
    QDialog::showEvent(se);
    ui->textEditReceiver->setMaximumHeight(ui->textEditReceiver->document()->size().height());
    ui->textEditTip->setMaximumHeight(ui->textEditTip->document()->size().height());
}

void DialogWriteMsg::retranslateUi()
{
    switch (m_type) {
    case Type_WriteMsg:
        setWindowTitle(tr("Write Message"));
        break;
    case Type_Reply:
        setWindowTitle(tr("Reply"));
        break;
    case Type_Forwarding:
        setWindowTitle(tr("Forwarding"));
        break;
    default:
        setWindowTitle(tr("Write Message"));
        break;
    }
    
    for (int i=0; i<=Contact::Type_Stranger; i++) {
        m_contactTypeItems[i]->setText(0, Contact::contactTypeToString((Contact::Type)i));
    }
}

bool DialogWriteMsg::checkValidity()
{   
    qDebug() << "checkValid" << __FILE__ << __LINE__;
    QString strReceiver = ui->textEditReceiver->toPlainText();
    if (strReceiver.isEmpty()) {
        showTip(tr("Phone number can't be empty"));
        return false;
    }
    if (ui->textEditInput->toPlainText().isEmpty()) {
        showTip(tr("Message can't be empty"));
        return false;
    }
    if (ui->checkBoxManualInputNum->isChecked()) {
        QStringList phoneNums = strReceiver.split(";");
        bool isValid = true;
        foreach (QString phoneNum, phoneNums) {
            QRegExp regExpUnNum("\\D");
            int indexUnNum = regExpUnNum.indexIn(phoneNum.trimmed());
            if (indexUnNum != -1) {
                showTip(tr("Receiver number contains non-numeric character."));
                isValid = false;
                break;
            } else if (phoneNum.trimmed().length()>11) {
                showTip(tr("Receiver number length is more than 11"));
                isValid = false;
                break;
            }            
        }
        return isValid;
    }
    return true;
}

void DialogWriteMsg::showTip(const QString &strTip)
{
    ui->textEditTip->setText(strTip);
    ui->textEditTip->show();
}

void DialogWriteMsg::hideTip()
{
    ui->textEditTip->hide();
}

void DialogWriteMsg::initContactList()
{
    for (int i=0; i<=Contact::Type_Stranger; i++) {
        QTreeWidgetItem *twItem = new QTreeWidgetItem(ui->treeWidget);
        twItem->setSizeHint(0, QSize(0, cons_tree_item_height));
        QList<Contact> contacts = g_phoneBook->getContactsOfType((Contact::Type)i);
        foreach (Contact contact, contacts) {
            QTreeWidgetItem *twItemT = new QTreeWidgetItem(twItem);
            twItemT->setText(0, QString("%1(%2)").arg(contact.name).arg(contact.phonenum));
            twItemT->setData(0, Qt::UserRole, QVariant::fromValue<Contact>(contact));
            twItemT->setSizeHint(0, QSize(0, cons_tree_item_height));
            twItemT->setCheckState(0, Qt::Unchecked);
        }
        m_contactTypeItems << twItem;
    }
}

void DialogWriteMsg::init()
{
    if (m_type != Type_Invalid) {
        qDebug() << "already init" << __FILE__ << __LINE__;
        return;        
    }
    m_type = Type_WriteMsg;
    setWindowTitle(tr("Write Message"));
}

void DialogWriteMsg::initReply(QString phoneNum)
{
    if (m_type != Type_Invalid) {
        qDebug() << "already init" << __FILE__ << __LINE__;
        return;        
    }
    m_type = Type_Reply;
    setWindowTitle(tr("Reply"));
    Contact contact = g_phoneBook->getContactOfPhoneNum(phoneNum);
    if (contact.isValid()) {
        ui->textEditReceiver->setText(QString("%1<%2>").arg(contact.name).arg(contact.phonenum)); 
        QTreeWidgetItem *contactTopItem = m_contactTypeItems.at(contact.type);
        for (int i=contactTopItem->childCount()-1; i>=0; i--) {
            QTreeWidgetItem *child = contactTopItem->child(i);
            Contact contactT = child->data(0, Qt::UserRole).value<Contact>();
            if (contactT == contact) {
                child->setCheckState(0, Qt::Checked);
                contactTopItem->setExpanded(true);
                break;
            }
        }
    } else {
        contact.phonenum = phoneNum;
        ui->textEditReceiver->setText(QString("%1").arg(contact.phonenum));       
    }
    m_contacts << contact;
}

void DialogWriteMsg::initForwarding(QString content)
{
    if (m_type != Type_Invalid) {
        qDebug() << "already init" << __FILE__ << __LINE__;
        return;        
    }
    m_type = Type_Forwarding;
    setWindowTitle(tr("Forwarding"));
    ui->textEditInput->setText(content);
}

void DialogWriteMsg::initSendUnsendMsg(Message msg)
{
    ui->textEditInput->setText(msg.content);
    ui->textEditReceiver->setText(msg.phonenum);
    bool isManualInput = true;
    if (checkValidity()) {
        //判断电话号码是否都在电话本，如果在，则不是手动写信息
        QStringList phoneNums = msg.phonenum.split(";");
        QList<Contact> contacts;
        bool isAllContact = true;
        foreach (QString phoneNum, phoneNums) {
            QString phoneNumT = phoneNum.trimmed();
            if (phoneNumT.isEmpty()) {
                continue;
            }
            Contact contact = g_phoneBook->getContactOfPhoneNum(phoneNumT);
            if (contact.isValid()) {
                contacts << contact;
            } else {
                isAllContact = false;
                qDebug() << "phonenum isn't in phonebook" << phoneNumT << __FILE__ << __LINE__;
                break;
            }
        }
        if (isAllContact) {
            isManualInput = false;
            m_contacts = contacts;
            QString strReceivers;
            foreach (Contact contact, contacts) {
                strReceivers += QString("%1<%2>; ").arg(contact.name).arg(contact.phonenum);
                QTreeWidgetItem *contactTopItem = m_contactTypeItems.at(contact.type);
                for (int i=contactTopItem->childCount()-1; i>=0; i--) {
                    QTreeWidgetItem *child = contactTopItem->child(i);
                    Contact contactT = child->data(0, Qt::UserRole).value<Contact>();
                    if (contactT == contact) {
                        child->setCheckState(0, Qt::Checked);
                        contactTopItem->setExpanded(true);
                        break;
                    }
                }
            }
            ui->textEditReceiver->setText(strReceivers);        
        }
    }
    if (isManualInput) {
        ui->treeWidget->setEnabled(false);
        ui->checkBoxManualInputNum->setChecked(true);
        ui->textEditReceiver->setReadOnly(false);
    }
}

void DialogWriteMsg::on_btnSend_clicked()
{   
    if (!checkValidity()) {
        return;
    }
        
    QStringList phoneNums;
    if (ui->checkBoxManualInputNum->isChecked()) {
        QStringList phoneNumsT = ui->textEditReceiver->toPlainText().split(";");
        foreach (QString phoneNum, phoneNumsT) {
            if (!phoneNum.trimmed().isEmpty()) {
                phoneNums.append(phoneNum.trimmed());
            }
        }
    } else {
        foreach (Contact contact, m_contacts) {
            phoneNums << contact.phonenum;
        }
    }
    QString content = ui->textEditInput->toPlainText();
    QStringList failedList = m_udpSender.sendMessage(content, phoneNums);    
    qDebug() << failedList << __FILE__ << __LINE__;
    foreach (QString phoneNum, phoneNums) {
        bool isFailed = failedList.contains(phoneNum);        
        g_msgbox->addMessage(phoneNum,
                             QDateTime::currentDateTime(),
                             content,
                             Message::MsgType_OutMsg,
                             isFailed ? Message::Box_Draftbox : Message::Box_Outbox,
                             isFailed ? Message::State_SendFail : Message::State_Sent);
    }

    if (!failedList.isEmpty()) {
        QString tip(tr("Some message send failed, they are saved to draftbox. Failed list: "));
        foreach (QString str, failedList) {
            if (!str.isEmpty()) {
                tip += str + ", ";
            }
        }
        int index = tip.lastIndexOf(",");
        tip.remove(index, 2);
        showTip(tip);
    } else {
        if (QMessageBox::No == QMessageBox::information(
                    this, tr("msg"), tr("Do you still want to send message?")
                    , QMessageBox::Yes | QMessageBox::No, QMessageBox::No)) {        
            close();
        } else {
            hideTip();
            if (!ui->checkBoxManualInputNum->isChecked()) {
                for (int i=0; i<=Contact::Type_Stranger; i++) {
                    QTreeWidgetItem *contactTopItem = m_contactTypeItems.at(i);
                    for (int i=contactTopItem->childCount()-1; i>=0; i--) {
                        QTreeWidgetItem *child = contactTopItem->child(i);
                        if (child->checkState(0) == Qt::Checked) {
                            child->setCheckState(0, Qt::Unchecked);
                        }
                    }
                }
                m_contacts.clear();
            }
            ui->textEditInput->clear();
            ui->textEditReceiver->clear();
        }
    }    
}

void DialogWriteMsg::on_btnClose_clicked()
{
    if (!ui->textEditInput->toPlainText().isEmpty() 
            && QMessageBox::Yes == QMessageBox::information(
                this, tr("msg"), tr("Do you want to save the message to draftbox?")
                , QMessageBox::Yes | QMessageBox::No, QMessageBox::No)) {        
        QString strPhoneNum;
        if (ui->checkBoxManualInputNum->isChecked()) {
            strPhoneNum = ui->textEditReceiver->toPlainText();
        } else {
            foreach (Contact contact, m_contacts) {
                strPhoneNum.append(contact.phonenum + ";");
            }
        }
        g_msgbox->addMessage(strPhoneNum,
                             QDateTime::currentDateTime(),
                             ui->textEditInput->toPlainText(),
                             Message::MsgType_OutMsg,
                             Message::Box_Draftbox,
                             Message::State_Unsend);
    }
    //由于事件响应里也会执行一次该函数，所以要清空一下
    ui->textEditInput->clear();
    close();
}

void DialogWriteMsg::on_treeWidget_itemClicked(QTreeWidgetItem* item, int column)
{
    if (ui->checkBoxManualInputNum->isChecked()) {
        item->setCheckState(0, Qt::Unchecked);
        return;
    }
    
    Contact contact = item->data(0, Qt::UserRole).value<Contact>();
    Qt::CheckState checkState = item->checkState(0);
    int index = m_contacts.indexOf(contact);    
    if (checkState == Qt::Checked) {  
        if (index == -1) {            
            m_contacts.append(contact);
        }
    } else {        
        if (index != -1) {
            m_contacts.removeAt(index);
        }
    }

    QString str;
    foreach (Contact contact, m_contacts) {
        str += QString("%1<%2>; ").arg(contact.name).arg(contact.phonenum);
    }
    ui->textEditReceiver->setText(str);        
}

void DialogWriteMsg::on_checkBoxManualInputNum_clicked(bool checked)
{
    //如果选择手动输入，则清空选择
    qDebug() << checked << __FILE__ << __LINE__;
    ui->treeWidget->setEnabled(!checked);
    if (checked) {
        for (int i=0; i<=Contact::Type_Stranger; i++) {
            QTreeWidgetItem *contactTopItem = m_contactTypeItems.at(i);
            for (int i=contactTopItem->childCount()-1; i>=0; i--) {
                QTreeWidgetItem *child = contactTopItem->child(i);
                if (child->checkState(0) == Qt::Checked) {
                    child->setCheckState(0, Qt::Unchecked);
                }
            }
        }
        m_contacts.clear();
    }    
    ui->textEditReceiver->clear();
    ui->textEditReceiver->setReadOnly(!checked);
}

void DialogWriteMsg::on_textEditReceiver_textChanged()
{
    int maxHeight = ui->textEditReceiver->document()->size().height();
    ui->textEditReceiver->setMaximumHeight(maxHeight>0 ? maxHeight : cons_textedit_min_height);    
}

void DialogWriteMsg::on_textEditTip_textChanged()
{
    int maxHeight = ui->textEditTip->document()->size().height();
    ui->textEditTip->setMaximumHeight(maxHeight>0 ? maxHeight : cons_textedit_min_height);    
}
