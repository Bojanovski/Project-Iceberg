#include "Events.h"
#include <typeinfo>

using namespace Makina;

EventHandler::EventHandler() : mEvents() {}

EventHandler::~EventHandler(){}

void EventHandler::FireEvents(void *argPt)
{
	for (unsigned int i = 0; i < mEvents.size(); i++)
		mEvents[i]->FireEvent(argPt);
}

bool EventHandler::RemoveEvent(const EventAbs &argEv)
{
	for (unsigned int i = 0; i < mEvents.size(); i++)
		if (mEvents[i]->Compare(argEv))
		{
			mEvents.erase(mEvents.begin() + i);
			return true;
		}

		return false;
}

void EventHandler::RemoveEvent(int index)
{
	mEvents.erase(mEvents.begin() + index);
}

int EventHandler::CountEvents()
{
	return mEvents.size();
}

bool EventHandler::operator+=(EventAbs *ev)
{
	mEvents.push_back(ev);

	return true;
}

bool EventHandler::operator-=(const EventAbs &argEv)
{
	return this->RemoveEvent(argEv);
}
