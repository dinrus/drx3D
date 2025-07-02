#pragma once

#include <drx3D/Maths/Time.h>
#include <drx3D/Animation/Keyframe.h>

namespace drx3d {
/**
 * @brief Класс, представляющий собой анимацию, которая может выполняться
 * анимированной сущностью. Содержит длину анимации в секундах, и список
 * {@link Keyframe}ов (ключевых кадров).
 */
class DRX3D_EXPORT Animation {
public:
	/**
	 * Создаёт новую анимацию.
	 * @param length Длина этой анимации.
	 * @param keyframes Все ключевые кадры анимации,
	 *                  упорядоченные по времени их появления в ней.
	 */
	Animation(const Time &length, std::vector<Keyframe> keyframes);

	/**
	 * Получает длину анимации.
	 * @return Длина анимации.
	 */
	const Time& GetLength() const { return length; }

	/**
	 * Получает массив ключевых кадров анимации. Массив упорядочен
	 * по порядку их использования в анимации.
	 * @return Массив ключевых кадров анимации.
	 */
	const std::vector<Keyframe> &GetKeyframes() const { return keyframes; }

	friend const Node &operator>>(const Node &node, Animation &animation);
	friend Node &operator<<(Node &node, const Animation &animation);

private:
	Time length;
	std::vector<Keyframe> keyframes;
};
}
