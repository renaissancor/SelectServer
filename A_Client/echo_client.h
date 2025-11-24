#pragma once

#define _WIN32_WINNT 0x0600 // InetPton / inet_ntop
#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <cstdio>
#include <cstdlib> 
#include <winsock2.h> 
#include <Ws2tcpip.h>
#include <Windows.h>
#include <process.h> 

#pragma comment(lib, "Ws2_32.lib")
