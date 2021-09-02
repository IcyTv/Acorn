#pragma once

#include "core/Core.h"

#include "acpch.h"

// #define BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)
#define BIND_EVENT_FN(fn) [this](auto &&...args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

namespace Acorn
{
	enum class EventType
	{
		None = 0,
		WindowClose,
		WindowResize,
		WindowFocus,
		WindowLostFocus,
		WindowMoved,
		AppTick,
		AppUpdate,
		AppRender,
		ViewportChanged,
		KeyPressed,
		KeyReleased,
		KeyTyped,
		MouseButtonPressed,
		MouseButtonReleased,
		MouseMoved,
		MouseScrolled,
	};

	enum EventCategory
	{
		None = 0,
		EventCategoryApplication = BIT(0),
		EventCategoryInput = BIT(1),
		EventCategoryKeyboard = BIT(2),
		EventCategoryMouse = BIT(3),
		EventCategoryMouseButton = BIT(4),
	};

#define EVENT_CLASS_TYPE(type)                                                  \
	static EventType GetStaticType() { return EventType::##type; }              \
	virtual EventType GetEventType() const override { return GetStaticType(); } \
	virtual const char *GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) \
	virtual int GetCategoryFlags() const override { return category; }

	class AC_API Event
	{
		friend class EventDispatcher;

	public:
		bool Handled = false;

		virtual ~Event() = default;

		virtual EventType GetEventType() const = 0;
		virtual const char *GetName() const = 0;
		virtual int GetCategoryFlags() const = 0;
		virtual std::string ToString() const
		{
			return GetName();
		}

		inline bool IsInCategory(EventCategory category)
		{
			return GetCategoryFlags() & category;
		}

	protected:
	};

	class EventDispatcher
	{
		template <typename T>
		using EventFn = std::function<bool(T &)>;

	public:
		EventDispatcher(Event &event) : m_Event(event) {}

		template <typename T>
		bool Dispatch(EventFn<T> func)
		{
			if (m_Event.GetEventType() == T::GetStaticType())
			{
				m_Event.Handled = func(*(T *)&m_Event);
				return true;
			}
			return false;
		}

	private:
		Event &m_Event;
	};

	inline std::ostream &operator<<(std::ostream &os, const Event &e)
	{
		return os << e.ToString();
	}
}