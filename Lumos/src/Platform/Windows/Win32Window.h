#pragma once

#include "LM.h"
#include "App/Window.h"
#include "Graphics/API/Context.h"

namespace Lumos
{

	class LUMOS_EXPORT Win32Window : public Window
	{
	public:
		Win32Window(const WindowProperties& properties, const String& title, RenderAPI api = RenderAPI::OPENGL);
		~Win32Window();

		void ToggleVSync() override;
		void SetWindowTitle(const String& title) override;
		void SetBorderlessWindow(bool borderless) override;
		void OnUpdate() override;

		void SetVSync(bool set) override {};
		void HideMouse(bool hide) override {};
		void SetMousePosition(const maths::Vector2& pos) override {};

		bool Init(const WindowProperties& properties, const String& title);

		inline uint GetWidth()  const override { return m_Data.Width; }
		inline uint GetHeight() const override { return m_Data.Height; }
		inline float GetScreenRatio() const override { return (float)m_Data.Width / (float)m_Data.Height; }
		inline bool GetExit() const override { return m_Data.Exit; }
		inline void SetExit(bool exit) override { m_Data.Exit = exit; }
		inline void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }

		inline void* GetHandle() override { return hWnd; }

	protected:

		HINSTANCE hInstance;
		HDC hDc;
		HWND hWnd;
		RAWINPUTDEVICE rid;

		struct WindowData
		{
			std::string Title;
			uint Width, Height;
			bool VSync;
			bool Exit;
			RenderAPI m_RenderAPI;

			EventCallbackFn EventCallback;
		};

		WindowData m_Data;
	};

}