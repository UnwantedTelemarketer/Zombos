#pragma once
#include <random>
#include <tuple>
#include <sys/stat.h>

#define maybe (rand() % 10 + 1) >= 5

namespace Math {
	//Input the max number, and this will return a float between 1 and your max (inclusive).
	inline float RandNum(int max) { return (rand() % max + 1); }

	//Input the max number, and this will return an int between min and max (exclusive).
	inline int RandInt(int min, int max) { return min + (rand() % max - min); }

	//Returns a random string from a vector of strings.
	inline std::string RandString(std::vector<std::string> list) { return list[rand() % (list.size() - 1) + 1]; } //also bad to look at, but i dont want to make a multiline function in a header

	//Limits the size of a vector of strings to 15, if anything goes over that amount, the oldest element is deleted and the new one is added.
	inline void PushBackLog(std::vector<std::string>* log, std::string message, int max = 15) {
		if (log->size() <= max) { log->push_back(message); }
		else { log->erase(log->begin()); log->push_back(message); }
	}
}

struct Vector2_B {
	uint32_t coords = 0;

	int getX() { return coords & 0xffff; }

	int getY() { return (coords >> 16) & 0xffff; }

	void setX(uint16_t x) { coords = coords & 0xffff0000 | x; }

	void setY(uint16_t y) {
		coords &= 0xffff;
		coords |= (y & 0xffff) << 16;
	}

	void setCoords(int x, int y) { coords = (x & 0xffff) | ((y & 0xffff) << 16); }
};

struct Vector2_I
{
	int x, y;

	//Serialization
	/*friend std::ostream& operator<<(std::ostream& stream, const Vector2_I& vec2) {
		stream << vec2.x << ' ' << vec2.y;
		return stream;
	}

	friend std::istream& operator<<(std::istream& stream, Vector2_I& vec2) {
		stream >> vec2.x >> vec2.y;
		return stream;
	}*/

	bool operator<(const Vector2_I& rhs) const {
		return std::tie(x, y) < std::tie(rhs.x, rhs.y);
	}

	bool operator==(const Vector2_I& rhs) const {
		return std::tie(x, y) == std::tie(rhs.x, rhs.y);
	}

	bool operator!=(Vector2_I vec2) {
		return (vec2.x == x && vec2.y == y);
	}
	void operator+=(Vector2_I vec2) {
		x += vec2.x;
		y += vec2.y;
	}
	Vector2_I operator+(Vector2_I vec2) {
		return { x + vec2.x, y + vec2.y };
	}
	bool operator<(Vector2_I vec2) {
		return (x < vec2.x&& y < vec2.y);
	}

	/*void operator=(Vector2_I vec2) {
		x = vec2.x;
		y = vec2.y;
	}*/
};

struct Vector2
{
	float x, y;

	bool operator==(Vector2 vec2) {
		return (vec2.x == x && vec2.y == y);
	}

	bool operator!=(Vector2 vec2) {
		return (vec2.x == x && vec2.y == y);
	}
};

struct Vector3
{
	float x, y, z;

	bool operator==(Vector3 vec3) {
		if (vec3.x == x && vec3.y == y && vec3.z == z) { return true; }
		return false;
	}

	bool operator!=(Vector3 vec3) {
		if (vec3.x == x && vec3.y == y && vec3.z == z) { return false; }
		return true;
	}

	void operator+=(Vector3 vec3) {
		x += vec3.x;
		y += vec3.y;
		z += vec3.z;
	}

	void operator/=(int num) {
		x /= num;
		y /= num;
		z /= num;
	}

};

struct Vector4
{
	float x, y, z, w;
	bool operator==(Vector4 vec4) {
		if (vec4.x == x && vec4.y == y && vec4.z == z && vec4.w == w) { return true; }
		return false;
	}

	bool operator!=(Vector4 vec4) {
		if (vec4.x == x && vec4.y == y && vec4.z == z && vec4.w == w) { return false; }
		return true;
	}

	void operator+=(Vector3 vec3) {
		x += vec3.x;
		y += vec3.y;
		z += vec3.z;
	}

	void operator/=(int num) {
		x /= num;
		y /= num;
		z /= num;
		w /= num;
	}

	//for maps
	bool operator<(const Vector4& other) const {
		if (x < other.x) return true;
		if (x > other.x) return false;
		if (y < other.y) return true;
		if (y > other.y) return false;
		if (z < other.z) return true;
		if (z > other.z) return false;
		return w < other.w;
	}
};

struct Vector4_I
{
	int x, y, z, w;
	bool operator==(Vector4_I vec4) {
		if (vec4.x == x && vec4.y == y && vec4.z == z && vec4.w == w) { return true; }
		return false;
	}

	bool operator!=(Vector4_I vec4) {
		if (vec4.x == x && vec4.y == y && vec4.z == z && vec4.w == w) { return false; }
		return true;
	}

	void operator+=(Vector3 vec3) {
		x += vec3.x;
		y += vec3.y;
		z += vec3.z;
	}

	void operator/=(int num) {
		x /= num;
		y /= num;
		z /= num;
		w /= num;
	}

	//for maps
	bool operator<(const Vector4_I& other) const {
		if (x < other.x) return true;
		if (x > other.x) return false;
		if (y < other.y) return true;
		if (y > other.y) return false;
		if (z < other.z) return true;
		if (z > other.z) return false;
		return w < other.w;
	}
};

struct Vector26
{
	float a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z;
};

typedef Vector2_I vec2_i;
typedef Vector2 vec2;
typedef Vector3 vec3;
typedef Vector4 vec4;
typedef Vector3 Color;