#pragma once

#include "Engine/Scripting/Script.h"
#include "Engine/Scripting/ScriptingObjectReference.h"
#include "Engine/Level/Actors/Camera.h"
#include "Engine/Physics/Actors/RigidBody.h"

/// <summary>
/// Camera controller script.
/// </summary>
API_CLASS() class GAME_API View : public Script
{
	API_AUTO_SERIALIZATION();
	DECLARE_SCRIPTING_TYPE(View);

	// Target actor to follow.
	API_FIELD() ScriptingObjectReference<RigidBody> Target;
	// Camera actor to control.
	API_FIELD() ScriptingObjectReference<Camera> Camera;
	// Range (min, max) for the distance between camera and target (in meters).
	API_FIELD() Float2 CameraDistanceRange = Float2(10, 20);

	// [Script]
	void OnFixedUpdate() override;
};
