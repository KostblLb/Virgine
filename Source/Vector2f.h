#include <cmath>
#include <algorithm>

class Vector2f{
private:
	float m_x;
	float m_y;
public:
	Vector2f() :m_x(0.0), m_y(0.0){}
	Vector2f(float _x, float _y) :m_x(_x), m_y(_y){}
	Vector2f(Vector2f &other) : m_x(other.m_x), m_y(other.m_y){}
	~Vector2f(){}

	float x(){ return m_x; }
	float y(){ return m_y; }
	void x(float t){ m_x = t; }
	void y(float t){ m_y = t; }

	void swap(Vector2f &one, Vector2f &another){
		using std::swap;
		swap(one.m_x, another.m_x);
		swap(one.m_y, another.m_y);
	}

	Vector2f operator+(Vector2f &v2){
		return Vector2f(this->m_x + v2.m_x, this->m_y + v2.m_y);
	}
	Vector2f operator-(Vector2f &v2){
		return Vector2f(this->m_x - v2.m_x, this->m_y - v2.m_y);
	}
	Vector2f operator*(Vector2f &v2){
		return Vector2f(this->m_x*v2.m_x, this->m_y*v2.m_y);
	}
	Vector2f& operator=(Vector2f v2){///////////////// cool stuff
		swap(*this, v2);
		return *this;
	}
	Vector2f& operator+=(Vector2f &v2){
		*this = (*this + v2);
		return *this;
	}
	Vector2f& operator*=(Vector2f &v2){
		*this = (*this * v2);
		return *this;
	}
	Vector2f& operator-=(Vector2f &v2){
		*this = (*this - v2);
		return *this;
	}

	float norma(){
		return sqrtf((m_x*m_x) + (m_y*m_y));
	}
	Vector2f normalize(){ //x/|x|
		return Vector2f(this->m_x / norma(), this->m_y / norma());
	}
};