#include "stdafx.h"

void init_winsock()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 1), &wsaData) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("Initialised.\n");
}

int get_file_size(const char* fname)
{
	FILE *fp = fopen(fname, "rb");
	if (!fp) return -1;

	fseek(fp, 0L, SEEK_END);
	int size = ftell(fp);
	rewind(fp);
	fclose(fp);
	return size;
}