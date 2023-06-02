#include "server.h"
#include "ui_Server.h"
#include <QFile>
#include <QTcpSocket>
#include <QTcpServer>
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include <QNetworkProxy>

Server::Server(QWidget *parent) : QDialog(parent), ui(new Ui::Server) {
    ui->setupUi(this);
    setFixedSize(430, 300); //固定窗口大小
    tPort = 5555;
    tSrv = new QTcpServer(this);    //创建QTcpServer对象
    initSrv();  //初始化服务器
    //进行信号和槽的关联
    connect(tSrv, SIGNAL(newConnection()), this, SLOT(sndMsg()));
    //打开按钮
    connect(ui->openFileBtn, SIGNAL(clicked(bool)), this, SLOT(openFileBtnClicked()));
    //发送按钮
    connect(ui->sendFileBtn, SIGNAL(clicked(bool)), this, SLOT(sendFileBtnClicked()));
    //关闭按钮
    connect(ui->closeFileBtn, SIGNAL(clicked(bool)), this, SLOT(closeFileBtnClicked()));
}

Server::~Server() {
    delete ui;
}

//初始化服务器
void Server::initSrv() {
    //初始化变量
    payloadSize = 64 * 1024;
    totalBytes = 0;
    bytesWritten = 0;
    bytesTobeWrite = 0;
    //初始化各按钮的状态
    ui->sStatusLabel->setText("请选择要发送的文件！");
    ui->sendPgsBtn->reset();
    ui->openFileBtn->setEnabled(true);
    ui->sendFileBtn->setEnabled(false);
    tSrv->close();  //关闭服务器
}

//发送数据
void Server::sndMsg() {
    ui->sendFileBtn->setEnabled(false);
    //获取这个服务器tSrv与客户端通信的套接字
    clntConn = tSrv->nextPendingConnection();
    clntConn->setProxy(QNetworkProxy::NoProxy);
    connect(clntConn, SIGNAL(bytesWritten(qint64)), this, SLOT(updateClntProgress(qint64)));

    ui->sStatusLabel->setText("开始传送文件 " + theFileName);

    //locFile 本地要发送的文件
    locFile = new QFile(fileName);
    //以只读方式打开选中的文件
    if (!locFile->open(QFile::ReadOnly)) {
        QMessageBox::warning(this, "应用程序", "无法提取文件" + fileName + '\n' + locFile->errorString());
        return;
    }
    //获取待发送文件的大小，暂存在totalBytes变量中
    totalBytes = locFile->size();
    //将发送缓冲区outBlock封装在一个QDataStream类型的变量中
    QDataStream sendOut(&outBlock, QIODevice::WriteOnly);
    sendOut.setVersion(QDataStream::Qt_5_8);

    //time统计传输所用的时间
    time.start();

    //通过QString类的right()函数去掉文件的路径部分，仅将文件部分保存在curFile变量中
    QString curFile = fileName.right(fileName.size() - fileName.lastIndexOf('/') - 1);
    //构造一个临时的文件头，将该值追加到totalBytes字段，从而完成实际需要发送字节数的记录
    sendOut << qint64(0) << qint64(0) << curFile;
    totalBytes += outBlock.size();

    //将读写操作指向从头开始
    sendOut.device()->seek(0);
    //填写实际的总长度和文件长度
    sendOut << totalBytes << qint64((outBlock.size() - sizeof(qint64) * 2));
    //将文件头发出，同时修改待发送字节数bytesTobeWrite
    bytesTobeWrite = totalBytes - clntConn->write(outBlock);
    //清空发送缓冲区以备下次使用
    outBlock.resize(0);
}

void Server::updateClntProgress(qint64 numBytes) {
    //用在传输大文件时，界面不会冻结
    qApp->processEvents();

    bytesWritten += int(numBytes);
    if (bytesWritten > 0) {
        outBlock = locFile->read(qMin(bytesTobeWrite, payloadSize));
        bytesTobeWrite -= int(clntConn->write(outBlock));
        outBlock.resize(0);
    } else {
        locFile->close();
    }

    ui->sendPgsBtn->setMaximum(totalBytes);
    ui->sendPgsBtn->setValue(bytesWritten);

    float useTime = time.elapsed();
    double speed = bytesWritten / useTime;
    ui->sStatusLabel->setText(tr("已接收 %1MB (%2MB/s)\n共%3MB 已用时：%4秒\n估计剩余时间：%5秒")
                                      .arg(bytesWritten / (1024 * 1024))
                                      .arg(speed * 1000 / (1024 * 1024), 0, 'f', 2)
                                      .arg(totalBytes / (1024 * 1024))
                                      .arg(useTime / 1000, 0, 'f', 0)
                                      .arg(totalBytes / speed / 1000 - useTime / 1000, 0, 'f', 0));
    if (bytesWritten == totalBytes) {
        locFile->close();
        tSrv->close();
        ui->sStatusLabel->setText("传送文件 " + theFileName + " 成功");
    }
}

void Server::openFileBtnClicked() {
    fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty()) {
        theFileName = fileName.right(fileName.size() - fileName.lastIndexOf('/') - 1);
        ui->sStatusLabel->setText("要传送的文件为：" + theFileName);
        ui->sendFileBtn->setEnabled(true);
        ui->openFileBtn->setEnabled(false);
    }
}

void Server::sendFileBtnClicked() {
    //开始监听所有网络接口和指定端口的连接
    if (!tSrv->listen(QHostAddress::Any, tPort)) {
        qDebug() << tSrv->errorString();
        close();
        return;
    }
    ui->sStatusLabel->setText("等待对方接收......");
    emit sndFileName(theFileName);
}

void Server::closeFileBtnClicked() {
    if (tSrv->isListening()) {
        tSrv->close();
        if (locFile->isOpen())
            locFile->close();
        clntConn->abort();
    }
    close();
}

void Server::closeEvent(QCloseEvent *) {
    closeFileBtnClicked();
}

void Server::refused() {
    tSrv->close();
    ui->sStatusLabel->setText("对方拒绝接收！");
}