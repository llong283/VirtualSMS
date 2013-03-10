#ifndef DIALOGCONTACT_H
#define DIALOGCONTACT_H

#include <QDialog>
#include "config.h"

namespace Ui {
    class DialogContact;
}

class DialogContact : public QDialog
{
    Q_OBJECT

public:
    explicit DialogContact(QWidget *parent = 0);
    ~DialogContact();
    void init(const Contact &contact=Contact());

protected:
    void changeEvent(QEvent *e);
    
signals:
    
private slots:
    void on_btnOK_clicked();    
    void on_btnCancel_clicked();
    
private:
    Ui::DialogContact *ui;
    Contact m_contact;
    QRegExpValidator m_regValid;
    
    void retranslateUi();
    bool checkValidity();
    void showTip(const QString &strTip);
    void hideTip();
};

#endif // DIALOGCONTACT_H
