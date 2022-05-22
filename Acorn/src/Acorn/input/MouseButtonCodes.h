#pragma once

//From glfw3.h

namespace Acorn
{
	using MouseButtonCode = uint16_t;

	namespace MouseButton
	{
		enum : MouseButtonCode
		{
			Button0 = 0,
			Button1 = 1,
			Button2 = 2,
			Button3 = 3,
			Button4 = 4,
			Button5 = 5,
			Button6 = 6,
			Button7 = 7,

			ButtonLast = Button7,
			ButtonLeft = Button0,
			ButtonRight = Button1,
			ButtonMiddle = Button2
		};
	}
}

#define AC_MOUSE_BUTTON_1 ::Acorn::MouseButton::Button0
#define AC_MOUSE_BUTTON_2 ::Acorn::MouseButton::Button1
#define AC_MOUSE_BUTTON_3 ::Acorn::MouseButton::Button2
#define AC_MOUSE_BUTTON_4 ::Acorn::MouseButton::Button3
#define AC_MOUSE_BUTTON_5 ::Acorn::MouseButton::Button4
#define AC_MOUSE_BUTTON_6 ::Acorn::MouseButton::Button5
#define AC_MOUSE_BUTTON_7 ::Acorn::MouseButton::Button6
#define AC_MOUSE_BUTTON_8 ::Acorn::MouseButton::Button7
#define AC_MOUSE_BUTTON_LAST ::Acorn::MouseButton::ButtonLast
#define AC_MOUSE_BUTTON_LEFT ::Acorn::MouseButton::ButtonLeft
#define AC_MOUSE_BUTTON_RIGHT ::Acorn::MouseButton::ButtonRight
#define AC_MOUSE_BUTTON_MIDDLE ::Acorn::MouseButton::ButtonMiddle