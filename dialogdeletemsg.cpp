#include "dialogdeletemsg.h"
#include "ui_dialogdeletemsg.h"

DialogDeleteMsg::DialogDeleteMsg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogDeleteMsg)
{
    ui->setupUi(this);
    
    setAttribute(Qt::WA_DeleteOnClose);    
    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
    m_valid.setRange(0, 365);    
    ui->lineEdit->setValidator(&m_valid);
    ui->lineEdit->setEnabled(false);
    ui->radioButtonDeleteAll->setChecked(true);
}

DialogDeleteMsg::~DialogDeleteMsg()
{
    delete ui;
}

void DialogDeleteMsg::on_btnDelete_clicked()
{
    QDateTime dateTime = QDateTime::currentDateTime();
    if (ui->radioButtonDeleteBefore->isChecked()) {
        dateTime = dateTime.addDays(0 - ui->lineEdit->text().toInt());
    }
    emit signDeleteMsgsBefore(dateTime);
    close();
}

void DialogDeleteMsg::on_btnCancel_clicked()
{
    close();
}

void DialogDeleteMsg::on_radioButtonDeleteAll_clicked()
{
    ui->lineEdit->setEnabled(false);
}

void DialogDeleteMsg::on_radioButtonDeleteBefore_clicked()
{
    ui->lineEdit->setEnabled(true);    
}
