#ifndef DIALOGSETTING_H
#define DIALOGSETTING_H

#include <QDialog>
#include "config.h"
#include "MessageIdentification.h"
#include "phonenumberidentification.h"
#include "numbersegmentidentification.h"

namespace Ui {
    class DialogSetting;
}

class DialogSetting : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSetting(QWidget *parent = 0);
    ~DialogSetting();  
    
protected:
    void changeEvent(QEvent *e);
    
signals:
    
private slots:
    void slotChooseContact(QList<Contact> contacts, Contact::Type chooseType);
    void slotListWidgetClicked();
    void slotAddIdentification(const QMap<QString, QString> &map, int type);
    void slotIdentifyTableClicked(QModelIndex index);
        
    void on_btnConfirm_clicked();    
    void on_btnApply_clicked();    
    void on_btnCancel_clicked();        
    void on_btnAddBlackList_clicked();    
    void on_btnMoveToWhitelist_clicked();    
    void on_btnRemoveBlackList_clicked();    
    void on_btnAddWhiteList_clicked();    
    void on_btnMoveToBlacklist_clicked();    
    void on_btnRemoveWhiteList_clicked();         
    void on_btnAddContent_clicked();    
    void on_btnAddNumber_clicked();    
    void on_btnRemoveContent_clicked();    
    void on_btnRemoveNumber_clicked();    
    void on_btnAddNumberSegment_clicked();    
    void on_btnRemoveNumberSegment_clicked();    
    void on_btnAddToCommon_clicked();
    void on_comboBoxSpeechLanguage_currentIndexChanged(int index);
    
private:
    Ui::DialogSetting *ui;
    QString m_settingFileName;
    QString m_keywordFileName;
    QListWidget *m_listWidget;
    QList<QTableWidget*> m_identifyTables;
    QStringList m_identifyContentHeads;
    QStringList m_identifyNumberHeads;
    QStringList m_identifyNumberSegmentHeads;
    QList<QStringList> m_identifyHeads;
    QRegExpValidator m_regValid;
    
    bool save();
    void readSetting();
    bool writeSetting();
    bool saveKeyword();
    void readIdentify();
    bool saveIdentify();
    bool modifyMyselfInfo();
    void updateMyselfInfo();
    void updateBlackWhiteList();
    void updateFilterKeyword();
    void processContactChange(QList<QListWidgetItem*> items, Contact::Type dstType);
    void CancelFocus(QWidget *w);
    QString identifyStateToString(IdentificationState state);
    IdentificationState identifyStateFromString(const QString &strState);
    QTableWidget* getFocusedIdentifyTable();
    void addIdentifyKeyword(MIInformation miiinfo);
    void addIdentifyNumber(PNIInformation pniinfo);
    void addIdentifyNumberSegment(NumberSegmentInformation nsinfo);
};

#endif // DIALOGSETTING_H
