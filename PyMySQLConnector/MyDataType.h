#pragma once

// 日期类型
class DateTime {
public:
	string time;

	DateTime(string time) {
		this->time = time;
	}
};

class Double {
public:
	double value;

	Double(double value) {
		this->value = value;
	}
};


class Int64 {
public:
	int64_t value;

	Int64(int64_t value) {
		this->value = value;
	}
};

class BigInt {
public:
	int value;

	BigInt(int value) {

		this->value = value;
	}


};
