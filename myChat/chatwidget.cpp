#include "chatwidget.h"
#include "ui_chatWidget.h"
#include "server.h"
#include "client.h"

#include <QUdpSocket>
#include <QHostInfo>
#include <QMessageBox>
#include <QScrollBar>
#include <QDateTime>
#include <QNetworkInterface>
#include <QProcess>
#include <QDebug>
#include <QFileDialog>
#include <QColorDialog>

chatWidget::chatWidget(QWidget *parent, QString usrname) : QWidget(parent), ui(new Ui::chatWidget) {
    ui->setupUi(this);
    initBtns();

    uName = usrname;
    udpSocket = new QUdpSocket(this);
    port = 8010;
    udpSocket->bind(port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    srv = new Server(this);

    sndMsg(UsrEnter);

    connect(udpSocket, SIGNAL(readyRead()), this, SLOT(processPendingDatagrams()));
    connect(ui->sendButton, SIGNAL(clicked(bool)), this, SLOT(sendBtnClicked()));
    connect(srv, SIGNAL(sndFileName(QString)), this, SLOT(getFileName(QString)));
    connect(ui->sendTBtn, SIGNAL(clicked(bool)), this, SLOT(sendTBtnClicked()));
    connect(ui->quitButton, SIGNAL(clicked(bool)), this, SLOT(quitBtnClicked()));
    connect(ui->fontCbx, SIGNAL(currentFontChanged(const QFont &)), this, SLOT(fontCbxChanged(const QFont &)));
    connect(ui->sizeCbx, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(fontSizeChanged(const QString &)));
    connect(ui->boldTBtn, SIGNAL(clicked(bool)), this, SLOT(boldTBtnClicked(bool)));
    connect(ui->italicTBtn, SIGNAL(clicked(bool)), this, SLOT(italicTBtnClicked(bool)));
    connect(ui->underlineTBtn, SIGNAL(clicked(bool)), this, SLOT(underlineTBtnClicked(bool)));
    connect(ui->colorTBtn, SIGNAL(clicked(bool)), this, SLOT(colorTBtnClicked()));
    connect(ui->saveTBtn, SIGNAL(clicked(bool)), this, SLOT(saveTBtnClicked()));
    connect(ui->clearTBtn, SIGNAL(clicked(bool)), this, SLOT(clearTBtnClicked()));


}


chatWidget::~chatWidget() {
    delete ui;
}

void chatWidget::initBtns() {
    ui->boldTBtn->setIcon(QPixmap("D:\\CLionProject\\Qt\\myChat\\images\\bold.png"));
    ui->boldTBtn->setIconSize(QPixmap("D:\\CLionProject\\Qt\\myChat\\images\\bold.png").size());
    ui->boldTBtn->setAutoRaise(true);

    ui->italicTBtn->setIcon(QPixmap("D:\\CLionProject\\Qt\\myChat\\images\\italic.png"));
    ui->italicTBtn->setIconSize(QPixmap("D:\\CLionProject\\Qt\\myChat\\images\\italic.png").size());
    ui->italicTBtn->setAutoRaise(true);

    ui->underlineTBtn->setIcon(QPixmap("D:\\CLionProject\\Qt\\myChat\\images\\under.png"));
    ui->underlineTBtn->setIconSize(QPixmap("D:\\CLionProject\\Qt\\myChat\\images\\under.png").size());
    ui->underlineTBtn->setAutoRaise(true);

    ui->colorTBtn->setIcon(QPixmap("D:\\CLionProject\\Qt\\myChat\\images\\color.png"));
    ui->colorTBtn->setIconSize(QPixmap("D:\\CLionProject\\Qt\\myChat\\images\\color.png").size());
    ui->colorTBtn->setAutoRaise(true);

    ui->sendTBtn->setIcon(QPixmap("D:\\CLionProject\\Qt\\myChat\\images\\send.png"));
    ui->sendTBtn->setIconSize(QPixmap("D:\\CLionProject\\Qt\\myChat\\images\\send.png").size());
    ui->sendTBtn->setAutoRaise(true);

    ui->saveTBtn->setIcon(QPixmap("D:\\CLionProject\\Qt\\myChat\\images\\save.png"));
    ui->saveTBtn->setIconSize(QPixmap("D:\\CLionProject\\Qt\\myChat\\images\\save.png").size());
    ui->saveTBtn->setAutoRaise(true);

    ui->clearTBtn->setIcon(QPixmap("D:\\CLionProject\\Qt\\myChat\\images\\clear.png"));
    ui->clearTBtn->setIconSize(QPixmap("D:\\CLionProject\\Qt\\myChat\\images\\clear.png").size());
    ui->clearTBtn->setAutoRaise(true);
}

//data = type + username + address(IP) + msg
void chatWidget::sndMsg(MsgType type, QString srvaddr /* =" " */) {
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    QString address = getIP();
    out << type << getUsr();

    switch (type) {
        case Msg:
            if (ui->msgTxtEdit->toPlainText() == "") {
                QMessageBox::warning(0, "警告", "发送内容不能为空", QMessageBox::Ok);
                return;
            }
            out << address << getMsg(); //向发送的数据中写入本机的IP地址和用户输入的文本信息
            ui->msgBrowser->verticalScrollBar()->setValue(ui->msgBrowser->verticalScrollBar()->maximum());
            break;
        case UsrEnter:
            out << address; //对于新用户加入，只需要简单的写入IP地址
            break;
        case UsrLeft:
            break;
        case FileName: {
            int row = ui->usrTblWidget->currentRow();
            QString clntName = ui->usrTblWidget->item(row, 0)->text();//adds
            QString clntaddr = ui->usrTblWidget->item(row, 1)->text();

            out << address << clntaddr << clntName << fileName; //adds
            //out << address << clntaddr << fileName; //origin
            break;
        }
        case Refuse:
            out << srvaddr;
            break;
    }
    //完成对信息的处理后，用writeDatagram()对信息进行广播
    udpSocket->writeDatagram(data, data.length(), QHostAddress::Broadcast, port);

}

void chatWidget::processPendingDatagrams() {
    //判断是否有数据可供读取
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(datagram.data(), datagram.size());
        QDataStream in(&datagram, QIODevice::ReadOnly);
        int msgType;
        in >> msgType;
        QString usrName, ipAddr, msg;
        QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        switch (msgType) {
            case Msg:
                in >> usrName >> ipAddr >> msg;
                ui->msgBrowser->setTextColor(Qt::blue);
                ui->msgBrowser->setCurrentFont(QFont("Times New Roman", 12));
                ui->msgBrowser->append("[" + usrName + "]" + time);
                ui->msgBrowser->append(msg);
                break;
            case UsrEnter:
                in >> usrName >> ipAddr;
                usrEnter(usrName, ipAddr);
                break;
            case UsrLeft:
                in >> usrName;
                usrLeft(usrName, time);
                break;
            case FileName: {
                in >> usrName >> ipAddr;  //adds
                QString clntAddr, clntName, fileName;//adds
                in >> clntAddr >> clntName >> fileName;
                if (clntAddr == getIP() && clntName == uName)
                    hasPendingFile(usrName, ipAddr, clntAddr, fileName);
//                in >> usrName >> ipAddr;
//                QString clntAddr, fileName;
//                in >> clntAddr >> fileName;
//                hasPendingFile(usrName, ipAddr, clntAddr, fileName);
                break;
            }
            case Refuse: {
                in >> usrName;
                QString srvAddr;
                in >> srvAddr;
                QString ipAddr = getIP();
                if (ipAddr == srvAddr) {
                    srv->refused();
                }
                break;
            }
        }
    }

}

void chatWidget::usrEnter(QString usrname, QString ipaddr) {
    bool isEmpty = ui->usrTblWidget->findItems(usrname, Qt::MatchExactly).isEmpty();
    if (isEmpty) {
        QTableWidgetItem *usr = new QTableWidgetItem(usrname);
        QTableWidgetItem *ip = new QTableWidgetItem(ipaddr);

        ui->usrTblWidget->insertRow(0);
        ui->usrTblWidget->setItem(0, 0, usr);
        ui->usrTblWidget->setItem(0, 1, ip);

        ui->msgBrowser->setTextColor(Qt::gray);
        ui->msgBrowser->setCurrentFont(QFont("Times New Roman", 10));
        ui->msgBrowser->append(tr("%1 在线！").arg(usrname));
        ui->usrNumLbl->setText(tr("在线人数：%1").arg(ui->usrTblWidget->rowCount()));
        sndMsg(UsrEnter);
    }
}
void chatWidget::usrLeft(QString usrname, QString time) {
    int rowNum = ui->usrTblWidget->findItems(usrname, Qt::MatchExactly).first()->row();
    ui->usrTblWidget->removeRow(rowNum);
    ui->msgBrowser->setTextColor(Qt::gray);
    ui->msgBrowser->setCurrentFont(QFont("Times New Roman", 10));
    ui->msgBrowser->append(usrname + " 于 " + time + " 离开！");
    ui->usrNumLbl->setText(tr("在线人数：%1").arg(ui->usrTblWidget->rowCount()));
}

QString chatWidget::getIP() {
    //获取本地主机名
    QString localHostName = QHostInfo::localHostName();
    //根据主机名获取相关主机信息，包括IP地址等
    QHostInfo hostInfo = QHostInfo::fromName(localHostName);
    //获得主机IP地址列表
    QList<QHostAddress> listAddress = hostInfo.addresses();
    if (!listAddress.isEmpty()) {
        for (int i = 0; i < listAddress.size(); i++) {
            if (listAddress[i].protocol() == QAbstractSocket::IPv4Protocol)
                return listAddress[i].toString();
        }
    }
}
QString chatWidget::getUsr() {
    return uName;
}
QString chatWidget::getMsg() {
    QString msg = ui->msgTxtEdit->toHtml();
    ui->msgTxtEdit->clear();
    ui->msgTxtEdit->setFocus();
    return msg;
}

void chatWidget::sendBtnClicked() {
    sndMsg(Msg);
}

void chatWidget::getFileName(QString name) {
    fileName = name;
    sndMsg(FileName);
}

void chatWidget::sendTBtnClicked() {
    if (ui->usrTblWidget->selectedItems().isEmpty()) {
        QMessageBox::warning(0, "选择用户", "请选择目标用户！", QMessageBox::Ok);
        return;
    }
//    QString destUsrName = ui->usrTblWidget->selectedItems().at(0)->text();
//    qDebug()<<destUsrName;
    srv->show();
    srv->initSrv();
}

//usrname是发送者的名字 srvaddr是服务器ip clntaddr是客户端ip filename为需要发送的信息

void chatWidget::hasPendingFile(QString usrname, QString srvaddr, QString clntaddr, QString filename) {
    QString ipAddr = getIP();
    //如果是发送端，执行refuse
    if (ipAddr == clntaddr && usrname != uName) {
        int btn = QMessageBox::information(this, "接收文件",
                                           tr("来自%1（%2）的文件：%3,是否接收？").arg(usrname).arg(ipAddr).arg(filename),
                                           QMessageBox::Yes, QMessageBox::No);
        if (btn == QMessageBox::Yes) {
            QString name = QFileDialog::getSaveFileName(0, "保存文件", filename);
            if (!name.isEmpty()) {
                client *clnt = new client(this);
                clnt->setFileName(name);
                clnt->setHostAddr(QHostAddress(srvaddr)); //problem
                clnt->show();
            }
        } else {
            sndMsg(Refuse, srvaddr);
        }
    }

}

void chatWidget::quitBtnClicked() {
    close();
}

void chatWidget::fontCbxChanged(const QFont &f) {
    ui->msgTxtEdit->setCurrentFont(f);
    ui->msgTxtEdit->setFocus();
}

void chatWidget::fontSizeChanged(const QString &arg) {
    ui->msgTxtEdit->setFontPointSize(arg.toDouble());
    ui->msgTxtEdit->setFocus();
}

void chatWidget::boldTBtnClicked(bool checked) {
    if (checked)
        ui->msgTxtEdit->setFontWeight(QFont::Bold);
    else
        ui->msgTxtEdit->setFontWeight(QFont::Normal);
    ui->msgTxtEdit->setFocus();
}
void chatWidget::italicTBtnClicked(bool checked) {
    ui->msgTxtEdit->setFontItalic(checked);
    ui->msgTxtEdit->setFocus();
}
void chatWidget::underlineTBtnClicked(bool checked) {
    ui->msgTxtEdit->setFontUnderline(checked);
    ui->msgTxtEdit->setFocus();
}
void chatWidget::colorTBtnClicked() {
    color = QColorDialog::getColor(color, this);
    if (color.isValid()) {
        ui->msgTxtEdit->setTextColor(color);
        ui->msgTxtEdit->setFocus();
    }
}
void chatWidget::saveTBtnClicked() {
    if (ui->msgBrowser->document()->isEmpty()) {
        QMessageBox::warning(0, "警告", "聊天记录为空，无法保存", QMessageBox::Ok);
    } else {
        QString fname = QFileDialog::getSaveFileName(this, "保存聊天记录", "聊天记录", "文本(*.txt;;所有文件(*.*)");
        if (!fname.isEmpty())
            saveFile(fname);
    }
}
bool chatWidget::saveFile(const QString &fileToSave) {
    QFile file(fileToSave);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, "保存文件", "无法保存文件" + tr("无法保存文件 %1:\n%2").arg(file.errorString()));
        return false;
    }
    QTextStream out(&file);
    out << ui->msgBrowser->toPlainText();
    return true;
}
void chatWidget::clearTBtnClicked() {
    ui->msgBrowser->clear();
}
void chatWidget::closeEvent(QCloseEvent *e) {
    sndMsg(UsrLeft);
    QWidget::closeEvent(e);
}


