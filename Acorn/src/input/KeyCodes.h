#pragma once

namespace Acorn
{
	typedef enum class KeyCode : uint16_t
	{
		Space = 32,
		Apostrophe = 39,  /* ' */
		Comma = 44,  /* , */
		Minus = 45,  /* - */
		Period = 46,  /* . */
		Slash = 47,  /* / */
		D0 = 48,
		D1 = 49,
		D2 = 50,
		D3 = 51,
		D4 = 52,
		D5 = 53,
		D6 = 54,
		D7 = 55,
		D8 = 56,
		D9 = 57,
		Semicolon = 59,  /* ; */
		Equal = 61,  /* = */
		A = 65,
		B = 66,
		C = 67,
		D = 68,
		E = 69,
		F = 70,
		G = 71,
		H = 72,
		I = 73,
		J = 74,
		K = 75,
		L = 76,
		M = 77,
		N = 78,
		O = 79,
		P = 80,
		Q = 81,
		R = 82,
		S = 83,
		T = 84,
		U = 85,
		V = 86,
		W = 87,
		X = 88,
		Y = 89,
		Z = 90,
		LeftBracket = 91,  /* [ */
		Backslash = 92,  /* \ */
		RightBracket = 93,  /* ] */
		GraveAccent = 96,  /* ` */
		World1 = 161, /* non-US #1 */
		World2 = 162, /* non-US #2 */
		Escape = 256,
		Enter = 257,
		Tab = 258,
		Backspace = 259,
		Insert = 260,
		Del = 261,
		Right = 262,
		Left = 263,
		Down = 264,
		Up = 265,
		PaacUp = 266,
		PaacDown = 267,
		Home = 268,
		End = 269,
		CapsLock = 280,
		ScrollLock = 281,
		NumLock = 282,
		PrintScreen = 283,
		Pause = 284,
		F1 = 290,
		F2 = 291,
		F3 = 292,
		F4 = 293,
		F5 = 294,
		F6 = 295,
		F7 = 296,
		F8 = 297,
		F9 = 298,
		F10 = 299,
		F11 = 300,
		F12 = 301,
		F13 = 302,
		F14 = 303,
		F15 = 304,
		F16 = 305,
		F17 = 306,
		F18 = 307,
		F19 = 308,
		F20 = 309,
		F21 = 310,
		F22 = 311,
		F23 = 312,
		F24 = 313,
		F25 = 314,
		Keypad0 = 320,
		Keypad1 = 321,
		Keypad2 = 322,
		Keypad3 = 323,
		Keypad4 = 324,
		Keypad5 = 325,
		Keypad6 = 326,
		Keypad7 = 327,
		Keypad8 = 328,
		Keypad9 = 329,
		KeypadDecimal = 330,
		KeypadDivide = 331,
		KeypadMultiply = 332,
		KeypadSubtract = 333,
		KeypadAdd = 334,
		KeypadEnter = 335,
		KeypadEqual = 336,
		LeftShift = 340,
		LeftControl = 341,
		LeftAlt = 342,
		LeftSuper = 343,
		RightShift = 344,
		RightControl = 345,
		RightAlt = 346,
		RightSuper = 347,
		Menu = 348,

	} Key;

	inline std::ostream& operator<<(std::ostream& os, KeyCode keyCode)
	{
		os << static_cast<int32_t>(keyCode);
		return os;
	}

	typedef enum class ModifierKeys : uint8_t
	{
		Shift = 0x0001,
		Control = 0x0002,
		Alt = 0x0004,
		Super = 0x0008,
		CapsLock = 0x0010,
		NumLock = 0x0020,
	} ModifierKey;



	inline std::ostream& operator<<(std::ostream& os, ModifierKeys keyCode)
	{
		os << static_cast<int32_t>(keyCode);
		return os;
	}
}

//From glfw.h
#define AC_KEY_SPACE              ::Acorn::Key::Space
#define AC_KEY_APOSTROPHE         ::Acorn::Key::Apostrophe
#define AC_KEY_COMMA              ::Acorn::Key::Comma
#define AC_KEY_MINUS              ::Acorn::Key::Minus
#define AC_KEY_PERIOD             ::Acorn::Key::Period
#define AC_KEY_SLASH              ::Acorn::Key::Slash
#define AC_KEY_0                  ::Acorn::Key::D0
#define AC_KEY_1                  ::Acorn::Key::D1
#define AC_KEY_2                  ::Acorn::Key::D2
#define AC_KEY_3                  ::Acorn::Key::D3
#define AC_KEY_4                  ::Acorn::Key::D4
#define AC_KEY_5                  ::Acorn::Key::D5
#define AC_KEY_6                  ::Acorn::Key::D6
#define AC_KEY_7                  ::Acorn::Key::D7
#define AC_KEY_8                  ::Acorn::Key::D8
#define AC_KEY_9                  ::Acorn::Key::D9
#define AC_KEY_SEMICOLON          ::Acorn::Key::Semicolon
#define AC_KEY_EQUAL              ::Acorn::Key::Equal
#define AC_KEY_A                  ::Acorn::Key::A
#define AC_KEY_B                  ::Acorn::Key::B
#define AC_KEY_C                  ::Acorn::Key::C
#define AC_KEY_D                  ::Acorn::Key::D
#define AC_KEY_E                  ::Acorn::Key::E
#define AC_KEY_F                  ::Acorn::Key::F
#define AC_KEY_G                  ::Acorn::Key::G
#define AC_KEY_H                  ::Acorn::Key::H
#define AC_KEY_I                  ::Acorn::Key::I
#define AC_KEY_J                  ::Acorn::Key::J
#define AC_KEY_K                  ::Acorn::Key::K
#define AC_KEY_L                  ::Acorn::Key::L
#define AC_KEY_M                  ::Acorn::Key::M
#define AC_KEY_N                  ::Acorn::Key::N
#define AC_KEY_O                  ::Acorn::Key::O
#define AC_KEY_P                  ::Acorn::Key::P
#define AC_KEY_Q                  ::Acorn::Key::Q
#define AC_KEY_R                  ::Acorn::Key::R
#define AC_KEY_S                  ::Acorn::Key::S
#define AC_KEY_T                  ::Acorn::Key::T
#define AC_KEY_U                  ::Acorn::Key::U
#define AC_KEY_V                  ::Acorn::Key::V
#define AC_KEY_W                  ::Acorn::Key::W
#define AC_KEY_X                  ::Acorn::Key::X
#define AC_KEY_Y                  ::Acorn::Key::Y
#define AC_KEY_Z                  ::Acorn::Key::Z
#define AC_KEY_LEFT_BRACKET       ::Acorn::Key::LeftBracket
#define AC_KEY_BACKSLASH          ::Acorn::Key::Backslash
#define AC_KEY_RIGHT_BRACKET      ::Acorn::Key::RightBracket
#define AC_KEY_GRAVE_ACCENT       ::Acorn::Key::GraveAccent
#define AC_KEY_WORLD_1            ::Acorn::Key::World1
#define AC_KEY_WORLD_2            ::Acorn::Key::World2
#define AC_KEY_ESCAPE             ::Acorn::Key::Escape
#define AC_KEY_ENTER              ::Acorn::Key::Enter
#define AC_KEY_TAB                ::Acorn::Key::Tab
#define AC_KEY_BACKSPACE          ::Acorn::Key::Backspace
#define AC_KEY_INSERT             ::Acorn::Key::Insert
#define AC_KEY_DELETE             ::Acorn::Key::Del
#define AC_KEY_RIGHT              ::Acorn::Key::Right
#define AC_KEY_LEFT               ::Acorn::Key::Left
#define AC_KEY_DOWN               ::Acorn::Key::Down
#define AC_KEY_UP                 ::Acorn::Key::Up
#define AC_KEY_PAAC_UP            ::Acorn::Key::PaacUp
#define AC_KEY_PAAC_DOWN          ::Acorn::Key::PaacDown
#define AC_KEY_HOME               ::Acorn::Key::Home
#define AC_KEY_END                ::Acorn::Key::End
#define AC_KEY_CAPS_LOCK          ::Acorn::Key::CapsLock
#define AC_KEY_SCROLL_LOCK        ::Acorn::Key::ScrollLock
#define AC_KEY_NUM_LOCK           ::Acorn::Key::NumLock
#define AC_KEY_PRINT_SCREEN       ::Acorn::Key::PrintScreen
#define AC_KEY_PAUSE              ::Acorn::Key::Pause
#define AC_KEY_F1                 ::Acorn::Key::F1
#define AC_KEY_F2                 ::Acorn::Key::F2
#define AC_KEY_F3                 ::Acorn::Key::F3
#define AC_KEY_F4                 ::Acorn::Key::F4
#define AC_KEY_F5                 ::Acorn::Key::F5
#define AC_KEY_F6                 ::Acorn::Key::F6
#define AC_KEY_F7                 ::Acorn::Key::F7
#define AC_KEY_F8                 ::Acorn::Key::F8
#define AC_KEY_F9                 ::Acorn::Key::F9
#define AC_KEY_F10                ::Acorn::Key::F10
#define AC_KEY_F11                ::Acorn::Key::F11
#define AC_KEY_F12                ::Acorn::Key::F12
#define AC_KEY_F13                ::Acorn::Key::F13
#define AC_KEY_F14                ::Acorn::Key::F14
#define AC_KEY_F15                ::Acorn::Key::F15
#define AC_KEY_F16                ::Acorn::Key::F16
#define AC_KEY_F17                ::Acorn::Key::F17
#define AC_KEY_F18                ::Acorn::Key::F18
#define AC_KEY_F19                ::Acorn::Key::F19
#define AC_KEY_F20                ::Acorn::Key::F20
#define AC_KEY_F21                ::Acorn::Key::F21
#define AC_KEY_F22                ::Acorn::Key::F22
#define AC_KEY_F23                ::Acorn::Key::F23
#define AC_KEY_F24                ::Acorn::Key::F24
#define AC_KEY_F25                ::Acorn::Key::F25
#define AC_KEY_KP_0               ::Acorn::Key::Keypad0
#define AC_KEY_KP_1               ::Acorn::Key::Keypad1
#define AC_KEY_KP_2               ::Acorn::Key::Keypad2
#define AC_KEY_KP_3               ::Acorn::Key::Keypad3
#define AC_KEY_KP_4               ::Acorn::Key::Keypad4
#define AC_KEY_KP_5               ::Acorn::Key::Keypad5
#define AC_KEY_KP_6               ::Acorn::Key::Keypad6
#define AC_KEY_KP_7               ::Acorn::Key::Keypad7
#define AC_KEY_KP_8               ::Acorn::Key::Keypad8
#define AC_KEY_KP_9               ::Acorn::Key::Keypad9
#define AC_KEY_KP_DECIMAL         ::Acorn::Key::KeypadDecimal
#define AC_KEY_KP_DIVIDE          ::Acorn::Key::KeypadDivide
#define AC_KEY_KP_MULTIPLY        ::Acorn::Key::KeypadMultiply
#define AC_KEY_KP_SUBTRACT        ::Acorn::Key::KeypadSubtract
#define AC_KEY_KP_ADD             ::Acorn::Key::KeypadAdd
#define AC_KEY_KP_ENTER           ::Acorn::Key::KeypadEnter
#define AC_KEY_KP_EQUAL           ::Acorn::Key::KeypadEqual
#define AC_KEY_LEFT_SHIFT         ::Acorn::Key::LeftShift
#define AC_KEY_LEFT_CONTROL       ::Acorn::Key::LeftControl
#define AC_KEY_LEFT_ALT           ::Acorn::Key::LeftAlt
#define AC_KEY_LEFT_SUPER         ::Acorn::Key::LeftSuper
#define AC_KEY_RIGHT_SHIFT        ::Acorn::Key::RightShift
#define AC_KEY_RIGHT_CONTROL      ::Acorn::Key::RightControl
#define AC_KEY_RIGHT_ALT          ::Acorn::Key::RightAlt
#define AC_KEY_RIGHT_SUPER        ::Acorn::Key::RightSuper
#define AC_KEY_MENU               ::Acorn::Key::Menu


//Modifiers
#define AC_MOD_SHIFT           Acorn::ModifierKey::Shift
#define AC_MOD_CONTROL         Acorn::ModifierKey::Control
#define AC_MOD_ALT             Acorn::ModifierKey::Alt
#define AC_MOD_SUPER           Acorn::ModifierKey::Super
#define AC_MOD_CAPS_LOCK       Acorn::ModifierKey::CapsLock
#define AC_MOD_NUM_LOCK        Acorn::ModifierKey::NumLock