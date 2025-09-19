#pragma once

#include <iostream>


class vect2 {
private:
	int x;
	int y;
public:
	vect2() : x(0), y(0) {}
	vect2(int _x, int _y) : x(_x), y(_y) {}
	~vect2() {}
	vect2(const vect2& copy);
	vect2& operator=(const vect2& other);
	int operator[](int idx) const ;
	vect2 operator++(int);
	vect2& operator++();
	vect2 operator--(int);
	vect2& operator--();
	void operator+=(const vect2& v);
	vect2 operator+(const vect2& v);
	void operator-=(const vect2& v);
	vect2 operator-(const vect2& v);
	vect2 operator*(int nb);
	void operator*=(const vect2& v);
	// void operator+();
	// void operator+();
};

vect2::vect2(const vect2 &copy) {
	*this = copy;
}

vect2& vect2::operator=(const vect2 &other) {
	if (this != &other) {
		this->x = other.x;
		this->y = other.y;
	}
	return *this;
}