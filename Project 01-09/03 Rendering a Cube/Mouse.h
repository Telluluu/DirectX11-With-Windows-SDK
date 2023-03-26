//--------------------------------------------------------------------------------------
// File: Mouse.h
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// http://go.microsoft.com/fwlink/?LinkId=248929
// http://go.microsoft.com/fwlink/?LinkID=615561
//--------------------------------------------------------------------------------------

#pragma once

#if !defined(USING_XINPUT) && !defined(USING_GAMEINPUT) && !defined(USING_COREWINDOW)

#ifdef _GAMING_DESKTOP
#include <grdk.h>
#endif

#if (defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_GAMES)) || (defined(_GAMING_DESKTOP) && (_GRDK_EDITION >= 220600))
#define USING_GAMEINPUT
#elif (defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_APP)) || (defined(_XBOX_ONE) && defined(_TITLE))
#define USING_COREWINDOW
#endif

#endif // !USING_XINPUT && !USING_GAMEINPUT && !USING_WINDOWS_GAMING_INPUT

#if defined(USING_GAMEINPUT) && !defined(_GAMING_XBOX)
#pragma comment(lib,"gameinput.lib")
#endif

#include <memory>

#ifdef USING_COREWINDOW
namespace ABI { namespace Windows { namespace UI { namespace Core { struct ICoreWindow; } } } }
#endif

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#endif



namespace DirectX
{
    class Mouse
    {
    public:
        Mouse() noexcept(false);

        Mouse(Mouse&&) noexcept;
        Mouse& operator= (Mouse&&) noexcept;

        Mouse(Mouse const&) = delete;
        Mouse& operator=(Mouse const&) = delete;

        virtual ~Mouse();

        std::unique_ptr<DirectX::Mouse> m_pMouse;
        //鼠标模式有以下两种
        enum Mode
        {
            MODE_ABSOLUTE = 0, //绝对坐标模式，每次状态更新xy值为屏幕像素坐标，且鼠标可见
            MODE_RELATIVE,     //相对运动模式，每次状态更新xy值为每一帧之间的像素位移量，且鼠标不可见
        };

        struct State
        {
            bool    leftButton;  //左键被按下
            bool    middleButton;  //滚轮被按下
            bool    rightButton;  //右键被按下
            bool    xButton1;  //侧键被按下，下同
            bool    xButton2;
            int     x;  //绝对坐标x或相对偏移量
            int     y;  //绝对坐标y或相对偏移量
            int     scrollWheelValue;  //滚轮滚动累计值
            Mode    positionMode;  //鼠标模式
        };

        //鼠标追踪类
        class ButtonStateTracker
        {
        public:
            //鼠标状态枚举
            enum ButtonState
            {
                UP = 0,         // Button is up       按键未被按下
                HELD = 1,       // Button is held down  按键正在被按住
                RELEASED = 2,   // Button was just released  按键刚刚松开
                PRESSED = 3,    // Buton was just pressed    按键刚刚按下
            };

            ButtonState leftButton;  //左键状态
            ButtonState middleButton;  //滚轮状态
            ButtonState rightButton;  //右键状态
            ButtonState xButton1;  //鼠标侧键状态，下同
            ButtonState xButton2;

        #pragma prefast(suppress: 26495, "Reset() performs the initialization")
            ButtonStateTracker() noexcept { Reset(); }

            //在每一帧的时候硬提供Mouse的当前状态去更新它
            void __cdecl Update(const State& state) noexcept;

            void __cdecl Reset() noexcept;

            //在获取上一帧的鼠标事件，应当在Update之前使用，否则变为获取当前帧的状态
            State __cdecl GetLastState() const noexcept { return lastState; }

        private:
            State lastState;
        };

        // Retrieve the current state of the mouse
        State __cdecl GetState() const;  //获取当前帧下鼠标的运动状态

        // Resets the accumulated scroll wheel value
        void __cdecl ResetScrollWheelValue() noexcept;  //清空滚轮的滚动累计值

        // Sets mouse mode (defaults to absolute)
        void __cdecl SetMode(Mode mode);  //设置鼠标模式，默认为绝对坐标模式

        // Signals the end of frame (recommended, but optional)
        void __cdecl EndOfInputFrame() noexcept;

        // Feature detection
        bool __cdecl IsConnected() const;

        // Cursor visibility
        bool __cdecl IsVisible() const noexcept;  //检验鼠标是否可见
        void __cdecl SetVisible(bool visible);  //设置鼠标是否可见

    #ifdef USING_COREWINDOW
        void __cdecl SetWindow(ABI::Windows::UI::Core::ICoreWindow* window);
    #ifdef __cplusplus_winrt
        void __cdecl SetWindow(Windows::UI::Core::CoreWindow^ window)
        {
            // See https://msdn.microsoft.com/en-us/library/hh755802.aspx
            SetWindow(reinterpret_cast<ABI::Windows::UI::Core::ICoreWindow*>(window));
        }
    #endif
    #ifdef CPPWINRT_VERSION
        void __cdecl SetWindow(winrt::Windows::UI::Core::CoreWindow window)
        {
            // See https://docs.microsoft.com/en-us/windows/uwp/cpp-and-winrt-apis/interop-winrt-abi
            SetWindow(reinterpret_cast<ABI::Windows::UI::Core::ICoreWindow*>(winrt::get_abi(window)));
        }
    #endif

        static void __cdecl SetDpi(float dpi);
    #elif defined(WM_USER)
        void __cdecl SetWindow(HWND window);  //在初始化阶段给鼠标类设置要绑定的窗口句柄

        //处理鼠标信息
        static void __cdecl ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam);

    #ifdef _GAMING_XBOX
        static void __cdecl SetResolution(float scale);
    #endif
    #endif

        // Singleton
        static Mouse& __cdecl Get();//获取Mouse实例

    private:
        // Private implementation.
        class Impl;

        std::unique_ptr<Impl> pImpl;
    };
}

#ifdef __clang__
#pragma clang diagnostic pop
#endif
