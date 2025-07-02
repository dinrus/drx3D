#pragma once

#include <drx3D/Maths/Color.h>

#include <drx3D/Files/Node.h>

namespace drx3d {
constexpr bool Color::operator==(const Color &rhs) const {
	return r == rhs.r && g == rhs.g && b == rhs.b && a == rhs.a;
}

constexpr bool Color::operator!=(const Color &rhs) const {
	return !operator==(rhs);
}

constexpr Color operator+(const Color &lhs, const Color &rhs) {
	return {lhs.r + rhs.r, lhs.g + rhs.g, lhs.b + rhs.b, lhs.a + rhs.a};
}

constexpr Color operator-(const Color &lhs, const Color &rhs) {
	return {lhs.r - rhs.r, lhs.g - rhs.g, lhs.b - rhs.b, lhs.a - rhs.a};
}

constexpr Color operator*(const Color &lhs, const Color &rhs) {
	return {lhs.r * rhs.r, lhs.g * rhs.g, lhs.b * rhs.b, lhs.a * rhs.a};
}

constexpr Color operator/(const Color &lhs, const Color &rhs) {
	return {lhs.r / rhs.r, lhs.g / rhs.g, lhs.b / rhs.b, lhs.a / rhs.a};
}

constexpr Color operator+(float lhs, const Color &rhs) {
	return Color(lhs, lhs, lhs, 0.0f) + rhs;
}

constexpr Color operator-(float lhs, const Color &rhs) {
	return Color(lhs, lhs, lhs, 0.0f) - rhs;
}

constexpr Color operator*(float lhs, const Color &rhs) {
	return Color(lhs, lhs, lhs) * rhs;
}

constexpr Color operator/(float lhs, const Color &rhs) {
	return Color(lhs, lhs, lhs) / rhs;
}

constexpr Color operator+(const Color &lhs, float rhs) {
	return lhs + Color(rhs, rhs, rhs, 0.0f);
}

constexpr Color operator-(const Color &lhs, float rhs) {
	return lhs - Color(rhs, rhs, rhs, 0.0f);
}

constexpr Color operator*(const Color &lhs, float rhs) {
	return lhs * Color(rhs, rhs, rhs);
}

constexpr Color operator/(const Color &lhs, float rhs) {
	return lhs / Color(rhs, rhs, rhs);
}

constexpr Color &Color::operator+=(const Color &rhs) {
	return *this = *this + rhs;
}

constexpr Color &Color::operator-=(const Color &rhs) {
	return *this = *this - rhs;
}

constexpr Color &Color::operator*=(const Color &rhs) {
	return *this = *this * rhs;
}

constexpr Color &Color::operator/=(const Color &rhs) {
	return *this = *this / rhs;
}

constexpr Color &Color::operator+=(float rhs) {
	return *this = *this + rhs;
}

constexpr Color &Color::operator-=(float rhs) {
	return *this = *this - rhs;
}

constexpr Color &Color::operator*=(float rhs) {
	return *this = *this * rhs;
}

constexpr Color &Color::operator/=(float rhs) {
	return *this = *this / rhs;
}

inline const Node &operator>>(const Node &node, Color &color) {
	// Loads from hex if RGBA is not provided.
	if (node.GetProperties().empty()) {
		std::string hex;
		node >> hex;
		color = hex;
	} else {
		node["r"].Get(color.r);
		node["g"].Get(color.g);
		node["b"].Get(color.b);
		node["a"].Get(color.a);
	}

	return node;
}

inline Node &operator<<(Node &node, const Color &color) {
	//node << color.GetHex();
	node["r"].Set(color.r);
	node["g"].Set(color.g);
	node["b"].Set(color.b);
	node["a"].Set(color.a);
	return node;
}

inline std::ostream &operator<<(std::ostream &stream, const Color &color) {
	return stream << color.r << ", " << color.g << ", " << color.b << ", " << color.a;
}
}

namespace std {
template<>
struct hash<drx3d::Color> {
	size_t operator()(const drx3d::Color &color) const noexcept {
		size_t seed = 0;
		drx3d::Maths::HashCombine(seed, color.r);
		drx3d::Maths::HashCombine(seed, color.g);
		drx3d::Maths::HashCombine(seed, color.b);
		drx3d::Maths::HashCombine(seed, color.a);
		return seed;
	}
};
}
