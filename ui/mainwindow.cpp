#include "mainwindow.h"
#include "base64.h"
#include "information.h"
#include "mythread.h"
#include "process_http.h"
#include "sha256.h"
#include "ui_mainwindow.h"
#include <QEasingCurve>
#include <QFileDialog>
#include <QGraphicsOpacityEffect>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QThread>
#include <QTimer>
#include <Qstring>
#include <filesystem>
#include <fstream>
#include <iostream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  // 处理按钮
  //  创建按钮组
  m_buttonGroup = new QButtonGroup(this);
  m_buttonGroup->setExclusive(true); // 设置互斥，只能选一个
  // 将按钮添加到按钮组
  m_buttonGroup->addButton(ui->user_toolButton, 0);
  m_buttonGroup->addButton(ui->file_toolButton, 1);
  m_buttonGroup->addButton(ui->setting_toolButton, 2);
  // 设置所有按钮为可选中状态
  for (auto button : m_buttonGroup->buttons()) {
    button->setCheckable(true);
  }
  // 默认选中第一个按钮
  ui->user_toolButton->setChecked(true);
  ui->stackedWidget->setCurrentWidget(ui->userwindow);
  // 连接槽函数
  connect(m_buttonGroup, &QButtonGroup::buttonClicked, this,
          &MainWindow::onButtonClicked);

  // 处理自带的窗口标题(不好看)
  setAttribute(Qt::WA_TranslucentBackground);
  setWindowFlags(windowFlags() | Qt::FramelessWindowHint);

  QWidget *central = centralWidget();
  // 给现有centralWidget添加阴影效果
  m_shadowEffect = new QGraphicsDropShadowEffect(central);
  m_shadowEffect->setBlurRadius(20);              // 阴影模糊半径
  m_shadowEffect->setColor(QColor(0, 0, 0, 120)); // 半透明黑色
  m_shadowEffect->setOffset(0, 0);                // 无偏移
  central->setGraphicsEffect(m_shadowEffect);
  setContentsMargins(10, 10, 10, 10);

  // 连接信号与槽
  connect(ui->minimizeButton, &QToolButton::clicked, this,
          &MainWindow::minimizeWindow);
  connect(ui->fullscreenButton, &QToolButton::clicked, this,
          &MainWindow::toggleFullscreen);
  connect(ui->closeButton, &QToolButton::clicked, this,
          &MainWindow::closeWindow);
  connect(ui->listWidget, &QListWidget::itemDoubleClicked, this,
          &MainWindow::onItemDoubleClicked);
  ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu);

  connect(ui->listWidget, &QListWidget::customContextMenuRequested, this,
          &MainWindow::showContextMenu);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::init() {
  ui->ip_LineEdit->setText(
      QString::fromStdString(information::getInstance().get_ip()));
  ui->downloadPath_LineEdit->setText(
      QString::fromStdString(information::getInstance().get_download_path()));
  ui->port_LineEdit->setText(
      QString::fromStdString(information::getInstance().get_port()));
  ui->nameEdit->setText(QString::fromStdString(information::getInstance().get_user()));
  init_listWidget();
}

void MainWindow::downloadFile(const QString &filename_remote) {


    json content;
    json rev;
    Process_http download;
    content["username"] = "111";
    content["virtual_file_path"] = information::getInstance().get_remote_path() +
                                   filename_remote.toStdString();
    content["downloading"] = false;



    download.http_send("POST", "/file/download", "close", content);
    download.http_rev(rev);

    content["actual_file_path"] = rev["body"]["actual_file_path"];
    int file_size = rev["body"]["file_size"];
    int block_size = 1024*16;

    int tasknum = (file_size / block_size) + 1;
     content["block_size"] = block_size;

      QThread *managerThread = new QThread(this);
      TaskManager *manager = new TaskManager(tasknum,content,block_size);

      manager->moveToThread(managerThread);
      connect(managerThread, &QThread::started, manager, &TaskManager::start);
      connect(manager, &TaskManager::allTasksFinished, this, [managerThread,manager]() {

          managerThread->quit();        // 请求退出
          qDebug() << "下载完成";
      });
      connect(managerThread, &QThread::finished, manager, &QObject::deleteLater);
      connect(managerThread, &QThread::finished, managerThread,&QThread::deleteLater);

      managerThread->start(); // 启动管理线程，立即返回
}

void MainWindow::renameFile(const QString &filename) {

  bool ok;
  QString newfileName = QInputDialog::getText(
      this, tr("重命名"), tr("文件名称:"), QLineEdit::Normal, "", &ok);
  if (!ok || newfileName.isEmpty()) {
    return; // 用户取消或未输入
  }
  Process_http rename;
  json content;
  json rev;
  content["username"] = information::getInstance().get_user();
  std::filesystem::path remote_vir_path =
      information::getInstance().get_remote_path();
  std::filesystem::path newnamePath =
      remote_vir_path / newfileName.toStdWString();
  remote_vir_path = std::filesystem::path(
      (remote_vir_path / filename.toStdString()).generic_string());
  content["virtual_file_path"] = remote_vir_path.string();
  // 3. 组合完整路径
  content["new_name_path"] = newnamePath.string();
  rename.http_send("POST", "/file/rename", "close", content);
  rename.http_rev(rev);
  init_listWidget();
}

void MainWindow::deleteFile(const QString &filename) {

  Process_http deletefile;
  json content;
  json rev;
  content["username"] = information::getInstance().get_user();
  std::filesystem::path remote_vir_path =
      information::getInstance().get_remote_path();
  remote_vir_path = std::filesystem::path(
      (remote_vir_path / filename.toStdString()).generic_string());
  content["virtual_file_path"] = remote_vir_path.string();
  deletefile.http_send("POST", "/file/delete", "close", content);
  deletefile.http_rev(rev);
  init_listWidget();
}

void MainWindow::renameDir(const QString &dirname) {
  bool ok;
  QString folderName = QInputDialog::getText(
      this, tr("重命名"), tr("文件夹名称:"), QLineEdit::Normal, "", &ok);
  if (!ok || folderName.isEmpty()) {
    return; // 用户取消或未输入
  }
  Process_http rename;
  json content;
  json rev;
  content["username"] = information::getInstance().get_user();
  std::filesystem::path remote_vir_path =
      information::getInstance().get_remote_path();
  std::filesystem::path newnamePath =
      remote_vir_path / folderName.toStdWString();

  remote_vir_path = std::filesystem::path(
      (remote_vir_path / dirname.toStdString()).generic_string());
  content["old_dir_path"] = remote_vir_path.string();
  // 3. 组合完整路径

  content["new_dir_path"] = newnamePath.string();
  rename.http_send("POST", "/directory/rename", "close", content);
  rename.http_rev(rev);
  init_listWidget();
}

void MainWindow::deleteDir(const QString &dirname) {
  Process_http deletefile;
  json content;
  json rev;
  content["username"] = information::getInstance().get_user();
  std::filesystem::path remote_vir_path =
      information::getInstance().get_remote_path();
  remote_vir_path = std::filesystem::path(
      (remote_vir_path / dirname.toStdString()).generic_string());

  content["virtual_dir_path"] = remote_vir_path.string();
  deletefile.http_send("POST", "/directory/delete", "close", content);
  deletefile.http_rev(rev);
  init_listWidget();
}

void MainWindow::showContextMenu(const QPoint &pos) {
  QListWidgetItem *item = ui->listWidget->itemAt(pos);
  if (!item)
    return; // 没点到任何 item

  // 获取存储的数据
  bool isDir = item->data(Qt::UserRole).toBool();
  QString name = item->data(Qt::UserRole + 1).toString();

  QMenu menu;

  if (!isDir) {
    // 文件：下载、重命名、删除
    menu.addAction("下载", this, [this, name]() { downloadFile(name); });
    menu.addAction("重命名", this, [this, name, item]() { renameFile(name); });
    menu.addAction("删除", this, [this, name]() { deleteFile(name); });
  } else {
    // 目录：重命名、删除（可能不支持下载）
    menu.addAction("重命名", this, [this, name, item]() { renameDir(name); });
    menu.addAction("删除", this, [this, name]() { deleteDir(name); });
  }

  menu.exec(ui->listWidget->mapToGlobal(pos));}

void MainWindow::init_listWidget() {
  ui->listWidget->clear();
  const std::string virtual_dir_path =
      information::getInstance().get_remote_path();

  Process_http dir_list;
  json content;
  json rev;
  content["virtual_dir_path"] = virtual_dir_path;
  content["username"] = information::getInstance().get_user();
  dir_list.http_send("POST", "/directory/list", "close", content);
  dir_list.http_rev(rev);


  if (rev["body"]["status"] == "success") {
    json dir_list = rev["body"]["dir_list"];

    for (const auto &item : dir_list) {
      std::string name = item["name"];
      int is_dir = item["is_dir"];
      qint64 file_size = stoi(item.value("file_size", "0"));
      addFileItem(QString::fromStdString(name), file_size, is_dir == 1);
    }
  } else {
  }
}

void MainWindow::onItemDoubleClicked(QListWidgetItem *item) {
  // 判断是否为目录（通过存储的自定义数据）
  bool isDir = item->data(Qt::UserRole).toBool();
  if (isDir) {
    QString dirName = item->data(Qt::UserRole + 1).toString();
    // 构建新路径（假设你有一个 currentPath 成员变量记录当前目录）
    std::filesystem::path old = information::getInstance().get_remote_path();
    std::filesystem::path new_path = (old / dirName.toStdString()).generic_string();
    information::getInstance().set_current_remote_path(new_path.string());
    // 调用刷新列表的函数，重新加载该目录下的内容
    init_listWidget(); // 假设 init_listwight 接受路径参数
  }
  // 文件项也可以双击下载(未来可选)
  // else {
  //     downloadFile(item->data(Qt::UserRole + 1).toString());
  // }
}
// 下面三个函数处理拖动
void MainWindow::mousePressEvent(QMouseEvent *event) {
  if (this->isFullScreen()) {
    return;
  }
  if (event->button() == Qt::LeftButton) {
    // 检查点击是否在 widget 区域内
    if (ui->widget->geometry().contains(event->pos())) {
      m_dragging = true;
      m_dragPosition =
          event->globalPosition().toPoint() - frameGeometry().topLeft();
      event->accept();
      return;
    }
    if (ui->widget_2->geometry().contains(event->pos())) {
      m_dragging = true;
      m_dragPosition =
          event->globalPosition().toPoint() - frameGeometry().topLeft();
      event->accept();
      return;
    }
    if (ui->stackedWidget->geometry().contains(event->pos())) {
      m_dragging = true;
      m_dragPosition =
          event->globalPosition().toPoint() - frameGeometry().topLeft();
      event->accept();
      return;
    }
  }
  QMainWindow::mousePressEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event) {
  if (m_dragging && (event->buttons() & Qt::LeftButton)) {
    move(event->globalPosition().toPoint() - m_dragPosition);
    event->accept();
    return;
  }
  QMainWindow::mouseMoveEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    m_dragging = false;
    event->accept();
    return;
  }
  QMainWindow::mouseReleaseEvent(event);
}

// 下面三个函数处理右上角三个按钮
void MainWindow::minimizeWindow() { showMinimized(); }

void MainWindow::toggleFullscreen() {

  if (isFullScreen()) {
    ui->closeButton->setStyleSheet(
        "QToolButton#closeButton::hover{ border-top-right-radius: 8px; }");
    ui->centralwidget->setStyleSheet(
        "QWidget#centralwidget { border-radius: 8px; }");
    ui->fullscreenButton->setIcon(QIcon(":/img/full-screen.svg"));
    setContentsMargins(10, 10, 10, 10);
    showNormal();

  } else {

    m_normalGeometry = geometry(); // 保存当前窗口大小和位置
    // 更改样式 (和你的代码一致)
    ui->closeButton->setStyleSheet(
        "QToolButton#closeButton::hover{ border-top-right-radius: 0px; }");
    ui->centralwidget->setStyleSheet(
        "QWidget#centralwidget { border-radius: 0px; }");
    ui->fullscreenButton->setIcon(QIcon(":/img/oExit-fullscreen.svg"));
    setContentsMargins(0, 0, 0, 0); // 移除边距
    showFullScreen();
  }
}

void MainWindow::closeWindow() { close(); }

// 按钮对应页面
void MainWindow::onButtonClicked(QAbstractButton *button) {
  // 获取按钮在按钮组中的索引
  int index = m_buttonGroup->buttons().indexOf(button);

  if (index != -1) {
    // 设置堆栈窗口显示对应的页面
    ui->stackedWidget->setCurrentIndex(index);
  }
}

void MainWindow::on_uploadButton_clicked() {
  const size_t CHUNK_SIZE = 1024*16;
    QString fileName = QFileDialog::getOpenFileName(
        this, "选择单个文件", QDir::homePath(),
        "所有文件 (*.*);;图片文件 (*.png *.jpg *.jpeg *.bmp)");

    if (!fileName.isEmpty()) {
        qDebug() << "选择的文件:" << fileName;
    } else {
        return;
    }
  json content;
  json rev;
  content["username"] = information::getInstance().get_user();

  std::string filepath = fileName.toStdString();
  std::string sha256_num = calculateFileSHA256(filepath);

  std::ifstream file(filepath.c_str(), std::ios::binary);
  file.seekg(0, std::ios::end);
  size_t size = file.tellg();
  file.seekg(0, std::ios::beg);

  Process_http upload;

  if (!file.is_open()) {
    qDebug() << "无法打开文件\n";
    return;
  }

  std::filesystem::path p = filepath;
  std::string name = p.filename().string();

  content["sha256_num"] = sha256_num.c_str();
  std::filesystem::path remote_vir_path =
      information::getInstance().get_remote_path();
  remote_vir_path =
      std::filesystem::path((remote_vir_path / name).generic_string());
  content["virtual_file_path"] = remote_vir_path.string();
  content["file_size"] = std::to_string(size);
  content["appanding"] = false;
  upload.http_send("POST", "/file/upload", "keep-alive", content);
  upload.http_rev(rev);
  content["actual_file_path"] = rev["body"]["actual_file_path"];
  content.erase("file_size");
  content.erase("sha256_num");
  int count = 0;
  while (file) {
    // 准备缓冲区
    char buffer[CHUNK_SIZE];
    memset(buffer, 0, CHUNK_SIZE);
    // 读取数据到缓冲区
    file.read(buffer, CHUNK_SIZE - 1);

    // 获取实际读取的字节数
    size_t bytes_read = file.gcount();

    if (bytes_read == 0) {
      break; // 没有读到数据，结束循环
    }

    // 创建字符串并复制数据
    std::string chunk(buffer, bytes_read);

    chunk = base64_encode(chunk);

    content["document_content"] = chunk;
    content["appanding"] = true;
    // 判断是否为最后一个块
    std::string temp;
    if (file.peek() == EOF) {
      // 最后一个块，使用 close
      temp = "close";
      qDebug() << "文件末尾,关闭";

    } else if (rev["body"]["status"] == "success") {
      // 还有后续块，使用 keep-alive
      temp = "keep-alive";
    } else {
      qDebug() << "有错误,最后的报文是:";
      qDebug() << rev["body"].dump(4);
      break;
    }
    if (!upload.http_send("POST", "/file/upload", temp.c_str(), content)) {
      qDebug() << "发送失败";
      break;
    }
    if (!upload.http_rev(rev)) {
      qDebug() << "有问题,最后的报文是:";
      qDebug() << rev["body"].dump(4);
      qDebug() << "接收响应失败，终止上传";
      break;
    }
    if (temp == "close") {
      break;
    }
  }
  file.close();
  qDebug()<<"上传结束";
  init_listWidget();
}

void MainWindow::addFileItem(const QString &fileName, qint64 fileSize,
                             bool is_dir) {

  // 创建容器 widget
  QWidget *itemWidget = new QWidget;
  QHBoxLayout *layout = new QHBoxLayout(itemWidget);
  layout->setContentsMargins(5, 5, 5, 5);
  layout->setSpacing(10);

  // 图标
  QLabel *icon = new QLabel;
  if (is_dir) {
    icon->setPixmap(
        QPixmap(":/img/dircon.svg").scaled(32, 32, Qt::KeepAspectRatio));
  } else {
    icon->setPixmap(
        QPixmap(":/img/filecon.svg").scaled(32, 32, Qt::KeepAspectRatio));
  }

  // 名称（目录不显示大小，文件显示大小）
  QVBoxLayout *infoLayout = new QVBoxLayout;
  QLabel *nameLabel = new QLabel(fileName);
  nameLabel->setStyleSheet("font-weight: bold;");
  infoLayout->addWidget(nameLabel);

  if (!is_dir) {
    QLabel *sizeLabel =
        new QLabel(tr("%1 KB").arg(fileSize / 1024.0, 0, 'f', 1));
    sizeLabel->setStyleSheet("color: blue; font-size: 10px;");
    infoLayout->addWidget(sizeLabel);
  }

  // 布局组装
  layout->addWidget(icon);
  layout->addLayout(infoLayout);
  layout->addStretch(); // 弹性空间，让内容左对齐

  itemWidget->setFixedHeight(50);

  // 创建 QListWidgetItem
  QListWidgetItem *item = new QListWidgetItem(ui->listWidget);
  item->setSizeHint(itemWidget->sizeHint());

  // 存储数据
  item->setData(Qt::UserRole, is_dir);       // true=目录, false=文件
  item->setData(Qt::UserRole + 1, fileName); // 文件名/目录名（或相对路径）

  // 如果是文件，还可以存储大小（可选）
  if (!is_dir) {
    item->setData(Qt::UserRole + 2, fileSize);
  }

  ui->listWidget->addItem(item);
  ui->listWidget->setItemWidget(item, itemWidget);
}

void MainWindow::on_parentdirButton_clicked() {
  std::filesystem::path old = information::getInstance().get_remote_path();
  std::filesystem::path new_path = old.parent_path();
  information::getInstance().set_current_remote_path(
      new_path.empty() ? old.string() : new_path.string());
  init_listWidget();
}

void MainWindow::on_newdirButton_clicked() {
  std::filesystem::path targetDir =
      information::getInstance().get_remote_path();

  // 2. 弹出输入框，获取文件夹名称
  bool ok;
  QString folderName = QInputDialog::getText(
      this, tr("新建文件夹"), tr("文件夹名称:"), QLineEdit::Normal, "", &ok);
  if (!ok || folderName.isEmpty()) {
    return; // 用户取消或未输入
  }

  // 3. 组合完整路径
  std::filesystem::path newFolderPath = targetDir / folderName.toStdWString();

  Process_http dir_new;
  json content;
  json rev;
  content["username"] = information::getInstance().get_user();
  content["new_dir_path"] = newFolderPath.generic_string();
  dir_new.http_send("POST", "/directory/create", "close", content);
  dir_new.http_rev(rev);

  if (rev["body"]["status"] == "exists") {
    QMessageBox::warning(this, tr("错误"), tr("该名称已存在，请更换名称。"));
    return;
  } else if (rev["body"]["status"] == "error") {
    return;
  }
  init_listWidget();
}

void MainWindow::on_pushButton_clicked()
{
    QString dirPath = QFileDialog::getExistingDirectory(
        this,
        "选择文件夹",
        QDir::homePath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
        );

    if (!dirPath.isEmpty()) {
        qDebug() << "选择的文件夹:" << dirPath;
    } else {
        return;
    }
    information::getInstance().set_download_path(dirPath.toStdString());
    ui->downloadPath_LineEdit->setText(dirPath);
}


void MainWindow::on_set_ipButton_clicked()
{
    bool ok;
    QString ip = QInputDialog::getText(
        this, tr("设置ip"), tr("ip:"), QLineEdit::Normal, "", &ok);
    if (!ok || ip.isEmpty()) {
        return; // 用户取消或未输入
    }
    ui->ip_LineEdit->setText(ip);
    information::getInstance().set_ip(ip.toStdString());
}


void MainWindow::on_set_portButton_clicked()
{
    bool ok;
    QString port = QInputDialog::getText(
        this, tr("设置port"), tr("port:"), QLineEdit::Normal, "", &ok);
    if (!ok || port.isEmpty()) {
        return; // 用户取消或未输入
    }
    ui->port_LineEdit->setText(port);
    information::getInstance().set_port(port.toStdString());
}


void MainWindow::on_pushButton_2_clicked()
{
    bool ok;
    QString passwd = QInputDialog::getText(
        this, tr("设置密码"), tr("密码:"), QLineEdit::Normal, "", &ok);
    if (!ok || passwd.isEmpty()) {
        return; // 用户取消或未输入
    }

    Process_http c_passwd;
    json content;
    json rev;
    content["username"] = information::getInstance().get_user();
    content["old_password"] = information::getInstance().get_passwd();
    content["new_password"] = passwd.toStdString();
    c_passwd.http_send("POST", "/auth/change_password", "close", content);
    c_passwd.http_rev(rev);
    if(rev["body"]["status"] == "success"){
        information::getInstance().set_passwd(passwd.toStdString());
        qDebug()<<"修改成功";
    }
}

