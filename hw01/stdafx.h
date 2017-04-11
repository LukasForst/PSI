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

#pragma comment (lib, "ws2_32.lib")
#pragma warring(disable: 4996)

#define PORT 4000
#define PACKET_MAX_LEN	1024
#define IP_ADDRESS "127.0.0.1"
#define PATH_MAX_LEN 256

#define _CRT_SECURE_NO_WARNINGS 1


int start_it();
int server_mode();
int client_mode();
void init_winsock();
int get_file_size(const char* fname);
