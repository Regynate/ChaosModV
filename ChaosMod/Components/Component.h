#pragma once

#include <memory>
#include <set>

using DWORD = unsigned long;

class Component;

inline std::set<Component *> g_Components;

template <class T>
struct ComponentHolder
{
	static_assert(std::is_base_of_v<Component, T>, "T must be derived from Component");
	class Ptr
	{
		struct Deleter
		{
			void operator()(T *ptr)
			{
				delete ptr;
			}
		};
		std::unique_ptr<T, Deleter> m_Ptr;

	  public:
		T *operator()() const
		{
			return m_Ptr.get();
		}

		void operator=(T *ptr)
		{
			m_Ptr.reset();
			m_Ptr = std::unique_ptr<T, Deleter>(ptr);
		}

		void Reset()
		{
			m_Ptr.reset();
		}
	};

	static inline Ptr Instance;
};

template <class T>
inline T *GetComponent()
requires std::is_base_of_v<Component, T>
{
	return ComponentHolder<T>::Instance();
}

template <class T>
inline bool ComponentExists()
requires std::is_base_of_v<Component, T>
{
	return ComponentHolder<T>::Instance();
}

template <class T, class X = T>
inline void InitComponent(auto &&...args)
requires std::is_base_of_v<Component, T> && std::is_base_of_v<T, X>
{
	ComponentHolder<T>::Instance = new X(args...);
}

template <class T>
inline void UninitComponent()
requires std::is_base_of_v<Component, T>
{
	ComponentHolder<T>::Instance.Reset();
}

class Component
{
  public:
	Component()
	{
		g_Components.insert(this);
	}

	~Component()
	{
		g_Components.erase(this);
	}

	Component(const Component &)            = delete;

	Component &operator=(const Component &) = delete;

	virtual void OnModPauseCleanup()
	{
	}

	virtual void OnRun()
	{
	}

	virtual void OnKeyInput(DWORD key, bool repeated, bool isUpNow, bool isCtrlPressed, bool isShiftPressed,
	                        bool isAltPressed)
	{
	}
};