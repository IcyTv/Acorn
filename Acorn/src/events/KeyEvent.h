#pragma once

#include "Event.h"

#include "input/KeyCodes.h"

namespace Acorn
{
	class KeyEvent : public Event
	{
	public:
		inline KeyCode GetKeyCode() const { return m_KeyCode; }
		inline int GetmodKeys() const { return m_ModKeys; }
		inline bool IsModKeyPressed(ModifierKeys modKey) { return m_ModKeys & static_cast<int32_t>(modKey); }

		EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
	protected:
		KeyEvent(KeyCode keycode, int modKeys) : m_KeyCode(keycode), m_ModKeys(modKeys) {}

		KeyCode m_KeyCode;
		int m_ModKeys;
	};

	class KeyPressedEvent : public KeyEvent
	{
	public:
		KeyPressedEvent(KeyCode keycode, int modKeys, int repeatCount) : KeyEvent(keycode, modKeys), m_RepeatCount(repeatCount) {}

		inline int GetRepeatCount() const { return m_RepeatCount;  }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyPressedEvent: " << m_KeyCode << " (x" << m_RepeatCount << ")";
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyPressed)
	private:
		int m_RepeatCount;
	};

	class KeyReleasedEvent : public KeyEvent
	{
	public:
		KeyReleasedEvent(KeyCode keycode, int modKeys) : KeyEvent(keycode, modKeys) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << m_KeyCode;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyReleased)
	};

	class KeyTypedEvent : public KeyEvent
	{
	public:
		KeyTypedEvent(KeyCode keycode, int modKeys) : KeyEvent(keycode, modKeys) {}


		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyTypedEvent: " << m_KeyCode;
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyTyped)
	private:
	};
}