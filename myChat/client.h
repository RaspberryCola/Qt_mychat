#ifndef MYCHAT_CLIENT_H
#define MYCHAT_CLIENT_H

#include <QDialog>
#include <QHostAddress>
#include <QFile>
#include <QTime>


QT_BEGIN_NAMESPACE
namespace Ui { class client; }
QT_END_NAMESPACE

class QTcpSocket;

class client : public QDialog {
Q_OBJECT

public:
    explicit client(QWidget *parent = nullptr);
    ~client() override;
    void setHostAddr(QHostAddress addr);    //获取发送端IP地址
    void setFileName(QString name);         //获取文件保存路径

protected:
    void closeEvent(QCloseEvent *) override;

private:
    Ui::client *ui;
    QTcpSocket *tClnt;  //客户端套接字
    quint16 blockSize;
    QHostAddress hostAddress;
    qint16 tPort;
    qint64 totalBytes;      //总共需要接收的字节数
    qint64 bytesReceived;   //已接收字节数
    qint64 fileNameSize;
    QString fileName;
    QFile *locFile;         //待接收的文件
    QByteArray inBlock;     //缓存一次接收的数据
    QTime time;

private slots:
    void newConn();         //连接到服务器
    void readMsg();         //读取文件数据
    void displayErr(QAbstractSocket::SocketError);//显示错误信息
    void cCancleBtnClicked();
    void cCloseBtnClicked();
};


#endif //MYCHAT_CLIENT_H