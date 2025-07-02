#include <drx3D/Lights/Light.h>

#include <drx3D/Scenes/Entity.h>

namespace drx3d {
Light::Light(const Color &color, float radius) :
	color(color),
	radius(radius) {
}

void Light::Start() {
}

void Light::Update() {
}

const Node &operator>>(const Node &node, Light &light) {
	node["color"].Get(light.color);
	node["radius"].Get(light.radius);
	return node;
}

Node &operator<<(Node &node, const Light &light) {
	node["color"].Set(light.color);
	node["radius"].Set(light.radius);
	return node;
}
}
