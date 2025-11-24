#pragma once

#define _WIN32_WINNT 0x0600 // InetPton / inet_ntop

#include <iostream>
#include <vector>
#include <cstdio>
#include <cstdlib> 
#include <winsock2.h> 
#include <Ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")
