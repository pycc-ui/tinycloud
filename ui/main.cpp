#include "information.h"
#include "login.h"
#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  information::getInstance();
  login loginWidget;

  MainWindow w;

  loginWidget.show();
  QObject::connect(&loginWidget, &login::loginSuccess, &w, &MainWindow::show);
  QObject::connect(&loginWidget, &login::loginSuccess, &w, &MainWindow::init);
  return a.exec();
}
