#pragma once

class CombatFreeRotateCamera;

/// Original Idea From iRetrospect. Thanks.
// Similar Combat Free Camera.
class CombatFreeRotateCamera : public RE::BSTEventSink<SKSE::ActionEvent>
{
public:
	RE::BSEventNotifyControl ProcessEvent(const SKSE::ActionEvent* a_event, RE::BSTEventSource<SKSE::ActionEvent>* a_eventSource) override;
	static void Install();
};

