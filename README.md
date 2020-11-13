# 项目概况

这是一个Python连接MySQL的库，采用c++编写。利用Boost.Python C++库，将C++的函数或类进行包装，暴露给Python进行import调用。

使用帮助文档：参加Document.md

# 最新

+ SQL执行语句完善

    1）普通方式执行SQL语句；

    2）预编译方式执行SQL语句；

    3）支持手动提交与回滚（非自动提交模式）；

    4）支持事务，详见回滚保存点；

    5）不支持一次性多条语句，如需多条语句执行，请使用事务（创建回滚保存点）；

+ 完善了日志系统：

    1）定义了日志级别：STATUS、INFO、FATAL、WARNING、ERROR、SQL_ERROR；

    2）固定了日志显示格式；

    3）SQL语句出现错误时，会显示MySQL返回的SQL错误信息；

    4）用户可设置是否显示日志；

+ 完善了自动提交：

    1）用户可自定义是否自动提交更新；

    2）用户可在python内通过commit()函数来手动提交更新；

+ 内部优化：

    1）提升了稳定性

    2）在事务处理时，如发生错误，自动回滚至回滚保存点

# 环境

编程环境：Boost 1.74.0，Python3.7.7，Windows10，MySQL 8.0、MySQLConnector/C++ 1.1

编译环境：vs2019

# vs2019编译配置

1、标准：latest；符合标准：否；

2、字符集：使用多字节字符集

3、头文件包含目录：python安装目录/include、mysql安装目录/include、boost头文件目录、MySQLConnector1.1解压目录/include

4、库目录：python安装目录/libs、mysql安装目录/lib、boost库文件目录、MySQLConnector1.1解压目录/lib

5、包含的库：libmysql.lib、python37.lib、mysqlcppconn.lib

6、c++命令行：/utf-8

# 安装

将生成的dll扩展名更改为pyd，将仓库根目录内的libmysql.dll和mysqlcppconn.dll连同pyd文件一同复制到python安装目录内Dlls文件夹内，这三个文件务必在同一文件夹，否则无法使用。

# 联系

如有问题，请联系我。（废话）

本人新人，代码极为青涩，有任何建议欢迎提出。

嘻嘻嘻。