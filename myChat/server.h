#ifndef MYCHAT_SERVER_H
#define MYCHAT_SERVER_H

#include <QDialog>
#include <QTime>

class QFile;


//在TCP服务器类中，创建一个发送对话框以供用户选择文件发送，这里通过新建的QTcpServer对象实现
class QTcpServer;

class QTcpSocket;

QT_BEGIN_NAMESPACE
namespace Ui { class Server; }
QT_END_NAMESPACE

class Server : public QDialog {
Q_OBJECT
public:
    explicit Server(QWidget *parent = nullptr);
    ~Server() override;
    void initSrv(); //初始化服务器
    void refused(); //关闭服务器
protected:
    void closeEvent(QCloseEvent *);
private:
    Ui::Server *ui;

    QTcpServer *tSrv;       //tcp客户端
    QTcpSocket *clntConn;   //客户端连接的套接字
    qint16 tPort;           //端口号

    QString fileName;       //包含路径的文件名 D:\Clion\a.txt
    QString theFileName;    //文件名（不含路径） a.txt
    QFile *locFile;         //待发送的文件
    qint64 totalBytes;      //总共需要发送的字节数
    qint64 bytesWritten;    //已发送的字节数
    qint64 bytesTobeWrite;  //待发送字节数
    qint64 payloadSize;     //被初始化为一个常量
    QByteArray outBlock;    //缓存一次发送的数据

    QTime time;
private slots:
    void sndMsg();                             //发送数据
    void updateClntProgress(qint64 numBytes);  //更新进度条

    void openFileBtnClicked();  //槽函数：点击打开按钮
    void sendFileBtnClicked();  //槽函数：点击发送按钮
    void closeFileBtnClicked(); //槽函数：点击关闭按钮
signals:
    void sndFileName(QString name); //信号函数：
};


#endif //MYCHAT_SERVER_H