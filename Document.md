# 概述

使用C++结合MySQL Connector/C++ 1.1.13编写的Python连接MySQL的库。

功能：

+ 执行SQL语句
+ 执行预编译SQL语句

# 基本使用

## 初始化连接

+ 导入模块：

```python
from PyMySQLConnector import *
```

+ 创建连接：

该模块构造函数有三种方式：

```python
conn = Connection(host: str, username: str, password: str, database: str, port: int, opt_dic: dict, opt: Option)
```

```python
conn = Connection(url: str, username: str, password: str, database: str, opt_dic: dict, opt: Option)
```

```python
conn = Connection(opt_dic: dict, opt: Option)
```

在构造函数中，opy_dict是对连接进行核心配置，如果不在构造函数内传入用户凭证信息，则凭证信息必须由该字典传入。包括userName，password，port，schema，host等等，有关其他选项，例如字符集之类的，请参阅：

[Connection Options 配置]: https://dev.mysql.com/doc/connector-cpp/1.1/en/connector-cpp-connect-options.html	"MySQL :: MySQL Connector/C++ 1.1 Developer Guide :: 10 Connector/C++ Connection Options"

用户可以自行进行网页翻译，往下翻可以看到所有可以用到的字段名称，注意区分大小写，一定要注意字段值的类型，是数值，布尔还是字符串。个别字段无法修改。

+ 构造函数中的opt：

为类Option，需要创建Option对象。

构造函数：

```python
opt = Option(isLog: bool, autocommit: bool)
```

isLog：是否打印日志

autocommit：是否自动提交

## 执行SQL

### 普通方式执行SQL

不支持多条SQL语句执行，支持事务，但事务相关提交回滚等等需要自行在MySQL中定义。

```python
res = conn.execute(sql: str)
```

该方法返回字典。

+ 字典结构

1、查询语句：

columnCount：字段数

columnNameAndType：字段名称和类型

row_count：结果行数

res：结果集，是一个二维列表，每一行是一个列表，每个列表内的元素均为字符串形式，用户在使用数据时需要自行转换类型。

2、非查询语句

首MySQL Connector/C++ 1.1 API的限制，非查询语句不能获取受影响的行数，只能返回成功与失败

### 预编译语句

不支持多条语句。

```python
res = execute_stmt(sql: str, param: list)
```

param：参数列表，注意数据类型，防止不走索引，注意参数个数与占位符数量相同。

**注意：**在使用MySQL特有数据类型时，需要使用库内定义的数据类型类，详见：方法说明章节的数据类型。

参数暂不支持二进制Blob类型。用户不要轻易尝试，以免发生难以预测的错误。

### 提交

非自动提交情况下：

```python
conn.commit()
```

### 回滚

非自动提交情况下：

```python
conn.commit()
```

# 非自动提交情况下的使用

※ 类似事务操作。

※ 基本步骤：

+ 创建回滚保存点（整个连接对象只能创建一个）
+ 执行SQL语句：执行成功，则继续下一条；执行失败，回滚至回滚保存点。
+ 提交/回滚

※ 回滚保存点生命周期：

用户创建——>执行SQL——>执行SQL——>……——>用户提交/用户回滚——>释放——>用户创建...

用户创建——>执行SQL——>执行SQL（发送错误）——>回滚——>释放——>用户创建...

### 创建回滚保存点

```python
conn.setSavePoint(name: str)
```

name：回滚保存点名称（没有实际意义）

注意：整个连接对象的生命周期中只能存在一个回滚保存点，只有在执行commit()或rollback()或SQL执行失败后才能再次创建。



### 执行SQL语句

包括execute和execute_stmt，执行时提交模式为非自动提交。

在执行SQL语句时，若发生错误，则自动回滚至回滚保存点，回滚保存点释放。

### 提交/回滚

```python
conn.commit()
```

```python
conn.rollback()
```

执行后，回滚保存点释放，数据修改提交至数据库。

## 关闭连接

```python
conn.close()
```



# 方法说明

## Connection类

+ setIsLog()

```python
x.setIsLog(isLog: bool)
```

设置是否打印日志

+ switchDatabase()

```python
x.switchDatabase(database: str)
```

切换操作的数据库

+ printConnInfo()

```python
x.printConnInfo()
```

打印连接信息。

+ printOptHelp()

```python
x.printOptHelp()
```

打印MySQL配置字典帮助信息

## 数据类型

除了python内置基本数据类型可以与MySQL的数据类型相对应，MySQL内的一些数据类型在python中无法得到对应，故建立了相应的数据类型的Class，内部处理后来与之进行匹配。

注意：创建对象时构造函数的参数数据类型必须完全匹配，否则飘红报错。

+ Double类：

    对应double数据类型

    ```python
    Double(value: int)
    ```

+ BigInt类：

    对应BigInt数据类型

    ```python
    BigInt(value: int)
    ```

+ Int64类：

    对应int64类型

    ```python
    Int64(value: int)
    ```

+ DateTime类：

    对应DateTime日期类型

    ```python
    DateTime(datetime: str)
    ```

    