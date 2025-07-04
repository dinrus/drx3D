#include <drx3D/Physics/Force.h>

namespace drx3d {
Force::Force(const Vector3f &force, const Vector3f &position) :
	force(force),
	neverExpires(true),
	position(position) {
}

Force::Force(const Vector3f &force, const Time &time, const Vector3f &position) :
	force(force),
	neverExpires(false),
	timeLeft(time),
	position(position) {
}

void Force::Update(const Time &delta) {
	timeLeft -= delta;
}
}
