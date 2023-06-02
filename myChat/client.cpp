#include "client.h"
#include "ui_client.h"
#include <QTcpSocket>
#include <QDebug>
#include <QMessageBox>
#include <QNetworkProxy>

client::client(QWidget *parent) : QDialog(parent), ui(new Ui::client) {
    ui->setupUi(this);
    setFixedSize(400, 250);

    totalBytes = 0;
    bytesReceived = 0;
    fileNameSize = 0;

    tClnt = new QTcpSocket(this);   //创建QTcpSocket对象tClnt
    tClnt->setProxy(QNetworkProxy::NoProxy);
    tPort = 5555;
    connect(tClnt, SIGNAL(readyRead()), this, SLOT(readMsg()));
    connect(tClnt, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayErr(QAbstractSocket::SocketError)));
    connect(ui->cCancleBtn, SIGNAL(clicked(bool)), this, SLOT(cCancleBtnClicked()));
    connect(ui->cCloseBtn, SIGNAL(clicked(bool)), this, SLOT(cCloseBtnClicked()));

}

client::~client() {
    delete ui;
}

void client::displayErr(QAbstractSocket::SocketError socketError) {
    switch (socketError) {
        case QAbstractSocket::RemoteHostClosedError:
            break;
        default:
            qDebug() << tClnt->errorString();
    }
}

void client::newConn() {
    blockSize = 0;
    tClnt->abort();
    tClnt->connectToHost(hostAddress, tPort);   //problem
    time.start();
}

void client::readMsg() {
    QDataStream in(tClnt);
    in.setVersion(QDataStream::Qt_5_8);

    float useTime = time.elapsed();

    if (bytesReceived <= sizeof(qint64) * 2) {
        if ((tClnt->bytesAvailable() >= sizeof(qint64) * 2) && (fileNameSize == 0)) {
            in >> totalBytes >> fileNameSize;
            bytesReceived += sizeof(qint64) * 2;
        }
        if ((tClnt->bytesAvailable() >= fileNameSize) && (fileNameSize != 0)) {
            in >> fileName;
            bytesReceived += fileNameSize;
            if (!locFile->open(QFile::WriteOnly)) {
                QMessageBox::warning(this, "应用程序", "无法读取文件" + fileName + locFile->errorString());
                return;
            }
        } else {
            return;
        }
    }
    if (bytesReceived < totalBytes) {
        bytesReceived += tClnt->bytesAvailable();
        inBlock = tClnt->readAll();
        locFile->write(inBlock);
        inBlock.resize(0);
    }

    ui->clientPgsBar->setMaximum(totalBytes);
    ui->clientPgsBar->setValue(bytesReceived);

    double speed = bytesReceived / useTime;
    ui->cStatusLabel->setText(tr("已接收 %1MB (%2MB/s)\n共%3MB 已用时：%4秒\n估计剩余时间：%5秒")
                                      .arg(bytesReceived / (1024 * 1024))
                                      .arg(speed * 1000 / (1024 * 1024), 0, 'f', 2)
                                      .arg(totalBytes / (1024 * 1024))
                                      .arg(useTime / 1000, 0, 'f', 0)
                                      .arg(totalBytes / speed / 1000 - useTime / 1000, 0, 'f', 0));
    if (bytesReceived == totalBytes) {
        locFile->close();
        tClnt->close();
        ui->cStatusLabel->setText("接收文件 " + fileName + " 完毕");
    }
}
void client::cCancleBtnClicked() {
    tClnt->abort();
    if (locFile->isOpen())
        locFile->close();
}

void client::cCloseBtnClicked() {
    tClnt->abort();
    if (locFile->isOpen())
        locFile->close();
    close();
}
void client::closeEvent(QCloseEvent *) {
    cCloseBtnClicked();
}

void client::setFileName(QString name) {
    locFile = new QFile(name);
}

void client::setHostAddr(QHostAddress addr) {
    hostAddress = addr;
    newConn();
}