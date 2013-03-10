#ifndef DIALOGCHOOSECONTACT_H
#define DIALOGCHOOSECONTACT_H

#include <QDialog>

#include "config.h"

namespace Ui {
    class DialogChooseContact;
}

class DialogChooseContact : public QDialog
{
    Q_OBJECT

public:
    explicit DialogChooseContact(const Contact::Type &chooseType, QWidget *parent = 0);
    ~DialogChooseContact();

signals:
    void signChooseContact(QList<Contact>, Contact::Type);
    
private slots:
    void on_btnOK_clicked();
    
    void on_btnCancel_clicked();
    
private:
    Ui::DialogChooseContact *ui;
    Contact::Type m_chooseType;
};

#endif // DIALOGCHOOSECONTACT_H
