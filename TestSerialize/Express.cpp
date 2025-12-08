#include "stdafx.h"
#include "Express.h"
#include "Session.h"
#include "Network.h"
#include "Stub.h"

void Express::ProcessRecvPackets() noexcept {
	while (!_recvPackets.empty()) {
		std::pair<int, Packet*> entry = _recvPackets.front();
		int sessionIndex = entry.first;
		Packet* packet = entry.second;
		_recvPackets.pop();

		Session& session = Network::GetInstance().GetSession(sessionIndex);
		if (session.GetSocket() == INVALID_SOCKET) {
			delete packet;
			continue;
		}

		uint8_t code = packet->GetCode();
		if(code != 0x89) {
			delete packet;
			continue; // Invalid packet code, handle error or ignore
		}
		// switch case on packet type 

		ProcessPacket(sessionIndex, packet); // Assuming sessionIndex is 0 for this example 


		delete packet; // Clean up the packet after processing
	}
}

// Call from L7 after creating Packet 

void Express::Unicast(int sessionIndex, Packet* packet) noexcept {
	Session& session = Network::GetInstance().GetSession(sessionIndex);
	if (session.GetSocket() == INVALID_SOCKET) return; 
	session.Send(*packet); 
}

void Express::BroadcastExcept(int excludeSessionIndex, Packet* packet) noexcept {
	Session& excludeSession = Network::GetInstance().GetSession(excludeSessionIndex); 
	for (int i = 0; i < Network::SESSION_MAX; ++i) {
		if (i == excludeSessionIndex) continue;
		Session& session = Network::GetInstance().GetSession(i);
		if (session.GetSocket() == INVALID_SOCKET) continue; 
		session.Send(*packet); 
	}
}

void Express::Broadcast(Packet* packet) noexcept {
	for (int i = 0; i < Network::SESSION_MAX; ++i) {
		Session& session = Network::GetInstance().GetSession(i);
		if (session.GetSocket() == INVALID_SOCKET) continue; 
		session.Send(*packet); 
	}
}
