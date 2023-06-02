#include <QApplication>
#include <QPushButton>
#include "chatwidget.h"
#include "server.h"
#include "login.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    login l;
    l.show();
    return QApplication::exec();
}