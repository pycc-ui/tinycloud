#include "mythread.h"
#include "base64.h"

TaskManager::TaskManager(int totalTasks,json &content,int block_size) : m_totalTasks(totalTasks) {
  m_threadPool.setMaxThreadCount(10); // 最多5个并发工作线程
    this->content = std::move(content);
  this->block_size = block_size;


    std::string filepath = information::getInstance().get_download_path();
    std::filesystem::path dir = filepath;
    if (!std::filesystem::exists(dir)) {
        std::filesystem::create_directories(dir);
    }
    std::filesystem::path full_path =
        dir / std::filesystem::path(this->content["virtual_file_path"]).filename();

    // 以二进制模式打开，并清空已有内容
     file.open(full_path,
                       std::ios::out | std::ios::binary | std::ios::trunc);
    if (!file.is_open()) {
        // 处理打开失败
        qDebug() << "Failed to open file for writing";
        return;
    }
}


void TaskManager::start() {
  // 运行在【管理线程】
  m_nextTaskId = 0;
  m_completedCount = 0;
  startNextBatch(); // 开始第一批
}

void TaskManager::startNextBatch() {
  // 运行在【管理线程】
  if (m_nextTaskId >= m_totalTasks)
    return;

  int remaining = m_totalTasks - m_nextTaskId;
  currentBatchSize = qMin(m_batchSize, remaining);

  m_batchCompletedCount = 0;

  // 分配本批次任务（最多5个）
  for (int i = 0; i < currentBatchSize; ++i) {
      int taskId = m_nextTaskId++;
      SubTask *task = new SubTask(taskId, this);
      m_threadPool.start(task);
  }
  // 函数返回，管理线程继续运行（等待子任务完成）
}

void TaskManager::onSubTaskFinished(int taskId, QByteArray result) {
  // 运行在【管理线程】（由工作线程通过 invokeMethod 触发）
  // 收集结果到批次列表中（假设按完成顺序，这里简单处理）
  m_result[taskId % 10] = result;
  m_batchCompletedCount++;
  m_completedCount++;

  // 如果当前批次所有任务都完成了
  if (m_batchCompletedCount == currentBatchSize) {
    processBatchResults(); // 处理本批次结果
    // 处理完毕后，启动下一批
    startNextBatch();
  }

  // 如果所有任务都完成了，发射信号通知主线程
  if (m_completedCount == m_totalTasks) {
      file.close();
    emit allTasksFinished();
  }
}

void TaskManager::processBatchResults() {

    for (int i = 0 ; i < currentBatchSize; i++)
    {
        const char* data = m_result[i].constData();
        std::streamsize size = m_result[i].size();
        file.write(data, size);
    }
}

SubTask::SubTask(int id, TaskManager *manager) : m_id(id), m_manager(manager) {
    this->content = manager->content;
}

void SubTask::set_id(int id)
{
    m_id = id;
}
void SubTask::set_manager(TaskManager *manager)
{
    m_manager = manager;
}

void SubTask::run() {
    thread_local  Process_http download;
    content["block_begin"] = m_manager->block_size * m_id;
    content["downloading"] = true;
    download.http_send("POST","/file/download","keep-alive",content);
    download.http_rev(rev);

    std::string decoded = base64_decode(rev["body"]["document_content"]);
    QByteArray result(decoded.data(), decoded.size());

    QMetaObject::invokeMethod(m_manager, "onSubTaskFinished",
                              Qt::QueuedConnection,
                              Q_ARG(int, m_id),
                              Q_ARG(QByteArray, result));

}
