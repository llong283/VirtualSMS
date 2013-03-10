/************************************************
类名: DialogSetting
功能: 设置界面
说明: 无
************************************************/

#include "dialogsetting.h"
#include "ui_dialogsetting.h"

#include <QHostAddress>
#include "phonebook.h"
#include "setting.h"
#include "dialogchoosecontact.h"
#include "QtSpeech.h"

#include "dialogaddidentification.h"

enum TableIndex {
    TableIndex_Content,
    TableIndex_Number,
    TableIndex_NumberSegment
};

enum TableContent {
    TableContent_Keyword,
    TableContent_Time,
    TableContent_Handle
};
enum TableNumber {
    TableNumber_Number,
    TableNumber_Time,
    TableNumber_Handle
};
enum TableNumberSegment {
    TableNumberSegment_StartNumber,
    TableNumberSegment_EndNumber,
    TableNumberSegment_Time,
    TableNumberSegment_Handle
};

DialogSetting::DialogSetting(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSetting)
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
    
    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
    setAttribute(Qt::WA_DeleteOnClose);
    
    ui->tabWidget->setCurrentIndex(0);
    ui->listWidgetBlackList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->listWidgetBlackList->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->listWidgetBlackList->setSelectionMode(QAbstractItemView::MultiSelection);
    connect(ui->listWidgetBlackList, SIGNAL(clicked(QModelIndex)), this, SLOT(slotListWidgetClicked()));
    ui->listWidgetWhiteList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->listWidgetWhiteList->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->listWidgetWhiteList->setSelectionMode(QAbstractItemView::MultiSelection);
    connect(ui->listWidgetWhiteList, SIGNAL(clicked(QModelIndex)), this, SLOT(slotListWidgetClicked()));
    
    m_identifyTables << ui->tableWidgetContent << ui->tableWidgetNumber << ui->tableWidgetNumberSegment;
    m_identifyContentHeads << tr("Content") <<  tr("Time") << tr("Handle");
    m_identifyNumberSegmentHeads << tr("StartNumber") << tr("EndNumber") << tr("Time") << tr("Handle");
    m_identifyNumberHeads << tr("Number") << tr("Time") << tr("Handle");
    m_identifyHeads << m_identifyContentHeads << m_identifyNumberHeads << m_identifyNumberSegmentHeads;   
    for (int i=0; i<3; i++) {
        m_identifyTables[i]->setColumnCount(m_identifyHeads[i].count());
        m_identifyTables[i]->setHorizontalHeaderLabels(m_identifyHeads[i]);
        m_identifyTables[i]->verticalHeader()->hide();
        m_identifyTables[i]->setSelectionMode(QAbstractItemView::SingleSelection);
        m_identifyTables[i]->setSelectionBehavior(QAbstractItemView::SelectRows);
        m_identifyTables[i]->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_identifyTables[i]->setSortingEnabled(true);
        connect(m_identifyTables[i], SIGNAL(clicked(QModelIndex)), this, SLOT(slotIdentifyTableClicked(QModelIndex)));
    }
    
    m_settingFileName = QApplication::applicationDirPath() + "/other/setting.ini";
    m_keywordFileName = QApplication::applicationDirPath() + "/other/key.txt";
    
    readSetting();
    updateMyselfInfo();
    updateBlackWhiteList();
    updateFilterKeyword();
    readIdentify();
    
    ui->lineEditName->setFocus();
    QRegExp rx("[1-9]\\d{0,10}");
    m_regValid.setRegExp(rx);
    ui->lineEditPhoneNum->setValidator(&m_regValid);
    
    //应用按钮暂不使用
    ui->btnApply->hide();
//    CancelFocus(this);
}

DialogSetting::~DialogSetting()
{
    delete ui;
}

void DialogSetting::changeEvent(QEvent *e)
{
    if (e->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        //combox控件会复位，需要重新加载
        readSetting();
    }
    QDialog::changeEvent(e);
}

void DialogSetting::CancelFocus(QWidget *w)
{
    if(!w) {
        return;
    }
    QObjectList list = w->children();
    foreach(QObject *obj, list) {
        if(!obj->isWidgetType()) {
            continue;
        }
        CancelFocus((QWidget*)obj);
    }
    w->setFocusPolicy(Qt::NoFocus);
}

void DialogSetting::slotListWidgetClicked()
{
    QListWidget *listWidget;
    if (ui->listWidgetBlackList->hasFocus()) {
        listWidget = ui->listWidgetWhiteList;
    } else {
        listWidget = ui->listWidgetBlackList;
    }
    QList<QListWidgetItem*> items = listWidget->selectedItems();
    foreach (QListWidgetItem *lwItem, items) {
        listWidget->setItemSelected(lwItem, false);
    }
}

void DialogSetting::slotIdentifyTableClicked(QModelIndex index)
{
    QTableWidget *focusedTable = getFocusedIdentifyTable();
    //取消当前表格以外的其他表格的选择
    if (focusedTable) {
        foreach (QTableWidget *tableWidget, m_identifyTables) {
            if (tableWidget != focusedTable) {
                QList<QTableWidgetItem*> items = tableWidget->selectedItems();
                foreach (QTableWidgetItem *item, items) {
                    item->setSelected(false);
                }
                tableWidget->setCurrentItem(0);
            }
        }
    }
    if (index.column() == focusedTable->columnCount()-1) {
        QTableWidgetItem *twItem = focusedTable->item(index.row(), index.column());
        IdentificationState state = identifyStateFromString(twItem->text());
        qDebug() << "state" << state << __FILE__ << __LINE__;
        switch (state) {
        case Allow:
            twItem->setText(identifyStateToString(Refuse));
            break;
        case Refuse:
            twItem->setText(identifyStateToString(Caution));
            break;
        case Caution:
            twItem->setText(identifyStateToString(Allow));
            break;
        default:
            break;
        }
        state = identifyStateFromString(twItem->text());
        int tableIndex = m_identifyTables.indexOf(focusedTable);
        switch (tableIndex) {
        case TableIndex_Content:
            g_MessageIdentification->setWordPropety(focusedTable->item(index.row(), TableContent_Keyword)->text(),
                                                    state);
            break;
        case TableIndex_Number:
            g_PhoneNumberIdentification->setNumberPropety(
                        focusedTable->item(index.row(), TableNumber_Number)->text(),
                        state);
            break;
        case TableIndex_NumberSegment:
            g_NumberSegmentIdentification->setNumberSegmentPropety(
                        focusedTable->item(index.row(), TableNumberSegment_StartNumber)->text().toULongLong(),
                        focusedTable->item(index.row(), TableNumberSegment_EndNumber)->text().toULongLong(),
                        state);
            break;
        default:
            break;
        }
    }
}

QTableWidget* DialogSetting::getFocusedIdentifyTable()
{
    QTableWidget *focusedTable(NULL);
    foreach (QTableWidget *tableWidget, m_identifyTables) {
        if (tableWidget->hasFocus()) {
            focusedTable = tableWidget;
            break;
        }
    }    
    return focusedTable;
}

void DialogSetting::readSetting()
{
    int filterMode = g_setting->getFilterMode();
    switch (filterMode) {
    case FM_Unused:
        ui->radioButtonUnUsed->setChecked(true);
        break;
    case FM_Blacklist:
        ui->radioButtonBlackList->setChecked(true);
        break;
    case FM_Whitelist:
        ui->radioButtonWhiteList->setChecked(true);
        break;
    default:
        qDebug() << "this is a bug" << __FILE__ << __LINE__;
        break;
    }
    ui->groupBoxContent->setChecked(g_setting->getIsIdentifyKeyword());
    ui->groupBoxNumber->setChecked(g_setting->getIsIdentifyNumber());
    ui->groupBoxNumberSegment->setChecked(g_setting->getIsIdentifyNumberSegment());
    
    ui->comboBoxLanguage->setCurrentIndex(g_setting->getLanguage());
    ui->comboBoxRunMode->setCurrentIndex(g_setting->getRunMode());
    ui->checkBoxCloseMin->setChecked(g_setting->getIsCloseMin());   
    
    int speakLanguage = g_setting->getSpeakLanguage();
    ui->comboBoxSpeechLanguage->setCurrentIndex(speakLanguage);
    QList<VoiceInfo> voices;
    if (speakLanguage == Speak_Chinese) {        
        voices = g_speech->getChineseVoices();
    } else {
        voices = g_speech->getEnglishVoices();
    }
    foreach(VoiceInfo v, voices) {
        ui->comboBoxVoices->addItem(v.name, v.id);        
    }            
    int voiceIndex = ui->comboBoxVoices->findText(g_setting->getVoiceInfo().name);
    ui->comboBoxVoices->setCurrentIndex(voiceIndex);
    qDebug() << voiceIndex << g_setting->getVoiceInfo().name << __FILE__ << __LINE__;
}

bool DialogSetting::writeSetting()
{
    QSettings settings(m_settingFileName, QSettings::IniFormat);
    settings.clear();
    settings.beginGroup("Filter");
    if (ui->radioButtonBlackList->isChecked()) {
        settings.setValue("FilterMode", FM_Blacklist);
    } else if (ui->radioButtonWhiteList->isChecked()) {
        settings.setValue("FilterMode", FM_Whitelist);
    } else {
        settings.setValue("FilterMode", FM_Unused);
    }
    settings.endGroup();
    settings.beginGroup("Identify");
    settings.setValue("IdentifyKeyword", ui->groupBoxContent->isChecked());
    settings.setValue("IdentifyNumber", ui->groupBoxNumber->isChecked());
    settings.setValue("IdentifyNumberSegment", ui->groupBoxNumberSegment->isChecked());
    settings.endGroup();
    settings.beginGroup("Other");
    settings.setValue("Language", ui->comboBoxLanguage->currentIndex());
    settings.setValue("RunMode", ui->comboBoxRunMode->currentIndex());
    settings.setValue("CloseMin", ui->checkBoxCloseMin->isChecked());
    settings.setValue("SpeakLanguage", ui->comboBoxSpeechLanguage->currentIndex());
    settings.setValue("VoiceId", ui->comboBoxVoices->itemData(ui->comboBoxVoices->currentIndex()));
    settings.setValue("VoiceName", ui->comboBoxVoices->currentText());
    settings.endGroup();
    return true;
}

void DialogSetting::updateMyselfInfo()
{
    Contact contact = g_phoneBook->getMyself();
    if (contact.isValid()) {
        ui->lineEditName->setText(contact.name);
        ui->lineEditPhoneNum->setText(contact.getCurrentPhoneNum());
    } else {
        qDebug() << "Myself is not in phonebook!" << __FILE__ << __LINE__;
    }
}

void DialogSetting::updateBlackWhiteList()
{
    ui->listWidgetBlackList->clear();
    ui->listWidgetWhiteList->clear();
    QList<Contact> blackList = g_phoneBook->getContactsOfType(Contact::Type_BlackList);
    foreach (Contact contact, blackList) {
        QListWidgetItem *lwItem = new QListWidgetItem;
        lwItem->setText(QString("%1(%2)").arg(contact.name).arg(contact.phonenum));
        lwItem->setData(Qt::UserRole, QVariant::fromValue<Contact>(contact));
        lwItem->setSizeHint(QSize(0, cons_other_item_height));
        ui->listWidgetBlackList->addItem(lwItem);
    }
    QList<Contact> whiteList = g_phoneBook->getContactsOfType(Contact::Type_WhiteList);
    foreach (Contact contact, whiteList) {
        QListWidgetItem *lwItem = new QListWidgetItem;
        lwItem->setText(QString("%1(%2)").arg(contact.name).arg(contact.phonenum));
        lwItem->setData(Qt::UserRole, QVariant::fromValue<Contact>(contact));
        lwItem->setSizeHint(QSize(0, cons_other_item_height));
        ui->listWidgetWhiteList->addItem(lwItem);
    }
}

void DialogSetting::updateFilterKeyword()
{
    QFile file(m_keywordFileName);
    if (!file.open(QFile::ReadOnly)) {
        qDebug() << file.errorString() << __FILE__ << __LINE__;
        return;
    }
    ui->textEditKeyword->setText(file.readAll());
    file.close();
}

bool DialogSetting::saveKeyword()
{
    QFile file(m_keywordFileName);
    if (!file.open(QFile::WriteOnly)) {
        qDebug() << file.errorString() << __FILE__ << __LINE__;
        return false;
    }
    file.write(ui->textEditKeyword->toPlainText().toAscii());
    file.close();   
    return true;
}

bool DialogSetting::modifyMyselfInfo()
{
    Contact contact = g_phoneBook->getMyself();
    bool flag;
    if (contact.isValid()) {
        flag = g_phoneBook->modifyContact(contact.id, 
                                          ui->lineEditName->text(), 
                                          ui->lineEditPhoneNum->text() + "#" + contact.getCurrentPhoneNum(), 
                                          Contact::Type_Myself);
    } else {
        int id = g_phoneBook->addContact(ui->lineEditName->text(),
                                         ui->lineEditPhoneNum->text() + "#" + ui->lineEditPhoneNum->text(),
                                         Contact::Type_Myself);
        flag = (id==-1) ? false : true;
    }
    return flag;
}

bool DialogSetting::save()
{
    if (!writeSetting() || !saveKeyword() || !modifyMyselfInfo()) {
        return false;
    } else {
        return true;
    }
}

void DialogSetting::slotChooseContact(QList<Contact> contacts, Contact::Type chooseType)
{
    foreach (Contact contact, contacts) {
        if (!g_phoneBook->changeType(contact.id, chooseType)) {
            qDebug() << "changeType failed. id:" << contact.id << __FILE__ << __LINE__;
        }
    }
    updateBlackWhiteList();
}

void DialogSetting::processContactChange(QList<QListWidgetItem *> items, Contact::Type dstType)
{
    foreach (QListWidgetItem *item, items) {
        Contact contact = item->data(Qt::UserRole).value<Contact>();
        g_phoneBook->changeType(contact.id, dstType);
    }    
    updateBlackWhiteList();    
}

QString DialogSetting::identifyStateToString(IdentificationState state)
{
    QString strState;
    switch (state) {
    case Allow:
        strState = tr("Allow");
        break;
    case Refuse:
        strState = tr("Refuse");
        break;
    case Caution:
        strState = tr("Caution");
    default:
        break;
    }
    return strState;
}

IdentificationState DialogSetting::identifyStateFromString(const QString &strState)
{
    IdentificationState state;
    if (strState == tr("Allow")) {
        state = Allow;
    } else if (strState == tr("Refuse")) {
        state = Refuse;
    } else if (strState == tr("Caution")) {
        state = Caution;
    }
    return state;
}

void DialogSetting::slotAddIdentification(const QMap<QString, QString> &map, int type)
{
    switch (type) {
    case DialogAddIdentification::Type_AddKeyword:
    {
        qDebug() << "add keyword" << m_identifyContentHeads << __FILE__ << __LINE__;
        MIInformation miinfo;
        miinfo.keyWord = map.value(m_identifyContentHeads.at(0));
        miinfo.count = map.value(m_identifyContentHeads.at(1)).toInt();
        miinfo.state = identifyStateFromString(map.value(m_identifyContentHeads.at(2)));
        g_MessageIdentification->setWordPropety(miinfo.keyWord, miinfo.state);
        addIdentifyKeyword(miinfo);
    }
        break;
    case DialogAddIdentification::Type_AddNumber:
    {
        PNIInformation pniinfo;
        pniinfo.number = map.value(m_identifyNumberHeads.at(0));
        pniinfo.count = map.value(m_identifyNumberHeads.at(1)).toInt();
        pniinfo.state = identifyStateFromString(map.value(m_identifyNumberHeads.at(2)));
        g_PhoneNumberIdentification->setNumberPropety(pniinfo.number, pniinfo.state);
        addIdentifyNumber(pniinfo);     
    }
        break;
    case DialogAddIdentification::Type_AddNumberSegment:
    {
        NumberSegmentInformation numseg;
        numseg.startNumber = map.value(m_identifyNumberSegmentHeads.at(0)).toULongLong();
        numseg.endNumber = map.value(m_identifyNumberSegmentHeads.at(1)).toULongLong();
        numseg.count = map.value(m_identifyNumberSegmentHeads.at(2)).toInt();
        numseg.state = identifyStateFromString(map.value(m_identifyNumberSegmentHeads.at(3)));
        if (g_NumberSegmentIdentification->setNumberSegmentPropety(numseg.startNumber, 
                                                                   numseg.endNumber,
                                                                   numseg.state)) {
            addIdentifyNumberSegment(numseg);
        } else {
            QMessageBox::information(this, tr("msg"), tr("number segment is not valid. add fail"));
        }
    }
        break;
    default:
        break;
    }
}

void DialogSetting::readIdentify()
{   
    QList<MIInformation> miinfos = g_MessageIdentification->getShowList();
    foreach (MIInformation miinfo, miinfos) {
        addIdentifyKeyword(miinfo);
    }        
    QList<PNIInformation> pniinfos = g_PhoneNumberIdentification->getShowList();
    foreach (PNIInformation pniinfo, pniinfos) {
        addIdentifyNumber(pniinfo);
    }        
    QList<NumberSegmentInformation> numbersegs = g_NumberSegmentIdentification->getShowList();
    foreach (NumberSegmentInformation numberseg, numbersegs) {
        addIdentifyNumberSegment(numberseg);
    }
}

void DialogSetting::addIdentifyKeyword(MIInformation miiinfo)
{
    int row = ui->tableWidgetContent->rowCount();
    ui->tableWidgetContent->insertRow(row);
    ui->tableWidgetContent->setItem(row, TableContent_Keyword, new QTableWidgetItem(miiinfo.keyWord));
    ui->tableWidgetContent->setItem(row, TableContent_Time, new QTableWidgetItem(QString::number(miiinfo.count)));
    ui->tableWidgetContent->setItem(row, TableContent_Handle, new QTableWidgetItem(identifyStateToString(miiinfo.state)));
    QTableWidgetItem *twItem = ui->tableWidgetContent->item(row, TableContent_Handle);
    QFont font("宋体", 9);
    font.setUnderline(true);
    twItem->setFont(font);                
    twItem->setForeground(QBrush(QColor(Qt::blue)));
    twItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->tableWidgetContent->setRowHeight(row, 20);
}

void DialogSetting::addIdentifyNumber(PNIInformation pniinfo)
{
    int row = ui->tableWidgetNumber->rowCount();
    ui->tableWidgetNumber->insertRow(row);
    ui->tableWidgetNumber->setItem(row, TableNumber_Number, new QTableWidgetItem(pniinfo.number));
    ui->tableWidgetNumber->setItem(row, TableNumber_Time, new QTableWidgetItem(QString::number(pniinfo.count)));
    ui->tableWidgetNumber->setItem(row, TableNumber_Handle, new QTableWidgetItem(identifyStateToString(pniinfo.state)));
    QTableWidgetItem *twItem = ui->tableWidgetNumber->item(row, TableNumber_Handle);
    QFont font("宋体", 9);
    font.setUnderline(true);
    twItem->setFont(font);                
    twItem->setForeground(QBrush(QColor(Qt::blue)));
    twItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);    
    ui->tableWidgetNumber->setRowHeight(row, 20);
}

void DialogSetting::addIdentifyNumberSegment(NumberSegmentInformation nsinfo)
{
    qDebug() << "add number segment" << nsinfo.startNumber << nsinfo.endNumber << nsinfo.state << __FILE__ << __LINE__;
    int row = ui->tableWidgetNumberSegment->rowCount();
    ui->tableWidgetNumberSegment->insertRow(row);
    ui->tableWidgetNumberSegment->setItem(row, TableNumberSegment_StartNumber, new QTableWidgetItem(QString::number(nsinfo.startNumber)));
    ui->tableWidgetNumberSegment->setItem(row, TableNumberSegment_EndNumber, new QTableWidgetItem(QString::number(nsinfo.endNumber)));
    ui->tableWidgetNumberSegment->setItem(row, TableNumberSegment_Time, new QTableWidgetItem(QString::number(nsinfo.count)));
    ui->tableWidgetNumberSegment->setItem(row, TableNumberSegment_Handle, new QTableWidgetItem(identifyStateToString(nsinfo.state)));
    QTableWidgetItem *twItem = ui->tableWidgetNumberSegment->item(row, TableNumberSegment_Handle);
    QFont font("宋体", 9);
    font.setUnderline(true);
    twItem->setFont(font);                
    twItem->setForeground(QBrush(QColor(Qt::blue)));
    twItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);        
    ui->tableWidgetNumberSegment->setRowHeight(row, 20);
}

void DialogSetting::on_btnConfirm_clicked()
{
    modifyMyselfInfo();
    writeSetting();
    saveKeyword();
    g_setting->updateSetting();
    close();
}

void DialogSetting::on_btnApply_clicked()
{
    if (!save()) {
        return;
    }
    g_setting->updateSetting();
}

void DialogSetting::on_btnCancel_clicked()
{
    close();
}

void DialogSetting::on_btnAddBlackList_clicked()
{    
    DialogChooseContact *dcContact = new DialogChooseContact(Contact::Type_BlackList);
    connect(dcContact, SIGNAL(signChooseContact(QList<Contact>, Contact::Type)),
            this, SLOT(slotChooseContact(QList<Contact>, Contact::Type)));
    dcContact->exec();
}

void DialogSetting::on_btnMoveToWhitelist_clicked()
{
    QList<QListWidgetItem*> items = ui->listWidgetBlackList->selectedItems();
    processContactChange(items, Contact::Type_WhiteList);
}

void DialogSetting::on_btnRemoveBlackList_clicked()
{
    QList<QListWidgetItem*> items = ui->listWidgetBlackList->selectedItems();
    processContactChange(items, Contact::Type_Other);
}

void DialogSetting::on_btnAddWhiteList_clicked()
{
    DialogChooseContact *dcContact = new DialogChooseContact(Contact::Type_WhiteList);
    connect(dcContact, SIGNAL(signChooseContact(QList<Contact>, Contact::Type)),
            this, SLOT(slotChooseContact(QList<Contact>, Contact::Type)));
    dcContact->exec();    
}

void DialogSetting::on_btnMoveToBlacklist_clicked()
{
    QList<QListWidgetItem*> items = ui->listWidgetWhiteList->selectedItems();
    processContactChange(items, Contact::Type_BlackList);    
}

void DialogSetting::on_btnRemoveWhiteList_clicked()
{
    QList<QListWidgetItem*> items = ui->listWidgetWhiteList->selectedItems();
    processContactChange(items, Contact::Type_Other);        
}

void DialogSetting::on_btnAddContent_clicked()
{
    DialogAddIdentification *dAdd = new DialogAddIdentification;
    connect(dAdd, SIGNAL(signAdd(QMap<QString,QString>, int)), 
            this, SLOT(slotAddIdentification(QMap<QString,QString>,int)));
    dAdd->init(m_identifyContentHeads, DialogAddIdentification::Type_AddKeyword);
    dAdd->exec();
}

void DialogSetting::on_btnRemoveContent_clicked()
{
    int row = ui->tableWidgetContent->currentRow();
    if (row != -1) {
        g_MessageIdentification->setWordPropety(ui->tableWidgetContent->item(row, TableContent_Keyword)->text(),
                                                Delete);
        ui->tableWidgetContent->removeRow(row);
    }
}

void DialogSetting::on_btnAddNumber_clicked()
{
    DialogAddIdentification *dAdd = new DialogAddIdentification;
    connect(dAdd, SIGNAL(signAdd(QMap<QString,QString>, int)), 
            this, SLOT(slotAddIdentification(QMap<QString,QString>,int)));
    dAdd->init(m_identifyNumberHeads, DialogAddIdentification::Type_AddNumber);
    dAdd->exec();    
}

void DialogSetting::on_btnRemoveNumber_clicked()
{
    int row = ui->tableWidgetNumber->currentRow();
    if (row != -1) {
        g_PhoneNumberIdentification->setNumberPropety(ui->tableWidgetNumber->item(row, TableNumber_Number)->text(),
                                                      Delete);
        ui->tableWidgetNumber->removeRow(row);
    }    
}

void DialogSetting::on_btnAddNumberSegment_clicked()
{
    DialogAddIdentification *dAdd = new DialogAddIdentification;
    connect(dAdd, SIGNAL(signAdd(QMap<QString,QString>, int)), 
            this, SLOT(slotAddIdentification(QMap<QString,QString>,int)));
    dAdd->init(m_identifyNumberSegmentHeads, DialogAddIdentification::Type_AddNumberSegment);
    dAdd->exec();    
}

void DialogSetting::on_btnRemoveNumberSegment_clicked()
{
    int row = ui->tableWidgetNumberSegment->currentRow();
    if (row != -1) {
        g_NumberSegmentIdentification->setNumberSegmentPropety(
                    ui->tableWidgetNumberSegment->item(row, TableNumberSegment_StartNumber)->text().toULongLong(),
                    ui->tableWidgetNumberSegment->item(row, TableNumberSegment_EndNumber)->text().toULongLong(),
                    Delete);
        ui->tableWidgetNumberSegment->removeRow(row);
    }        
}

void DialogSetting::on_btnAddToCommon_clicked()
{
    qDebug() << "add to common" << __FILE__ << __LINE__;
    QList<QTableWidgetItem*> items = ui->tableWidgetContent->selectedItems();
    int row = ui->tableWidgetContent->row(items.at(0));
    qDebug() << row << items.at(0)->text();
    if (row != -1) {
        qDebug() << "add to common" << __FILE__ << __LINE__;
        QString keyword = ui->tableWidgetContent->item(row, TableContent_Keyword)->text();
        if (!g_MessageIdentification->addCommonWord(keyword)) {
            QMessageBox::information(this, tr("msg"), tr("keyword exist"));
        }
    }
}

void DialogSetting::on_comboBoxSpeechLanguage_currentIndexChanged(int index)
{
    if (index == Speak_Chinese && g_speech->getChineseVoices().isEmpty()) {
        QMessageBox::information(this, tr("msg"), tr("Your computer currently doesn't support speak chinese. Please install relative software"));
        ui->comboBoxSpeechLanguage->setCurrentIndex(Speak_English);
    }
    ui->comboBoxVoices->clear();
    QList<VoiceInfo> voices;
    if (index == Speak_Chinese) {        
        voices = g_speech->getChineseVoices();
    } else {
        voices = g_speech->getEnglishVoices();
    }
    foreach(VoiceInfo v, voices) {
        ui->comboBoxVoices->addItem(v.name, v.id);        
    }
    ui->comboBoxVoices->setCurrentIndex(0);
}
