#pragma once
#define BOOST_PYTHON_STATIC_LIB
#include "string"
#include "iostream"
#include "cppconn/driver.h"
#include "cppconn/statement.h"
#include "cppconn/prepared_statement.h"
#include "cppconn/metadata.h"
#include "cppconn/exception.h"
#include "boost/python.hpp"
#include "enums.h"
using namespace boost::python;
using namespace sql;
using namespace std;
using b_list = boost::python::list;

// 自定义属性
class Option {
public:
	bool isLog;
	bool autocommit;
	Option(bool t1, bool t2) {
		this->isLog = t1;
		this->autocommit = t2;
	}
};

class MyConnection {
private:
	// MySQL级属性
	Driver* driver;
	Connection* conn;
	ConnectOptionsMap connection_map;
	// 用户端属性
	bool isLog = true;
	bool autocommit = true;
	Option opt = Option{ true, true };
	void printLog(string log, LogType type);
	dict getResDict(ResultSet* res);
	ConnectOptionsMap getOptionFromDict(dict key);
	Savepoint* savepoint_ = nullptr;
public:
	
	MyConnection(string host, string username, string password, string database, int port, dict opt_dic, Option user_opt);
	MyConnection(string url, string username, string password, string database, Option user_opt);
	MyConnection(dict opt_dic, Option user_dict);
	dict execute(char* sql);						// 普通方式执行SQL语句
	dict execute_stmt(char* sql_stmt, b_list key);	// 预编译方式处理SQL语句
	bool switch_database(char* database);			// 修改所用数据库
	void print_conn_info();							// 打印连接信息
	void close() const;									// 关闭连接
	void setIsLog(bool isLog);						// 设置是否打印日志
	void setAutoCommit(bool autocommit);			// 设置是否自动提交
	void commit();
	bool createSavePoint(char* name);				// 创建储存点
	bool rollback();								// 回滚
	b_list getAutoCommitStatus();						// 打印当前
};
