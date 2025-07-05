#include "pool.h"
#include "db.h"
#include <fstream>
#include <thread>

ConnectionPool *ConnectionPool::getConnectPool()
{
    static ConnectionPool pool;
    return &pool;
}

shared_ptr<MysqlConn> ConnectionPool::getConnection()
{ // 有连接就给，没有就阻塞
    unique_lock<mutex> locker(m_mutexQ);
    while (m_connectionQ.empty())
    {
        if (cv_status::timeout == m_cond.wait_for(locker, chrono::milliseconds(m_timeout)))
        {
            if (m_connectionQ.empty())
            {
                // return nullptr;
                continue; // 继续阻塞
            }
        }
    }
    // 若队列有连接了，则从队头取出一个可用连接
    // 共享智能指针析构的时候，会释放它管理的空间，但是我们不希望连接被释放，因此创建智能指针的时候自定义删除器
    shared_ptr<MysqlConn> connptr(m_connectionQ.front(), [this](MysqlConn *conn)
                                  { lock_guard<mutex> locker(m_mutexQ);
                                    conn->getAliveTime();
                                    m_connectionQ.push(conn); });
    m_connectionQ.pop();
    m_cond.notify_all(); // 队列中有空闲，唤醒阻塞的生产者(这里由于使用一个条件变量，因此阻塞的消费者也会被唤醒，但是不影响，这些消费者会继续被阻塞)
    return connptr;
}

void ConnectionPool::produceConnection()
{
    while (true)
    {
        // unique_lock和lock_guard的区别？
        unique_lock<mutex> locker(m_mutexQ); // locker相当于一个锁管理者，locker对象创建/析构对应了m_mutexQ的加锁/解锁
        while (m_connectionQ.size() >= m_minSize)
        {
            m_cond.wait(locker); // 如果条件满足，生产者线程阻塞
        }
        addConnection();
        m_cond.notify_all(); // 队列中有连接了，唤醒阻塞的消费者（也会唤醒所有生产者线程，但是我们的程序中生产者线程只有一个。就算有多个，也不会影响，多余的生产者线程会通过下一次循环继续被阻塞。）
    } // locker在这里被析构，m_mutexQ解锁
}

void ConnectionPool::recycleConnection()
{
    while (true)
    {
        this_thread::sleep_for(chrono::seconds(1)); // 每隔一定时间检测一次
        lock_guard<mutex> locker(m_mutexQ);         // unique_lock和lock_guard的区别？
        while (m_connectionQ.size() >= m_minSize)
        {
            // 取出队头的连接，呆在队列里最久的
            MysqlConn *conn = m_connectionQ.front();
            if (conn->getAliveTime() >= m_maxIdleTime)
            { // 如果连接空闲时常以及超过阈值
                m_connectionQ.pop();
                delete conn;
            }
            else
            {
                break;
            }
        }
    }
}

void ConnectionPool::addConnection()
{
    MysqlConn *conn = new MysqlConn; // 初始化连接是在该类的构造函数中完成的，new运算符会调用构造函数
    conn->connect();
    conn->refreshAliveTime();
    m_connectionQ.push(conn);
}

ConnectionPool::ConnectionPool()
{
    m_minSize = 20;
    m_maxSize = 200;
    m_maxIdleTime = 5000;
    m_timeout = 1000;
    for (int i = 0; i < m_minSize; ++i)
    { // 创建minSize个数据库连接
        addConnection();
    }
    // “连接”数量的管理：创建新的连接，销毁空闲的连接
    thread producer(&ConnectionPool::produceConnection, this);
    thread recycle(&ConnectionPool::recycleConnection, this);
    /*线程对象在析构时必须处于明确的“已完成”或“已分离”状态，
    否则会触发 std::terminate() 导致程序崩溃。
    若不加detach,设主线程先执行完，thread对象析构，
    而thread对象关联的两子个线程仍在运行，会发生错误。
    （thread对象持有关联的线程的资源）*/
    producer.detach();
    recycle.detach();
}

ConnectionPool::~ConnectionPool() // 为什么析构函数不需要加锁
{
    while (!m_connectionQ.empty())
    {
        MysqlConn *conn = m_connectionQ.front();
        m_connectionQ.pop();
        delete conn;
    }
}
