#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QButtonGroup>
#include <QGraphicsOpacityEffect>
#include <QListWidgetItem>
#include <QMainWindow>
#include <QMouseEvent>
class QPropertyAnimation; // <--- 添加这个前向声明
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();
  void init();

private slots:
  // 下面三个函数处理右上角三个按钮
  void minimizeWindow();
  void toggleFullscreen();
  void closeWindow();

  void init_listWidget();
  void on_uploadButton_clicked();

  void onItemDoubleClicked(QListWidgetItem *item);
  void addFileItem(const QString &fileName, qint64 fileSize, bool is_dir);

  void on_parentdirButton_clicked();

  void on_newdirButton_clicked();
  void showContextMenu(const QPoint &pos);
  void renameFile(const QString &filename);
  void deleteFile(const QString &filename);
  void deleteDir(const QString &dirname);
  void renameDir(const QString &dirname);

  void downloadFile(const QString &filename_remote);
  void on_pushButton_clicked();

  void on_set_ipButton_clicked();

  void on_set_portButton_clicked();

  void on_pushButton_2_clicked();

  protected:
  // 阴影成员变量
  QGraphicsDropShadowEffect *m_shadowEffect;

private:
  // 用于上面的按钮
  QRect m_normalGeometry;
  // 用于菜单栏
  QButtonGroup *m_buttonGroup;
  void onButtonClicked(QAbstractButton *button);

protected:
  // 下面三个函数处理拖动
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;

private:
  // 用于上面的拖动
  bool m_dragging = false;
  QPoint m_dragPosition;

private:
  Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
