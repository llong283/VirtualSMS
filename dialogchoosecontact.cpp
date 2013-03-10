/************************************************
类名: DialogChooseContact
功能: 选择联系人界面
说明: 无
************************************************/

#include "dialogchoosecontact.h"
#include "ui_dialogchoosecontact.h"

#include "phonebook.h"

DialogChooseContact::DialogChooseContact(const Contact::Type &chooseType, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogChooseContact),
    m_chooseType(chooseType)
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

    ui->listWidgetContact->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->listWidgetContact->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->listWidgetContact->setSelectionMode(QAbstractItemView::MultiSelection);
    QList<Contact> contacts = g_phoneBook->getContactsOfType(Contact::Type_Other);
    foreach (Contact contact, contacts) {
        if (contact.type != Contact::Type_Myself) {
            QListWidgetItem *lwItem = new QListWidgetItem(contact.name, ui->listWidgetContact);
            lwItem->setData(Qt::UserRole, QVariant::fromValue<Contact>(contact));
            lwItem->setSizeHint(QSize(0, cons_other_item_height));
            ui->listWidgetContact->addItem(lwItem);
        }
    }    
    if (m_chooseType == Contact::Type_BlackList) {
        setWindowTitle(tr("Choose blacklist"));
    } else {
        setWindowTitle(tr("Choose whitelist"));
    }
    setAttribute(Qt::WA_DeleteOnClose);
}

DialogChooseContact::~DialogChooseContact()
{
    delete ui;
}

void DialogChooseContact::on_btnOK_clicked()
{
    QList<Contact> contacts;
    QList<QListWidgetItem*> items = ui->listWidgetContact->selectedItems();
    foreach (QListWidgetItem *item, items) {
        contacts.append(item->data(Qt::UserRole).value<Contact>());
    }
    emit signChooseContact(contacts, m_chooseType);
    close();
}

void DialogChooseContact::on_btnCancel_clicked()
{
    close();
}
