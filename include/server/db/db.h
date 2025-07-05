#ifndef DB_H
#define DB_H

#include <mysql/mysql.h>
#include <string>
#include <chrono>
using namespace std;
using namespace chrono;
// 数据库操作类
class MysqlConn
{
public:
    // 初始化数据库连接
    MysqlConn();
    // 释放数据库连接资源
    ~MysqlConn();
    // 连接数据库
    bool connect();
    // 更新操作
    bool update(string sql);
    // 查询操作
    MYSQL_RES *query(string sql);
    // 获取连接
    MYSQL *getConnection();
    // 计算连接的空闲时常：刷新起始的空闲时间点
    void refreshAliveTime();
    // 计算连接的空闲时常：计算存活的总时长
    long long getAliveTime();

private:
    MYSQL *_conn;
    steady_clock::time_point m_alivetime; // 创建连接时or连接刚被使用完时   记录/更新时间戳，以便计算连接的空闲时间
};

#endif