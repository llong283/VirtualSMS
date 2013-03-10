/************************************************
类名: DialogContact
功能: 显示联系人信息或添加联系人
说明: 无
************************************************/

#include "dialogcontact.h"
#include "ui_dialogcontact.h"

#include "phonebook.h"
#include "udpbroadcast.h"

const int cons_phonenum_max_length = 11;

DialogContact::DialogContact(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogContact)
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
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
    ui->btnOK->setDefault(true);
    ui->labelTip->hide();
    
    QRegExp rx("[1-9]\\d{0,10}");
    m_regValid.setRegExp(rx);
    ui->lineEditPhoneNum->setValidator(&m_regValid);
    
    retranslateUi();
}

DialogContact::~DialogContact()
{
    delete ui;
}

void DialogContact::changeEvent(QEvent *e)
{
    if (e->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        retranslateUi();
        //combox控件会复位，需要重新加载
        if (m_contact.isValid()) {
            ui->comboBoxType->setCurrentIndex(m_contact.type);
        }        
    }
    QDialog::changeEvent(e);
}

void DialogContact::init(const Contact &contact)
{
    m_contact = contact;
    if (m_contact.isValid()) {
        ui->lineEditName->setText(m_contact.name);
        ui->lineEditPhoneNum->setText(m_contact.getCurrentPhoneNum());
        ui->comboBoxType->setCurrentIndex(m_contact.type);
        if (m_contact.type==Contact::Type_Myself) {
            ui->comboBoxType->setEnabled(false);
        } else {
            ui->comboBoxType->removeItem(Contact::Type_Myself);
        }
        setWindowTitle(tr("Contact Info"));
    } else {
        if (!g_phoneBook->getMyself().isValid()) {
            ui->comboBoxType->setCurrentIndex(Contact::Type_Myself);
            ui->comboBoxType->setEnabled(false);
            setWindowTitle(tr("Add Myself"));
        } else {
            ui->comboBoxType->removeItem(Contact::Type_Myself);
            setWindowTitle(tr("Add Contact"));
            ui->lineEditPhoneNum->setText(contact.phonenum);
        }
    }
}

void DialogContact::retranslateUi()
{
    if (m_contact.isValid()) {
        setWindowTitle(tr("Contact Info"));
    } else {
        if (!g_phoneBook->getMyself().isValid()) {
            setWindowTitle(tr("Add Myself"));
        } else {
            setWindowTitle(tr("Add Contact"));
        }        
    }
}

bool DialogContact::checkValidity()
{
    QString strPhoneNum = ui->lineEditPhoneNum->text();
    if (ui->lineEditName->text().isEmpty()) {
        showTip(tr("Name can't be empty"));
        return false;
    } else if (strPhoneNum.isEmpty()) {
        showTip(tr("Phonenum can't be empty"));
        return false;
    } else {
        QRegExp regExpUnNum("\\D");
        int indexUnNum = regExpUnNum.indexIn(strPhoneNum);
        if (indexUnNum == -1) {
            if (strPhoneNum.length() > cons_phonenum_max_length) {
                showTip(tr("Phonenum length is larger than %1").arg(cons_phonenum_max_length));
                return false;
            } else {
                hideTip();
                return true;
            }
        } else {
            showTip(tr("Phonenum contains non-numeric character"));
            return false;
        }      
    }
}

void DialogContact::showTip(const QString &strTip)
{
    ui->labelTip->setText(strTip);
    ui->labelTip->show();
}

void DialogContact::hideTip()
{
    ui->labelTip->hide();
}

void DialogContact::on_btnOK_clicked()
{    
    if (!checkValidity()) {
        return;
    }
    if (g_udpbroadcast->checkConflict(ui->lineEditPhoneNum->text())) {
        showTip(tr("Phone number conflicts with someone in the network"));
    } else {
        int ret;
        if (m_contact.isValid()) {
            m_contact.name = ui->lineEditName->text();
            if (m_contact.type == Contact::Type_Myself) {
                m_contact.phonenum = ui->lineEditPhoneNum->text() + "#" + m_contact.getCurrentPhoneNum();
            } else {
                m_contact.phonenum = ui->lineEditPhoneNum->text();
            }
            m_contact.type = ui->comboBoxType->currentIndex();
            ret = g_phoneBook->modifyContact(m_contact.id, m_contact.name, m_contact.phonenum, (Contact::Type)m_contact.type);        
        } else {
            QString phoneNum = ui->lineEditPhoneNum->text();
            if (ui->comboBoxType->currentIndex() == Contact::Type_Myself) {
                phoneNum += "#" + ui->lineEditPhoneNum->text();
            }
            ret = g_phoneBook->addContact(ui->lineEditName->text(), 
                                    phoneNum,
                                    (Contact::Type)ui->comboBoxType->currentIndex());
        }
        qDebug() << ret << __FILE__ << __LINE__;
        if (ret) {
            close();
        }
    }
}

void DialogContact::on_btnCancel_clicked()
{
    close();
}
