#ifndef MYCHAT_CHATWIDGET_H
#define MYCHAT_CHATWIDGET_H

#include <QWidget>

class QUdpSocket;

class Server;

//消息类型
enum MsgType {
    Msg, UsrEnter, UsrLeft, FileName, Refuse
};


QT_BEGIN_NAMESPACE
namespace Ui { class chatWidget; }
QT_END_NAMESPACE


class chatWidget : public QWidget {
Q_OBJECT
public:
    explicit chatWidget(QWidget *parent, QString usrname);
    ~chatWidget();
    void initBtns();

protected:
    void usrEnter(QString usrname, QString ipaddr);     //处理有新用户加入
    void usrLeft(QString usrname, QString time);        //处理用户离开
    void sndMsg(MsgType type, QString srvaddr = "");    //广播UDP消息
    bool saveFile(const QString &filename);             //保存文件
    void closeEvent(QCloseEvent *) override;            //关闭
    QString getIP();    //获取IP地址
    QString getUsr();   //获取用户名
    QString getMsg();   //获取聊天信息
    //收到文件名UDP消息后判断是否接收该文件
    void hasPendingFile(QString usrname, QString srvaddr,
                        QString clntaddr, QString filename);

private:
    Ui::chatWidget *ui;         //UI界面文件
    QUdpSocket *udpSocket;      //UDP通信套接字
    qint16 port;
    QString uName;
    QString fileName;
    Server *srv;
    QColor color;

private slots:
    void processPendingDatagrams(); //接收UDP消息
    void getFileName(QString);      //获取Server类sndFileName()信号所发送的文件名
    void sendTBtnClicked();         //文件发送按钮
    void sendBtnClicked();          //消息发送按钮
    void quitBtnClicked();
    void fontCbxChanged(const QFont &);
    void fontSizeChanged(const QString &arg);
    void boldTBtnClicked(bool checked);
    void italicTBtnClicked(bool checked);
    void underlineTBtnClicked(bool checked);
    void colorTBtnClicked();
    void saveTBtnClicked();
    void clearTBtnClicked();

};


#endif //MYCHAT_CHATWIDGET_H