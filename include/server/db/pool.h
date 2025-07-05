#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include "db.h"
class ConnectionPool // 单例模式，懒汉模式
{
public:
    static ConnectionPool *getConnectPool();
    ConnectionPool(const ConnectionPool &obj) = delete;
    ConnectionPool &operator=(const ConnectionPool &obj) = delete;
    shared_ptr<MysqlConn> getConnection(); // 用户从连接池获取连接

private:
    ConnectionPool();
    ~ConnectionPool();
    void produceConnection(); // 创建数据库连接
    void recycleConnection(); // 销毁数据库连接
    void addConnection();
    // 以下属性是每次建立连接时要传入的数据，将其写入json配置文件中，通过更改配置文件为它们动态的赋值。
    int m_minSize;
    int m_maxSize;
    int m_timeout;
    int m_maxIdleTime;

    queue<MysqlConn *> m_connectionQ;

    mutex m_mutexQ;
    condition_variable m_cond;
};
