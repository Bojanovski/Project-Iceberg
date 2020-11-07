
#ifndef EVENTS_D
#define EVENTS_D
#define NULL 0

#include <vector>

namespace Makina
{
	//Abstract event class
	class EventAbs
	{
	public:
		EventAbs() {}
		virtual ~EventAbs() {}

		virtual void FireEvent(void *argPt) = 0;
		virtual bool Compare(const EventAbs &evPt) = 0;
	};

	class GlobalEvent : public EventAbs
	{
	private:
		void (*methodPt)(void *argPt);

	public:
		GlobalEvent(void (*methodPt)(void *argPt)) : EventAbs(), methodPt(methodPt){}

		~GlobalEvent() {}

		void FireEvent(void *argPt)
		{
			methodPt(argPt);
		}

		bool Compare(const EventAbs &evPt)
		{
			const GlobalEvent *tempPt = dynamic_cast<const GlobalEvent *>(&evPt);

			if ((tempPt != NULL)
				&& (static_cast<const GlobalEvent *>(&evPt)->methodPt == this->methodPt))
				return true;

			return false;
		}
	};

	//Event that points to member method
	template <class someType>
	class Event : public EventAbs
	{
	private:
		void *object;
		void (someType::*methodPt)(void *argPt);

	public:
		Event(void *object, void (someType::*methodPt)(void *argPt)) : EventAbs(), object(object), methodPt(methodPt){}

		~Event() {}

		void FireEvent(void *argPt)
		{
			(static_cast<someType *>(object)->*methodPt)(argPt);
		}

		bool Compare(const EventAbs &evPt)
		{
			const Event<someType> *tempPt = dynamic_cast<const Event<someType> *>(&evPt);

			if ((tempPt != NULL)
				&& (static_cast<const Event<someType> *>(&evPt)->methodPt == this->methodPt)
				&& (static_cast<const Event<someType> *>(&evPt)->object == this->object))
				return true;

			return false;
		}
	};

	class EventHandler
	{
	private:
		std::vector<EventAbs *> mEvents;

	public:
		__declspec(dllexport) EventHandler();
		__declspec(dllexport) ~EventHandler();

		__declspec(dllexport) void FireEvents(void *argPt);
		__declspec(dllexport) bool RemoveEvent(const EventAbs &argEv);
		__declspec(dllexport) void RemoveEvent(int index);
		__declspec(dllexport) int CountEvents();

		__declspec(dllexport) bool operator+=(EventAbs *ev);
		__declspec(dllexport) bool operator-=(const EventAbs &argEv);
	};
}

#endif