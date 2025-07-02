#include <drx3D/Lights/Fog.h>

namespace drx3d {
Fog::Fog(const Color &color, float density, float gradient, float lowerLimit, float upperLimit) :
	color(color),
	density(density),
	gradient(gradient),
	lowerLimit(lowerLimit),
	upperLimit(upperLimit) {
}

void Fog::Start() {
}

void Fog::Update() {
}

const Node &operator>>(const Node &node, Fog &fog) {
	node["color"].Get(fog.color);
	node["density"].Get(fog.density);
	node["gradient"].Get(fog.gradient);
	node["lowerLimit"].Get(fog.lowerLimit);
	node["upperLimit"].Get(fog.upperLimit);
	return node;
}

Node &operator<<(Node &node, const Fog &fog) {
	node["color"].Set(fog.color);
	node["density"].Set(fog.density);
	node["gradient"].Set(fog.gradient);
	node["lowerLimit"].Set(fog.lowerLimit);
	node["upperLimit"].Set(fog.upperLimit);
	return node;
}
}
