#include "stdafx.h"
#include "Session.h"
#include "Network.h"


int main() {

	Network& network = Network::GetInstance();
	if (!network.Initialize()) {
		fprintf_s(stderr, "Failed to initialize network.\n");
		return -1;
	}
	network.RunEngine();
	network.Shutdown(); 

	return 0;
}