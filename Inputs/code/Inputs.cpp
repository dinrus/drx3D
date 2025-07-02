#include <drx3D/Inputs/Inputs.h>

#include <iomanip>

namespace drx3d {
Inputs::Inputs() :
	nullScheme(std::make_unique<InputScheme>()),
	currentScheme(nullScheme.get()) {
}

void Inputs::Update() {
}

InputScheme *Inputs::GetScheme(const STxt &name) const {
	auto it = schemes.find(name);
	if (it == schemes.end()) {
		Log::Error("Could not find input scheme: ", std::quoted(name), '\n');
		return nullptr;
	}
	return it->second.get();
}

InputScheme *Inputs::AddScheme(const STxt &name, std::unique_ptr<InputScheme> &&scheme, bool setCurrent) {
	auto inputScheme = schemes.emplace(name, std::move(scheme)).first->second.get();
	if (!currentScheme || setCurrent)
		SetScheme(inputScheme);
	return inputScheme;
} 

void Inputs::RemoveScheme(const STxt &name) {
	auto it = schemes.find(name);
	if (currentScheme == it->second.get())
		SetScheme(nullptr);
	if (it != schemes.end())
		schemes.erase(it);
	// If we have no current scheme grab some random one from the map.
	if (!currentScheme && !schemes.empty())
		currentScheme = schemes.begin()->second.get();
}

void Inputs::SetScheme(InputScheme *scheme) {
	if (!scheme) scheme = nullScheme.get();
	// We want to preserve signals from the current scheme to the new one.
	scheme->MoveSignals(currentScheme);
	currentScheme = scheme;
}

void Inputs::SetScheme(const STxt &name) {
	auto scheme = GetScheme(name);
	if (!scheme) return;
	SetScheme(scheme);
}

InputAxis *Inputs::GetAxis(const STxt &name) const {
	if (currentScheme)
		return currentScheme->GetAxis(name);
	return nullptr;
}

InputButton *Inputs::GetButton(const STxt &name) const {
	if (currentScheme)
		return currentScheme->GetButton(name);
	return nullptr;
}
}
