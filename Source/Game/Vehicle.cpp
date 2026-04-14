#include "Vehicle.h"
#include "Engine/Core/Log.h"
#include "Engine/Engine/Units.h"
#include "Engine/Input/Input.h"
#include "Engine/Level/Actor.h"
#include "Engine/Physics/Physics.h"
#include "Engine/Physics/Actors/RigidBody.h"
#include "Engine/Engine/Time.h"
#include "Engine/Physics/Actors/WheeledVehicle.h"

Vehicle::Vehicle(const SpawnParams& params)
    : Script(params)
{
    // Enable ticking OnUpdate/OnFixedUpdate functions
    _tickUpdate = true;
    _tickFixedUpdate = true;
}

void Vehicle::OnEnable()
{
	auto car = Cast<WheeledVehicle>(GetActor());
	CHECK(car);
	for (Actor* child : car->Children)
	{
		const auto collider = Cast<Collider>(child);
		if (collider && !collider->GetName().Contains(TEXT("wheel"), StringSearchCase::IgnoreCase))
		{
			collider->CollisionEnter.Bind<Vehicle, &Vehicle::OnCollisionEnter>(this);
		}
	}
}

void Vehicle::OnUpdate()
{
    // Read inputs
    _input += Float3(Input::GetAxis(TEXT("Horizontal")), 0.0f, Input::GetAxis(TEXT("Vertical")));
}

void Vehicle::OnFixedUpdate()
{
	float delta = Time::GetDeltaTime();
	auto car = Cast<WheeledVehicle>(GetActor());
	CHECK(car && Model);

	// Apply car drive inputs
	Float3 velocity(_input);
	//velocity.Normalize();
	car->SetThrottle(velocity.Z);
	car->SetSteering(velocity.X);

	// Slightly tilt body based on acceleration and steering
	auto forwardSpeed = UNITS_TO_METERS(car->GetForwardSpeed());
	_bodyLean = Math::Lerp(_bodyLean, -_input.X / 40 * forwardSpeed, delta * 5);
	if (Body)
	{
		_bodyPitch = Math::Lerp(_bodyPitch, _input.Z / 30 * Math::Abs(forwardSpeed), delta * 5);
		_bodyPitch = Math::Clamp(_bodyPitch, -0.14f, 0.07f);
		Body->SetLocalOrientation(Quaternion::RotationYawPitchRoll(0, _bodyPitch, _bodyLean));
	}

	// Sync wheels rotations
	float wheelsSlip = 0.0f;
	for (int32 i = 0; i < Wheels.Count(); i++)
	{
		auto wheel = Wheels[i].Get();
		WheeledVehicle::WheelState state;
		car->GetWheelState(i, state);
		wheelsSlip = Math::Max(wheelsSlip, state.LateralSlip + state.LongitudinalSlip);
		wheel->SetLocalOrientation(Quaternion::Euler(state.RotationAngle, state.SteerAngle, 0));
	}

	// Update engine sounds
	if (EngineSound)
	{
		auto speedFactor = Math::Saturate(Math::Abs(forwardSpeed * 0.1f));
		auto throttleFactor = Math::Saturate(Math::Abs(_input.Z));
		auto targetVolume = Math::Remap(speedFactor * 0.5f + throttleFactor * 1.5f, 0.0f, 1.5f, 0.2f, 0.5f);
		auto targetPitch = Math::Remap(speedFactor, 0.0f, 1.0f, 0.5f, 3.0f);
		if (throttleFactor > 0.1f)
			targetPitch += 0.2f;
		EngineSound->SetVolume(Math::Lerp(EngineSound->GetVolume(), targetVolume * EngineVolume, delta * 5.0f));
		EngineSound->SetPitch(Math::Lerp(EngineSound->GetPitch(), targetPitch, delta * 2.0));
	}

	// Update dirt trails and play skid sounds
	auto driftIntensity = wheelsSlip * 3.0f + Math::Abs(_bodyLean) * 2.0f;
	auto isDrifting = driftIntensity > 0.25;
	for (auto& particles : TrailParticles)
		particles->CanSpawn = isDrifting;
	if (ScreechSound)
	{
		auto targetVolume = 0.0f;
		if (isDrifting)
			targetVolume = Math::Remap(Math::Clamp(driftIntensity, 0.25f, 2.0f), 0.25f, 2.0f, 0.1f, 0.6f);
		auto targetPitch = Math::Clamp(Math::Abs(forwardSpeed * 0.2f), 0.8f, 1.2f);
		ScreechSound->SetPitch(Math::Lerp(ScreechSound->GetPitch(), targetPitch, delta * 2.0f));
		ScreechSound->SetVolume(Math::Lerp(ScreechSound->GetVolume(), targetVolume * ScreechVolume, delta * 10.0f));
	}

	// Clear processed input
	_input = Vector3::Zero;
}

void Vehicle::OnCollisionEnter(const Collision& collision)
{
	if (ImpactSound && ImpactSound->GetState() == AudioSource::States::Stopped)
	{
		// Play impact sound when car body collider hits sth
		auto impactFactor = Math::Saturate((float)collision.GetRelativeVelocity().Length() * 0.1f);
		auto targetVolume = Math::Remap(impactFactor, 0.0f, 1.0f, 0.2f, 0.8f) * ImpactVolume;
		ImpactSound->SetVolume(targetVolume);
		ImpactSound->Play();
	}
}
