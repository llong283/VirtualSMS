#include "dialogaddidentification.h"
#include "ui_dialogaddidentification.h"

DialogAddIdentification::DialogAddIdentification(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAddIdentification)
{
    ui->setupUi(this);
    
    //º”‘ÿqss
    QFile file(":/qss/Dialog.qss");
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << file.errorString() << __FILE__ << __LINE__;
    } else {
        setStyleSheet(file.readAll());
    } 
    file.close();    
    
    QRegExp rx("[1-9]\\d{0,10}");
    m_regValid.setRegExp(rx);   

    m_labelKeys << ui->labelKey1 << ui->labelKey2 << ui->labelKey3;
    m_lineEditKeys << ui->lineEditKey1 << ui->lineEditKey2 << ui->lineEditKey3;
}

DialogAddIdentification::~DialogAddIdentification()
{
    delete ui;
}

void DialogAddIdentification::init(const QList<QString> &keys, Type type)
{
    if (keys.count() < 3 || keys.count() > 4) {
        qDebug() << "Too much or too less keys" << __FILE__ << __LINE__;
        return;
    } else if (keys.count() == 3) {
        m_labelKeys.last()->hide();
        m_lineEditKeys.last()->hide();
        m_labelKeys.removeLast();
        m_lineEditKeys.removeLast();
    }
    m_keys = keys;
    m_type = type;
    for (int i=m_labelKeys.count()-1; i>=0; i--) {
        m_labelKeys[i]->setText(m_keys[i]);
        if (m_keys[i].contains(tr("Number"))) {
            m_lineEditKeys[i]->setValidator(&m_regValid);
        }
    }
    m_lineEditKeys.last()->setText("0");
    m_lineEditKeys.last()->setReadOnly(true);
}

void DialogAddIdentification::on_btnOK_clicked()
{
    QMap<QString, QString> map;
    for (int i=m_lineEditKeys.count()-1; i>=0; i--) {
        map.insert(m_keys[i], m_lineEditKeys[i]->text());
    }
    map.insert(tr("Handle"), ui->comboBoxHandle->currentText());
    qDebug() << map << __FILE__ << __LINE__;
    emit signAdd(map, m_type);
    close();
}

void DialogAddIdentification::on_btnCancel_clicked()
{
    close();
}
