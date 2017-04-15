// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"
#include "crypto.h"

#include <stdio.h>
#include <tchar.h>
#include <winsock2.h>
#include <stdio.h>
#include <cstdint>
#include <iostream>
#include <inttypes.h>

#pragma comment (lib, "ws2_32.lib")
#pragma warring(disable: 4996)

#define PACKET_MAX_LEN	1024
#define PATH_MAX_LEN 256

#define IP_ADDRESS_SERVER "127.0.0.1"
#define PORT_SERVER 4000

#define IP_ADDRESS_CLIENT "127.0.0.1"
#define PORT_CLIENT 3840

#define TIMEOUT_S 10000
#define PACKET_MAX_LOSS 4

#define TRUE 1
#define FALSE 0

#define _CRT_SECURE_NO_WARNINGS 1



int server_mode();
int client_mode();

void init_winsock();
int get_file_size(const char* fname);
