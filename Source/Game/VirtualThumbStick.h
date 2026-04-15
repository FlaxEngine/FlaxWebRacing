#pragma once

#include "Vehicle.h"
#include "Engine/Core/Math/Vector2.h"
#include "Engine/Input/Enums.h"
#include "Engine/Scripting/Script.h"
#include "Engine/Scripting/ScriptingObjectReference.h"

/// <summary>
/// Draws a virtual gamepad thumbstick on a screen to control game inputs on platforms with only a touch-screen.
/// </summary>
API_CLASS() class GAME_API VirtualThumbStick : public Script
{
	API_AUTO_SERIALIZATION();
	DECLARE_SCRIPTING_TYPE(VirtualThumbStick);
private:
	Window* _window = nullptr;
	bool _inputDown = false, _inputOver = false;
	Float2 _inputLocation = Float2::Zero;
	Float2 _input = Float2::Zero;
	float _pressTime = 0;

public:
	// Target vehicle to control.
	API_FIELD() ScriptingObjectReference<Vehicle> Target;
	// Stick brush image.
	API_FIELD() AssetReference<Texture> BrushImage;
	// Stick color.
	API_FIELD() Color ColorDefault;
	// Stick color when user is pressing it.
	API_FIELD() Color ColorPressed;
	// Percentage of the view size that stick should cover.
	API_FIELD() float ScreenSize = 0.3f;
	// Percentage of the view size that stick should move away from viewport border.
	API_FIELD() float MarginSize = 0.05f;
	// Debug option to display stick in Editor. By default, it's used only on touch-screen devices.
	API_FIELD() bool ShowInEditor = false;

	// [Script]
	void OnEnable() override;
	void OnDisable() override;
	void OnUpdate() override;

private:
	Rectangle GetBounds() const;
	void OnDraw();
	void OnMouseDown(const Float2& mousePosition, MouseButton button);
	void OnMouseMove(const Float2& mousePosition);
	void OnMouseUp(const Float2& mousePosition, MouseButton button);
	void OnTouchDown(const Float2& pointerPosition, int32 pointerIndex);
	void OnTouchMove(const Float2& pointerPosition, int32 pointerIndex);
	void OnTouchUp(const Float2& pointerPosition, int32 pointerIndex);
};
