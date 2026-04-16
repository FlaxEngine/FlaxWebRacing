#include "VirtualThumbStick.h"
#include "Engine/Engine/Engine.h"
#include "Engine/Engine/Time.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Platform/Window.h"
#include "Engine/Render2D/Render2D.h"
#if USE_EDITOR
#include "Editor/Editor.h"
#include "Editor/Managed/ManagedEditor.h"
#endif

VirtualThumbStick::VirtualThumbStick(const SpawnParams& params)
    : Script(params)
	, ColorDefault(Color::White)
	, ColorPressed(Color::White)
{
	_tickUpdate = true;
}

void VirtualThumbStick::OnEnable()
{
	bool show = (USE_EDITOR && ShowInEditor);
	auto platform = PLATFORM_TYPE;
#if PLATFORM_WEB
	// Use platform detected from User-Agent on web
	platform = WebPlatform::GetUserAgentPlatform();
#endif
	show |= platform == PlatformType::Android;
	show |= platform == PlatformType::iOS;
	if (!show)
		return;

	// Register for GUI drawing event
	_window = Engine::MainWindow;
#if USE_EDITOR
	if (Editor::Managed)
	{
		// Editor can use different window for the game
		_window = Editor::Managed->GetGameWindow(true);
	}
#endif
	if (!_window)
		return;
	_window->Draw.Bind<VirtualThumbStick, &VirtualThumbStick::OnDraw>(this);
	_window->MouseDown.Bind<VirtualThumbStick, &VirtualThumbStick::OnMouseDown>(this);
	_window->MouseMove.Bind<VirtualThumbStick, &VirtualThumbStick::OnMouseMove>(this);
	_window->MouseUp.Bind<VirtualThumbStick, &VirtualThumbStick::OnMouseUp>(this);
	_window->TouchDown.Bind<VirtualThumbStick, &VirtualThumbStick::OnTouchDown>(this);
	_window->TouchMove.Bind<VirtualThumbStick, &VirtualThumbStick::OnTouchMove>(this);
	_window->TouchUp.Bind<VirtualThumbStick, &VirtualThumbStick::OnTouchUp>(this);
	_inputDown = _inputOver = false;
}

void VirtualThumbStick::OnDisable()
{
	if (!_window)
		return;

	// Unregister events
	_window->MouseDown.Unbind<VirtualThumbStick, &VirtualThumbStick::OnMouseDown>(this);
	_window->MouseMove.Unbind<VirtualThumbStick, &VirtualThumbStick::OnMouseMove>(this);
	_window->MouseUp.Unbind<VirtualThumbStick, &VirtualThumbStick::OnMouseUp>(this);
	_window->TouchDown.Unbind<VirtualThumbStick, &VirtualThumbStick::OnTouchDown>(this);
	_window->TouchMove.Unbind<VirtualThumbStick, &VirtualThumbStick::OnTouchMove>(this);
	_window->TouchUp.Unbind<VirtualThumbStick, &VirtualThumbStick::OnTouchUp>(this);
	_window->Draw.Unbind<VirtualThumbStick, &VirtualThumbStick::OnDraw>(this);
	_window = nullptr;
}

void VirtualThumbStick::OnUpdate()
{
	if (_inputDown && _inputOver)
	{
		_pressTime += Time::GetDeltaTime();
		if (Target)
		{
			// Scale input intensity over time to provide smoother control
			float inputScale = Math::Saturate(_pressTime * 2.0f);
			Target->AddInput(_input * inputScale, true);
		}
	}
}

Rectangle VirtualThumbStick::GetBounds() const
{
	// Get viewport bounds
	Rectangle bounds(Float2::Zero, _window->GetClientSize());
#if USE_EDITOR
	// Editor can use different placement of the game viewport
	bounds.Location = _window->ScreenToClient(Editor::Managed->GameViewportToScreen(bounds.Location));
	bounds.Size = Editor::Managed->GetGameWindowSize();
#endif

	// Calculate thumb placement on a screen
	float margin = bounds.Size.X * MarginSize;
	bounds.Location.X += margin;
	bounds.Location.Y += bounds.Size.Y * (1 - ScreenSize) - margin;
	bounds.Size = (bounds.Size * ScreenSize).MinValue();

	return bounds;
}

void VirtualThumbStick::OnDraw()
{
	Rectangle bounds = GetBounds();
	auto pressedScale = 0.5f;
	auto normalSize = 0.8f;
	auto isPressed = _inputDown && _inputOver;
	auto brushBounds = isPressed ? Rectangle(Float2::Clamp(_inputLocation, bounds.Size * (pressedScale / 2), bounds.Size * (1 - pressedScale / 2)) - bounds.Size * (pressedScale / 2), bounds.Size * pressedScale) : Rectangle(bounds.Size * (0.5f - normalSize * 0.5f), bounds.Size * normalSize);
	brushBounds.Location += bounds.Location;
	Color color = isPressed ? ColorPressed : ColorDefault;
	Render2D::DrawTexture(BrushImage, brushBounds, color);
}

void VirtualThumbStick::OnMouseDown(const Float2& mousePosition, MouseButton button)
{
	if (button == MouseButton::Left && !_inputDown)
	{
		_inputDown = true;
		OnMouseMove(mousePosition);
	}
}

void VirtualThumbStick::OnMouseMove(const Float2& mousePosition)
{
	auto bounds = GetBounds();
	auto over = bounds.Contains(mousePosition);
	if (_inputOver != over && !(_inputOver && !over && _inputDown))
	{
		// Mouse/touch moved out or in the bounds
		_inputOver = over;
		_input = Float2::Zero;
		_inputLocation = Float2::Zero;
		_pressTime = 0;
		if (!over)
			return;
	}

	// Control the input of the joystick
	auto location = mousePosition - bounds.Location;
	auto sizeHalf = bounds.Size * 0.5f;
	auto dir = location - sizeHalf;
	auto intensity = dir.Length() / sizeHalf.Length();
	auto dirNormalized = Float2::Normalize(dir);
	_inputLocation = sizeHalf + dirNormalized * sizeHalf * intensity;
	_input = Float2(location.X - sizeHalf.X, sizeHalf.Y - location.Y) / sizeHalf;
}

void VirtualThumbStick::OnMouseUp(const Float2& mousePosition, MouseButton button)
{
	if (button == MouseButton::Left && _inputDown)
	{
		_inputDown = false;
		OnMouseMove(mousePosition);
	}
}

void VirtualThumbStick::OnTouchDown(const Float2& pointerPosition, int32 pointerIndex)
{
	OnMouseDown(pointerPosition, MouseButton::Left);
}

void VirtualThumbStick::OnTouchMove(const Float2& pointerPosition, int32 pointerIndex)
{
	OnMouseMove(pointerPosition);
}

void VirtualThumbStick::OnTouchUp(const Float2& pointerPosition, int32 pointerIndex)
{
	OnMouseUp(pointerPosition, MouseButton::Left);
}
