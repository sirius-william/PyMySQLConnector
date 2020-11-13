#include "PyMySQLConnector.hpp"
#include "ctime"
#include "atlstr.h"
#include "MyDataType.h"
using namespace std;

//rpcndr

// 构造函数1
MyConnection::MyConnection(dict opt_dic, Option user_dict) {
	try {
		this->isLog = user_dict.isLog;
		this->driver = get_driver_instance();
		b_list items = opt_dic.items();
		ConnectOptionsMap connect_options_map;
		connect_options_map = getOptionFromDict(opt_dic);
		this->conn = this->driver->connect(connect_options_map);
		this->conn->setAutoCommit(user_dict.autocommit);
		printLog("Init Connection successfully", INFO);
	}
	catch (exception e) {
		printLog("Init Connection with failure cause" + string(e.what()), FATAL);
		exit(-1);
	}
}


// 构造函数2
MyConnection::MyConnection(string host, string username, string password, string database, int port, dict opt_dic,
                           Option user_opt) {
	try {
		this->isLog = user_opt.isLog;
		this->driver = get_driver_instance();
		b_list items = opt_dic.items();
		ConnectOptionsMap connect_options_map;
		connect_options_map = getOptionFromDict(opt_dic);
		connect_options_map["hostName"] = host.c_str();
		connect_options_map["userName"] = username.c_str();
		connect_options_map["password"] = password.c_str();
		connect_options_map["port"] = port;
		this->conn = this->driver->connect(connect_options_map);
		this->conn->setSchema(database.c_str());
		this->conn->setAutoCommit(user_opt.autocommit);
		printLog("Init Connection successfully", INFO);
	}
	catch (exception e) {
		printLog("Init Connection with failure cause" + string(e.what()), FATAL);
		exit(-1);
	}
}

// 构造函数3
MyConnection::MyConnection(string url, string username, string password, string database, Option user_opt) {
	try {
		this->isLog = user_opt.isLog;
		this->driver = get_driver_instance();
		this->conn = this->driver->connect(url.c_str(), username.c_str(), password.c_str());
		this->conn->setSchema(database.c_str());
		this->conn->setAutoCommit(user_opt.autocommit);
		printLog("Init Connection successfully", INFO);
	}
	catch (exception e) {
		printLog("Init Connection with failure cause" + string(e.what()), FATAL);
		exit(-1);
	}
}

// 配置转换
ConnectOptionsMap MyConnection::getOptionFromDict(dict key_dict) {
	// 将字典转换为键值对列表
	b_list item = key_dict.items();
	ConnectOptionsMap connect_options_map;
	// Python API PyObject
	try {
		for (int i = 0; i < len(item); i++) {
			object key = item[i][0]; // value的boost::python::object对象
			// 将object转换成Python API内置对象
			object value = item[i][1];
			extract<char*> x(key);
			if (!x.check()) {
				throw invalid_argument("the Option dict keys (is Option name) must be string.");
			}
			// 字符串类型
			extract<char*> is_char(value);
			if (is_char.check()) {
				connect_options_map[static_cast<char*>(x)] = static_cast<char*>(is_char);
				continue;
			}
			// 整型
			extract<int> is_int(value);
			if (is_int.check()) {
				connect_options_map[static_cast<char*>(x)] = static_cast<int>(is_int);
				continue;
			}
			// 浮点型
			extract<float> is_float(value);
			if (is_float.check()) {
				connect_options_map[static_cast<char*>(x)] = static_cast<float>(is_float);
				continue;
			}
			// 布尔型
			extract<bool> is_bool(value);
			if (is_bool.check()) {
				connect_options_map[static_cast<char*>(x)] = static_cast<bool>(is_bool);
			}
			throw invalid_argument("can't match the type of key:" + string(x));
		}
		return connect_options_map;
	}
	catch (exception e) {
		throw invalid_argument(e.what());
	}


}

// 关闭连接
void MyConnection::close() const {
	this->conn->close();
}

// 执行SQL语句
dict MyConnection::execute(char* sql) {
	try {
		Statement* stmt = this->conn->createStatement();
		dict res_dict;
		try {
			const bool is_select = stmt->execute(sql);

			if (is_select) {
				ResultSet* res_set = stmt->getResultSet();
				res_dict = this->getResDict(res_set);
				delete res_set;
				printLog("Execute SQL:\t" + string(sql) + "\tsuccessfully", STATUS);
			}
			else {
				printLog("Execute SQL:\t" + string(sql) + "\tsuccessfully\t", STATUS);
				res_dict["res"] = true;
			}
			delete stmt;
			return res_dict;
		}
		catch (SQLException e) {
			if (this->savepoint_) {
				this->conn->rollback(this->savepoint_);
				this->conn->releaseSavepoint(this->savepoint_);
				this->savepoint_ = nullptr;
				this->conn->setAutoCommit(this->autocommit);
			}
			printLog(e.what(), SQL_ERROR);
			// throw SQLException(e.what());
		}
	}
	catch (exception e) {
		if (this->savepoint_) {
			this->conn->rollback(this->savepoint_);
			this->conn->releaseSavepoint(this->savepoint_);
			this->savepoint_ = nullptr;
			this->conn->setAutoCommit(this->autocommit);
		}
		throw runtime_error(e.what());
	}
	dict dic_r;
	dic_r["res"] = false;
	return dic_r;
}

// 获得结果集dict（private内部方法）
dict MyConnection::getResDict(ResultSet* res) {
	dict dic;
	if (res) {
		ResultSetMetaData* meta_data = res->getMetaData();
		b_list field_name;
		int field_count = meta_data->getColumnCount();
		for (int i = 0; i < field_count; i++) {
			string o = string(meta_data->getColumnName(i + 1)) + ":" + string(meta_data->getColumnTypeName(i + 1));
			field_name.append(o.c_str());
		}
		dic["columnCount"] = field_count;
		dic["columnNameAndType"] = field_name;
		dic["row_count"] = res->rowsCount();
		b_list row_list;
		while (res->next()) {
			b_list each;
			for (int j = 0; j < field_count; j++) {
				each.append(res->getString(j + 1).c_str());
			}
			row_list.append(each);
		}

		dic["res"] = row_list;
	}
	else {
		dic["res"] = false;
	}
	return dic;
}

// 预编译方式执行SQL语句
dict MyConnection::execute_stmt(char* sql_stmt, b_list key) {
	try {
		PreparedStatement* pre_stmt;

		ResultSet* res;
		dict res_dic;
		printLog("Analysis SQL: " + string(sql_stmt), INFO);
		string log_str = string(sql_stmt) + "";
		try {
			pre_stmt = this->conn->prepareStatement(sql_stmt);
			// 根据数据类型来进行赋值，尽量避免了MYSQL隐式类型转换，隐式类型转换会造成查询不走索引（如果定义了索引）
			for (int i = 0; i < len(key); i++) {
				extract<char*> is_char(key[i]);
				if (is_char.check()) {
					char* ob = static_cast<char*>(is_char);
					cout << "param: " << ob << "\ttype: VARCHAR" << endl;;
					pre_stmt->setString(i + 1, ob);
					continue;
				}
				extract<int> is_int(key[i]);
				if (is_int.check()) {
					int ob = static_cast<int>(is_int);
					cout << "param: " << ob << "\ttype: INT" << endl;;
					pre_stmt->setInt64(i + 1, ob);
					continue;
				}
				extract<Double> is_double(key[i]);
				if (is_double.check()) {
					double ob = static_cast<Double>(is_double).value;
					cout << "param: " << ob << "\ttype: DOUBLE" << endl;
					pre_stmt->setDouble(i + 1, ob);
					continue;
				}
				extract<DateTime> is_date(key[i]);
				if (is_date.check()) {
					const char* ob = static_cast<DateTime>(is_date).time.c_str();
					cout << "param: " << ob << "\ttype: DateTime" << endl;
					pre_stmt->setDateTime(i + 1, ob);
					continue;
				}
				if (((object)key[i]).is_none()) {
					pre_stmt->setNull(i, 0);
					cout << "param: None" << "\ttype: Null" << endl;
					continue;
				}
				extract<Int64> is_int64(key[i]);
				if (is_int64.check()) {
					int64_t ob = static_cast<Int64>(is_int64).value;
					cout << "param: " << ob << "\ttype: INT64" << endl;
					pre_stmt->setInt64(i + 1, ob);
					continue;
				}
				extract<BigInt> is_BigInt(key[i]);
				if (is_BigInt.check()) {
					int ob = static_cast<BigInt>(is_BigInt).value;
					cout << "param: " << ob << "\ttype: BigInt";
					pre_stmt->setBigInt(i + 1, std::to_string(ob));
					continue;
				}
				extract<bool> is_bool(key[i]);
				if (is_bool.check()) {
					bool ob = static_cast<bool>(is_bool);
					cout << "param: ";
					ob == 1 ? cout << "True\ttype: Boolean" << endl : cout << "False\ttype: Boolean" << endl;
					pre_stmt->setBoolean(i + 1, ob);
					continue;
				}
				extract<float> is_float(key[i]);
				if (is_float.check()) {
					double ob = static_cast<double>(is_float);
					cout << "param: " << ob << "\ttype: Float (to Double instead)" << endl;
					pre_stmt->setDouble(i + 1, ob);
					continue;
				}
				throw invalid_argument("illegal param type at index: " + i + 1);
			}
			// 执行
			bool is_select = pre_stmt->execute();
			// 根据结果集类型来返回数据（查询语句 vs 非查询语句）
			if (is_select) {
				res = pre_stmt->getResultSet();
				res_dic = getResDict(res);

				printLog("Execute Prepare SQL:\t" + string(sql_stmt) + "\tsuccessfully", INFO);
			}
			else {
				printLog("Execute Prepare SQL:\t" + string(sql_stmt) + "\tsuccessfully\t ", INFO);

				// printLog(string(sql_stmt) + "\tsuccessfully\t affect rows:", INFO);
				res_dic["res"] = "successfully";
			}

			delete pre_stmt;
			return res_dic;
		}
		catch (SQLException e) {
			printLog(e.what(), SQL_ERROR);
			if (this->savepoint_) {
				this->conn->rollback(this->savepoint_);
				this->conn->releaseSavepoint(this->savepoint_);
				this->savepoint_ = nullptr;
				this->conn->setAutoCommit(this->autocommit);
			}
			// throw SQLException(e.what());
		}
	}
	catch (exception e) {
		if (this->savepoint_) {
			this->conn->rollback(this->savepoint_);
			this->conn->releaseSavepoint(this->savepoint_);
			this->savepoint_ = nullptr;
			this->conn->setAutoCommit(this->autocommit);
		}
		throw runtime_error(e.what());
	}
	return {};
}

// 打印日志
void MyConnection::printLog(string log, LogType type) {
	if (this->isLog) {
		time_t now = time(nullptr);
		char now_time[26];
		ctime_s(now_time, sizeof now_time, &now);
		CString st = now_time;
		st.Remove('\n');
		cout << st << "\t";
		switch (type) {
		case STATUS: cout << "STATUS\t";
			break;
		case INFO: cout << "INFO\t";
			break;
		case _ERROR: cout << "ERROR\t";
			break;
		case FATAL: cout << "FATAL\t";
			break;
		case WARNING: cout << "WARNING\t";
			break;
		case SQL_ERROR: cout << "SQL ERROR\t";
		}
		cout << log << endl;
	}

}

// 打印连接信息
void MyConnection::print_conn_info() {
	if ((!this->conn) || this->conn->isClosed()) {
		cout << "the connection is None or closed" << endl;
		return;
	}
	cout << "The Connection is as followed:" << endl;
	DatabaseMetaData* dbcon_meta = this->conn->getMetaData();
	cout << "Database Product Name: " << dbcon_meta->getDatabaseProductName() << endl;
	cout << "Database Product Version: " << dbcon_meta->getDatabaseProductVersion() << endl;
	cout << "Database User Name: " << dbcon_meta->getUserName() << endl << endl;

	cout << "Driver name: " << dbcon_meta->getDriverName() << endl;
	cout << "Driver version: " << dbcon_meta->getDriverVersion() << endl << endl;

	cout << "Database in Read-Only Mode?: " << dbcon_meta->isReadOnly() << endl;
	cout << "Supports Transactions?: " << dbcon_meta->supportsTransactions() << endl;
	cout << "Supports DML Transactions only?: " << dbcon_meta->supportsDataManipulationTransactionsOnly() << endl;
	cout << "Supports Batch Updates?: " << dbcon_meta->supportsBatchUpdates() << endl;
	cout << "Supports Outer Joins?: " << dbcon_meta->supportsOuterJoins() << endl;
	cout << "Supports Multiple Transactions?: " << dbcon_meta->supportsMultipleTransactions() << endl;
	cout << "Supports Named Parameters?: " << dbcon_meta->supportsNamedParameters() << endl;
	cout << "Supports Statement Pooling?: " << dbcon_meta->supportsStatementPooling() << endl;
	cout << "Supports Stored Procedures?: " << dbcon_meta->supportsStoredProcedures() << endl;
	cout << "Supports Union?: " << dbcon_meta->supportsUnion() << endl << endl;

	cout << "Maximum Connections: " << dbcon_meta->getMaxConnections() << endl;
	cout << "Maximum Columns per Table: " << dbcon_meta->getMaxColumnsInTable() << endl;
	cout << "Maximum Columns per Index: " << dbcon_meta->getMaxColumnsInIndex() << endl;
	cout << "Maximum Row Size per Table: " << dbcon_meta->getMaxRowSize() << " bytes" << endl;

	
}

// 设置是否自动提交
void MyConnection::setAutoCommit(bool autocommit) {
	this->autocommit = autocommit;
	this->conn->setAutoCommit(autocommit);
}

// 设置是否打印日志
void MyConnection::setIsLog(bool isLog) {
	this->isLog = isLog;
}

// 切换数据库
bool MyConnection::switch_database(char* database) {
	try {
		const char* now = this->conn->getSchema().c_str();
		if (now == database) {
			cout << "The database is in used now, it is no need to switch." << endl;
			return false;
		}
		this->conn->setSchema(database);
		printLog("Switch to " + string(database), INFO);
		return true;
	}
	catch (SQLException e) {
		printLog("Switch to " + string(database) + " with failure, cause" + e.what(), FATAL);
		return false;
	}
}

void printOptHelp() {
	cout << "please visit https://dev.mysql.com/doc/connector-cpp/1.1/en/connector-cpp-connect-options.html to get help of the Connection Option" << endl;
	cout << "you just need to focus on the \"parameter\" and the type of value" << endl;
}

void MyConnection::commit() {
	if (!this->conn->getAutoCommit()) {
		this->conn->commit();
		this->savepoint_ = NULL;
	}
	if (this->conn->getAutoCommit() != this->autocommit) {
		this->setAutoCommit(this->autocommit);
		printLog("convert AutoCommit to the user Option: " + to_string(this->autocommit), INFO);
	}

}
b_list MyConnection::getAutoCommitStatus() {
	cout << "the AutoCommit Status is as followed:" << endl;
	cout << "user option:" << (this->autocommit == true ? "True" : "False") << endl;
	cout << "MySQL AutoCommitStatus:" << (this->conn->getAutoCommit() == true ? "True" : "False") << endl;
	b_list re;
	re.append(this->autocommit);
	re.append(this->conn->getAutoCommit());
	return re;
}

// 创建还原点
bool MyConnection::createSavePoint(char* name) {
	if (this->savepoint_) {
		throw runtime_error("the last SavePoint has not been released, pleased commit or rollback.");
	}
	try {
		this->conn->setAutoCommit(0);
		printLog("convert AutoCommit Option to False(0)", INFO);
		this->savepoint_ = this->conn->setSavepoint(name);
		printLog("Setting SavePoint named: " + string(this->savepoint_->getSavepointName()), INFO);
		return true;
	}
	catch (exception e) {
		if (!this->savepoint_) {
			this->savepoint_ = nullptr;
		}
		this->conn->setAutoCommit(this->autocommit);
		throw exception(e.what());
	}
	return false; // should not go to here

}

bool MyConnection::rollback() {
	if (this->savepoint_) {
		try {
			const char* point_name = (this->savepoint_->getSavepointName()).c_str();
			this->conn->rollback(this->savepoint_);
			printLog("rollback to SavePoint:\"" + string(point_name) + "\"", INFO);
			this->conn->releaseSavepoint(this->savepoint_);
			printLog("release the SavePoint \"" + string(point_name) + "\"", INFO);
			this->savepoint_ = nullptr;
			printLog("rollback and release successfully", INFO);
			if (this->conn->getAutoCommit() != this->autocommit) {
				this->conn->setAutoCommit(this->autocommit);
				printLog("converting AutoCommit setting to user setting: True", INFO);
			}
			return true;
		}
		catch (exception e) {
			printLog(e.what(), FATAL);
			throw exception(e.what());
		}
	}
	else {
		try {
			this->conn->rollback();
			return true;
		}
		catch (exception e) {
			this->printLog(e.what(), FATAL);
			throw runtime_error(e.what());
		}
	}
}

BOOST_PYTHON_MODULE(PyMySQLConnector) {
	class_<MyConnection>("Connection", "init", init<string, string, string, string, int, dict, Option>())
		.def(init<dict, Option>())
		.def(init<string, string, string, string, Option>())
		.def("close", &MyConnection::close)
		.def("commit", &MyConnection::commit)
		.def("setSavePoint", &MyConnection::createSavePoint)
		.def("rollback", &MyConnection::rollback)
		.def("switchDatabase", &MyConnection::switch_database)
		.def("setIsLog", &MyConnection::setIsLog)
		.def("setAutoCommit", &MyConnection::setAutoCommit)
		.def("printConnInfo", &MyConnection::print_conn_info)
		.def("execute", &MyConnection::execute)
		.def("execute_stmt", &MyConnection::execute_stmt)
		.def("printOptHelp", printOptHelp)
		.def("getAutoCommitStatus", &MyConnection::getAutoCommitStatus)
		.staticmethod("printOptHelp");
	class_<Option>("Option", "Option", init<bool, bool>());
	class_<Double>("Double", "Double", init<float>());
	class_<Int64>("Int64", "Int64", init<int>());
	class_<DateTime>("DateTime", "DateTime", init<string>());
}
