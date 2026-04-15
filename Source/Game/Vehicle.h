#pragma once

#include "Engine/Audio/AudioSource.h"
#include "Engine/Core/Math/Vector2.h"
#include "Engine/Core/Math/Vector3.h"
#include "Engine/Scripting/Script.h"
#include "Engine/Scripting/ScriptingObjectReference.h"
#include "Engine/Audio/AudioSource.h"
#include "Engine/Particles/ParticleEffect.h"

/// <summary>
/// Vehicle driving script.
/// </summary>
API_CLASS() class GAME_API Vehicle : public Script
{
	API_AUTO_SERIALIZATION();
	DECLARE_SCRIPTING_TYPE(Vehicle);

private:
	Float3 _input = Float3::Zero;
	float _bodyLean = 0.0f;
	float _bodyPitch = 0.0f;

public:
	// Vehicle model actor.
	API_FIELD() ScriptingObjectReference<Actor> Model;
	// Vehicle body model.
	API_FIELD() ScriptingObjectReference<Actor> Body;
	// Wheel models (order must match wheels data set on WheeledVehicle).
	API_FIELD() Array<ScriptingObjectReference<Actor>> Wheels;
	// Engine sound audio source.
	API_FIELD() ScriptingObjectReference<AudioSource> EngineSound;
	// Impact sound audio source.
	API_FIELD() ScriptingObjectReference<AudioSource> ImpactSound;
	// Screech sound audio source.
	API_FIELD() ScriptingObjectReference<AudioSource> ScreechSound;
	// All trails particle effects.
	API_FIELD() Array<ScriptingObjectReference<ParticleEffect>> TrailParticles;

public:
	// Engine sound volume scale.
	API_FIELD() float EngineVolume = 1.0f;
	// Impact sound volume scale.
	API_FIELD() float ImpactVolume = 1.0f;
	// Screech sound volume scale.
	API_FIELD() float ScreechVolume = 1.0f;

	// Adds external input control (X=horizonal, Y=vertical).
	void AddInput(const Float2& input);

public:
	// [Script]
	void OnEnable() override;
	void OnUpdate() override;
	void OnFixedUpdate() override;

private:
	void OnCollisionEnter(const struct Collision& collision);
};
