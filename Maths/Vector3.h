#pragma once

#include <cstdint>
#include <type_traits>

#include <drx3D/Export.h>

namespace drx3d {
template<typename T>
class Vector2;

template<typename T>
class Vector3;

template<typename T>
class Vector4;

using Vector3f = Vector3<float>;
using Vector3d = Vector3<double>;
using Vector3i = Vector3<int32_t>;
using Vector3ui = Vector3<uint32_t>;

/**
 * @brief Содержит вектор с тремя кортежами.
 * @tparam T Тип значения.
 */
template<typename T>
class Vector3 {
public:
	/**
	 * Конструктор класса Vector3.
	 */
	constexpr Vector3() = default;

	/**
	 * Конструктор класса Vector3.
	 * @param Значение, в которое устанавливаются все компоненты.
	 */
	constexpr explicit Vector3(const T &a);

	/**
	 * Конструктор класса Vector3.
	 * @param x Старт x.
	 * @param y Старт y.
	 * @param z Старт z.
	 */
	constexpr Vector3(const T &x, const T &y, const T &z);

	/**
	 * Конструктор класса Vector3.
	 * @tparam K Тип x.
	 * @tparam J Тип y.
	 * @tparam H Тип z.
	 * @param x Старт x.
	 * @param y Старт y.
	 * @param z Старт z.
	 */
	template<typename K, typename J, typename H>
	constexpr Vector3(const K &x, const J &y, const H &z);

	/**
	 * Конструктор класса Vector3.
	 * @tparam K Тип источника.
	 * @tparam J Тип z.
	 * @param source Создаёт этот вектор из существующего.
	 * @param z Старт z.
	 */
	template<typename K, typename J = T>
	explicit constexpr Vector3(const Vector2<K> &source, const J &z = 0);

	/**
	 * Конструктор класса Vector3.
	 * @tparam K Тип источника.
	 * @param source Создаёт этот вектор из существующего.
	 */
	template<typename K>
	constexpr Vector3(const Vector3<K> &source);

	/**
	 * Конструктор класса Vector3.
	 * @tparam K Тип источника.
	 * @param source Создаёт этот вектор из существующего.
	 */
	template<typename K>
	constexpr Vector3(const Vector4<K> &source);

	/**
	 * Слаживает этот вектор с другим.
	 * @tparam K Тип другого.
	 * @param other Другой вектор.
	 * @return Итоговый вектор.
	 */
	template<typename K>
	constexpr auto Add(const Vector3<K> &other) const;

	/**
	 * Вычитает этот вектор из другого.
	 * @tparam K Тип другого.
	 * @param other Другой вектор.
	 * @return Итоговый вектор.
	 */
	template<typename K>
	constexpr auto Subtract(const Vector3<K> &other) const;

	/**
	 * Умножвет этот вектор на другой.
	 * @tparam K Тип другого.
	 * @param other Другой вектор.
	 * @return Итоговый вектор.
	 */
	template<typename K>
	constexpr auto Multiply(const Vector3<K> &other) const;

	/**
	 * Делит этот вектор на другой.
	 * @tparam K Тип другого.
	 * @param other Другой вектор.
	 * @return Итоговый вектор.
	 */
	template<typename K>
	constexpr auto Divide(const Vector3<K> &other) const;

	/**
	 * Вычисляет угол между этим и другим вектором.
	 * @tparam K Тип другого.
	 * @param other Другой вектор.
	 * @return Угол, в радианах.
	 */
	template<typename K>
	auto Angle(const Vector3<K> &other) const;

	/**
	 * Calculates the dot product of the this vector and another vector.
	 * @tparam K Тип другого.
	 * @param other Другой вектор.
	 * @return The dot product.
	 */
	template<typename K>
	constexpr auto Dot(const Vector3<K> &other) const;

	/**
	 * Calculates the cross product of the this vector and another vector.
	 * @tparam K Тип другого.
	 * @param other Другой вектор.
	 * @return The cross product.
	 */
	template<typename K>
	constexpr auto Cross(const Vector3<K> &other) const;

	/**
	 * Calculates the linear interpolation between this vector and another vector.
	 * @tparam K The others type.
	 * @tparam J The progression type.
	 * @param other The other vector.
	 * @param progression The progression.
	 * @return Left lerp right.
	 */
	template<typename K, typename J = float>
	constexpr auto Lerp(const Vector3<K> &other, const J &progression) const;

	/**
	 * Scales this vector by a scalar.
	 * @tparam K The scalar type.
	 * @param scalar The scalar value.
	 * @return The scaled vector.
	 */
	template<typename K = float>
	constexpr auto Scale(const K &scalar) const;

	/**
	 * Rotates this vector by a angle around the origin.
	 * @tparam K The rotations type.
	 * @param angle The angle to rotate by, in radians.
	 * @return The rotated vector.
	 */
	//template<typename K = float>
	//auto Rotate(const Vector3<K> &angle) const;

	/**
	 * Normalizes this vector.
	 * @return The normalized vector.
	 */
	auto Normalize() const;

	/**
	 * Gets the length squared of this vector.
	 * @return The length squared.
	 */
	constexpr auto LengthSquared() const;

	/**
	 * Gets the length of this vector.
	 * @return The length.
	 */
	auto Length() const;

	/**
	 * Gets the absolute value of every component in this vector.
	 * @return The absolute value of this vector.
	 */
	auto Abs() const;

	/**
	 * Gets the minimal value in this vector.
	 * @return The minimal components.
	 */
	constexpr auto d3Min() const;

	/**
	 * Gets the maximal value in this vector.
	 * @return The maximal components.
	 */
	constexpr auto d3Max() const;

	/**
	 * Gets the minimal and maximal values in the vector.
	 * @return The minimal and maximal components.
	 */
	constexpr auto MinMax() const;

	/**
	 * Gets the lowest vector size between this vector and other.
	 * @tparam K The others type.
	 * @param other The other vector to get values from.
	 * @return The lowest vector.
	 */
	template<typename K>
	constexpr auto d3Min(const Vector3<K> &other);

	/**
	 * Gets the maximum vector size between this vector and other.
	 * @tparam K The others type.
	 * @param other The other vector to get values from.
	 * @return The maximum vector.
	 */
	template<typename K>
	constexpr auto d3Max(const Vector3<K> &other);

	/**
	 * Gets the distance between this vector and another vector.
	 * @tparam K The others type.
	 * @param other The other vector.
	 * @return The squared distance.
	 */
	template<typename K>
	constexpr auto DistanceSquared(const Vector3<K> &other) const;

	/**
	 * Gets the between this vector and another vector.
	 * @tparam K The others type.
	 * @param other The other vector.
	 * @return The distance.
	 */
	template<typename K>
	auto Distance(const Vector3<K> &other) const;

	/**
	 * Gets the vector distance between this vector and another vector.
	 * @tparam K The others type.
	 * @param other The other vector.
	 * @return The vector distance.
	 */
	template<typename K>
	constexpr auto DistanceVector(const Vector3<K> &other) const;

	/**
	 * Gradually changes this vector to a target.
	 * @param target The target vector.
	 * @param rate The rate to go from current to the target.
	 * @return The changed vector.
	 */
	template<typename K, typename J>
	constexpr auto SmoothDamp(const Vector3<K> &target, const Vector3<J> &rate) const;

	/**
	 * Converts from rectangular to spherical coordinates, this vector is in cartesian (x, y).
	 * @return The polar coordinates (radius, theta).
	 */
	auto CartesianToPolar() const;

	/**
	 * Converts from spherical to rectangular coordinates, this vector is in polar (radius, theta).
	 * @return The cartesian coordinates (x, y).
	 */
	auto PolarToCartesian() const;

	constexpr const T &operator[](uint32_t index) const;
	constexpr T &operator[](uint32_t index);

	template<typename K>
	constexpr bool operator==(const Vector3<K> &other) const;
	template<typename K>
	constexpr bool operator!=(const Vector3<K> &other) const;

	template<typename U = T>
	constexpr auto operator-() const -> std::enable_if_t<std::is_signed_v<U>, Vector3>;
	template<typename U = T>
	constexpr auto operator~() const -> std::enable_if_t<std::is_integral_v<U>, Vector3>;

	template<typename K>
	constexpr Vector3 &operator+=(const Vector3<K> &other);
	template<typename K>
	constexpr Vector3 &operator-=(const Vector3<K> &other);
	template<typename K>
	constexpr Vector3 &operator*=(const Vector3<K> &other);
	template<typename K>
	constexpr Vector3 &operator/=(const Vector3<K> &other);
	constexpr Vector3 &operator+=(const T &other);
	constexpr Vector3 &operator-=(const T &other);
	constexpr Vector3 &operator*=(const T &other);
	constexpr Vector3 &operator/=(const T &other);

	DRX3D_EXPORT static const Vector3 Zero;
	DRX3D_EXPORT static const Vector3 One;
	DRX3D_EXPORT static const Vector3 Infinity;
	DRX3D_EXPORT static const Vector3 Left;
	DRX3D_EXPORT static const Vector3 Right;
	DRX3D_EXPORT static const Vector3 Up;
	DRX3D_EXPORT static const Vector3 Down;
	DRX3D_EXPORT static const Vector3 Front;
	DRX3D_EXPORT static const Vector3 Back
	
	;

	T x = 0, y = 0, z = 0;
};

}

#include "Vector3.inl"
