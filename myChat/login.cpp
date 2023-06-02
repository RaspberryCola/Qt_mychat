//
// Created by bo on 2023/3/23.
//

// You may need to build the project (run Qt uic code generator) to get "ui_login.h" resolved

#include "login.h"
#include "ui_login.h"


login::login(QWidget *parent) : QMainWindow(parent), ui(new Ui::login) {
    ui->setupUi(this);
    setWindowIcon(QPixmap("D:\\CLionProject\\Qt\\myChat\\images\\qq.png"));
    connect(ui->logInBtn, SIGNAL(clicked(bool)), this, SLOT(loginBtnClicked()));
    connect(ui->registerBtn, SIGNAL(clicked(bool)), this, SLOT(registerBtnClicked()));

    sqliteInit();
}

login::~login() {
    delete ui;
}
void login::showMyChat() {
    myChatWidget = new chatWidget(0, ui->usrLineEdit->text());
    myChatWidget->setWindowTitle(ui->usrLineEdit->text());
    myChatWidget->setWindowIcon(QPixmap("D:\\CLionProject\\Qt\\myChat\\images\\qq.png"));
    myChatWidget->show();
}


void sqliteInit() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("userHash.db");
    if (!db.open()) {
        qDebug() << "Filed to open";
    }
    QSqlQuery query;
    QString createsql = QString(
            "CREATE TABLE users (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT, passwordHash TEXT)");
    if (!query.exec(createsql)) {
        qDebug() << "table create error";
    }
//    else {
//        qDebug() << "table create success";
//    }
}

void login::loginBtnClicked() {
    sqliteInit();
    QString username = ui->usrLineEdit->text();
    QString password = ui->pswLineEdit->text();
    QByteArray passwordHash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    QSqlQuery query;
    query.prepare("SELECT * FROM users WHERE username = :username AND passwordHash = :passwordHash");
    query.bindValue(":username", username);
    query.bindValue(":passwordHash", passwordHash.toHex());
    query.exec();

    if (!query.next()) {
        qDebug() << "Login error";
        QMessageBox::warning(this, "登录认证", "账号或密码错误");
    } else {
        qDebug() << "Login success";
        QMessageBox::information(this, "登录认证", "登录成功");
        showMyChat();
        this->close();
    }
}

void login::registerBtnClicked() {
    sqliteInit();  // 调用sqliteInit()函数，初始化SQLite数据库

    QString username = ui->usrLineEdit->text();  // 获取用户名
    QString password = ui->pswLineEdit->text();  // 获取密码
    // 使用SHA-256哈希算法对密码进行哈希计算，生成哈希值
    QByteArray passwordHash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);

    QSqlQuery query;  // 创建QSqlQuery对象，用于执行SQL语句
    // 使用prepare()函数和bindValue()函数，执行SQL语句，将用户名和密码哈希值插入到user表中
    query.prepare("insert into users(username,passwordHash) values(:username,:passwordHash)");
    query.bindValue(":username", username);
    query.bindValue(":passwordHash", passwordHash.toHex());

    // 如果SQL语句执行失败，输出错误信息并弹出警告对话框
    if (!query.exec()) {
        qDebug() << "insert into error";
        QMessageBox::warning(this, "注册认证", "注册失败");
    } else {
        // 如果SQL语句执行成功，输出成功信息并弹出提示对话框
        qDebug() << "insert into success";
        QMessageBox::information(this, "注册认证", "注册成功");
    }
}

