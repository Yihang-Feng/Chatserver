#include "db.h"
#include <muduo/base/Logging.h>

// 数据库配置信息
static string server = "172.18.16.1";
static string user = "root";
static string password = "Yxy690426.";
static string dbname = "chat";

// 初始化数据库连接
MysqlConn::MysqlConn()
{
    _conn = mysql_init(nullptr);
}

// 释放数据库连接资源
MysqlConn::~MysqlConn()
{
    if (_conn != nullptr)
        mysql_close(_conn);
}

// 连接数据库
bool MysqlConn::connect()
{
    MYSQL *p = mysql_real_connect(_conn, server.c_str(), user.c_str(),
                                  password.c_str(), dbname.c_str(), 3306, nullptr, 0);
    if (p != nullptr)
    {
        // C和C++代码默认的编码字符是ASCII，如果不设置，从MySQL上拉下来的中文显示？
        mysql_query(_conn, "set names gbk");
        LOG_INFO << "connect mysql success!";
    }
    else
    {
        LOG_INFO << "connect mysql fail!";
    }

    return p;
}

// 更新操作
bool MysqlConn::update(string sql)
{
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":"
                 << sql << "更新失败!";
        return false;
    }

    return true;
}

// 查询操作
MYSQL_RES *MysqlConn::query(string sql)
{
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":"
                 << sql << "查询失败!";
        return nullptr;
    }

    return mysql_use_result(_conn);
}

// 获取连接
MYSQL *MysqlConn::getConnection()
{
    return _conn;
}

void MysqlConn::refreshAliveTime()
{
    m_alivetime = steady_clock::now();
}

long long MysqlConn::getAliveTime()
{
    nanoseconds res = steady_clock::now() - m_alivetime;
    milliseconds millsec = duration_cast<milliseconds>(res);
    return millsec.count();
}