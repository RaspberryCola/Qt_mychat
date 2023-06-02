
#ifndef MYCHAT_LOGIN_H
#define MYCHAT_LOGIN_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMessageBox>
#include <QDebug>
#include "chatwidget.h"
#include <QCryptographicHash>
void sqliteInit();

QT_BEGIN_NAMESPACE
namespace Ui { class login; }
QT_END_NAMESPACE

class login : public QMainWindow {
Q_OBJECT

public:
    explicit login(QWidget *parent = nullptr);
    ~login() override;
private slots:
    void showMyChat();
    void loginBtnClicked();
    void registerBtnClicked();
private:
    Ui::login *ui;
    chatWidget *myChatWidget;


};


#endif //MYCHAT_LOGIN_H
