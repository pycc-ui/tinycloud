#ifndef MYTHREAD_H
#define MYTHREAD_H
#include <QMainWindow>
#include <QThreadPool>
#include "process_http.h"
#include <fstream>
#include <filesystem>
#include <string>
class TaskManager;
class SubTask : public QRunnable {
public:
    SubTask(int id, TaskManager *manager);
    SubTask() = default;
    void set_id(int id);
    void set_manager(TaskManager *manager);
    void run() override;   // 执行任务
private:
    int m_id;
    TaskManager *m_manager;

    json content;
    json rev;
};


class TaskManager : public QObject {
    Q_OBJECT
public:
    explicit TaskManager(int totalTasks,json &content,int block_size);

    json content;
    int block_size;
    std::ofstream file;
public slots:
    void start();                        // 在管理线程中执行：开始第一批任务
private slots:
    void onSubTaskFinished(int taskId, QByteArray result); // 在管理线程中执行：收集结果
signals:
    void allTasksFinished();             // 所有任务完成（发送给主线程）
private:
    QThreadPool m_threadPool;            // 线程池，最大并发数设为5
    int m_totalTasks;                    // 总任务数
    int m_nextTaskId;                    // 下一个待分配的任务ID
    int m_completedCount;    // 已完成的总任务数


    // 批次管理
    int m_batchSize = 10;                 // 每批并发数
    QByteArray m_result[10];

    int m_batchCompletedCount;           // 当前批次已完成数
    int currentBatchSize ;
    void startNextBatch();               // 分配下一批任务
    void processBatchResults();          // 处理当前批次结果
};


#endif // MYTHREAD_H
