#include "View.h"
#include "Engine/Core/Log.h"
#include "Engine/Engine/Engine.h"
#include "Engine/Level/Actor.h"
#include "Engine/Engine/Time.h"
#include "Engine/Engine/Units.h"
#include "Engine/Input/Input.h"

View::View(const SpawnParams& params)
    : Script(params)
{
    // Enable ticking OnFixedUpdate function
    _tickFixedUpdate = true;
}

void View::OnFixedUpdate()
{
	CHECK(Target && Camera);

	// Ease position towards target vehicle position
	auto position = GetActor()->GetPosition();
	auto target = Target->GetPosition();
	auto speedFactor = Math::Saturate(Math::Abs(UNITS_TO_METERS((Target->GetLinearVelocity() * Vector3(1, 0, 1)).Length())));
	target += Target->GetForward() * (METERS_TO_UNITS(2.0f) * speedFactor); // Move camera forward when moving fast
	position = Vector3::Lerp(position, target, Math::Saturate(Time::GetDeltaTime() * 4));
	GetActor()->SetPosition(position);

	// Zoom camera based on the speed of the vehicle
	auto targetZ = Math::Remap(speedFactor, 0.0f, 1.0f, METERS_TO_UNITS(CameraDistanceRange.X), METERS_TO_UNITS(CameraDistanceRange.Y));
	position = Camera->GetLocalPosition();
	position.Z = Math::Lerp(position.Z, targetZ, Math::Saturate(Time::GetDeltaTime() * 0.5f));
	Camera->SetLocalPosition(position);

	// Exit on escape
	if (Input::GetKeyDown(KeyboardKeys::Escape))
		Engine::RequestExit();
}
