#include "offlinemessagemodel.hpp"
#include "db.h"
#include "pool.h"
extern ConnectionPool *pool;
// 存储用户的离线消息
void OfflineMsgModel::insert(int userid, string msg)
{
    shared_ptr<MysqlConn> conn = pool->getConnection();
    // 1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into offlinemessage values(%d, '%s')", userid, msg.c_str());
    conn->update(sql);
}

// 删除用户的离线消息
void OfflineMsgModel::remove(int userid)
{
    shared_ptr<MysqlConn> conn = pool->getConnection();
    // 1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "delete from offlinemessage where userid=%d", userid);

    conn->update(sql);
}

// 查询用户的离线消息
vector<string> OfflineMsgModel::query(int userid)
{
    shared_ptr<MysqlConn> conn = pool->getConnection();
    // 1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "select message from offlinemessage where userid = %d", userid);

    vector<string> vec;

    MYSQL_RES *res = conn->query(sql);
    if (res != nullptr)
    {
        // 把userid用户的所有离线消息放入vec中返回
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res)) != nullptr)
        {
            vec.push_back(row[0]); // 感觉这里有bug   row[1]
        }
        mysql_free_result(res);
        return vec;
    }

    return vec;
}