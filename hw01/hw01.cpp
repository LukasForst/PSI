// cplusProject.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <winsock2.h>
#include <stdio.h>
#include <cstdint>

#pragma comment (lib, "ws2_32.lib")
#pragma warring(disable: 4996)

#define PORT 4000
#define PACKET_MAX_LEN	1024
#define IP_ADDRESS "127.0.0.1"
#define PATH_MAX_LEN 256

int server_mode();
int client_mode();
void InitWinsock();
void InitWinsock();
int sizeOfFile();

void InitWinsock()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 1), &wsaData) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("Initialised.\n");
}


int main()
{
	printf("1 - for client mode\n2 - for server mode\n");
	char option;
	scanf("%c", &option);
	if (option == '1')
	{
		int ret = client_mode();

		char c;
		scanf("%c", &c);
		return ret;
	}
	else if (option == '2')
	{
		int ret = server_mode();
		char c;
		scanf("%c", &c);
		return ret;
	}
	else
	{
		printf("Please make a choice!\n");
		main();
	}
	return EXIT_SUCCESS;
}

int sizeOfFile(const char* fname)
{
	FILE *fp = fopen(fname, "rb");
	if (!fp) return -1;

	fseek(fp, 0L, SEEK_END);
	int size = ftell(fp);
	rewind(fp);
	fclose(fp);
	return size;
}

int client_mode()
{
	SOCKET socketC;

	InitWinsock();
	struct sockaddr_in serverInfo;
	int len = sizeof(serverInfo);
	serverInfo.sin_family = AF_INET;
	serverInfo.sin_port = htons(PORT);
	serverInfo.sin_addr.s_addr = inet_addr(IP_ADDRESS);

	socketC = socket(AF_INET, SOCK_DGRAM, 0);

	printf("File path: ");
	char fname[PATH_MAX_LEN];
	ZeroMemory(fname, sizeof(fname));

	scanf("%s", fname);

	//int fsize = 10;
	//Wrong name
	int fsize = sizeOfFile(fname);
	if (fsize == -1)
	{
		printf("Wrong file path!");
		scanf("%s", fname);
		return EXIT_FAILURE;
	}

	char buffer[PACKET_MAX_LEN];

	strcpy(buffer, "NAME=");
	strcat(buffer, fname);
	printf("%s\n", buffer);
	//send file name
	sendto(socketC, buffer, sizeof(buffer), 0, (sockaddr*)&serverInfo, len);
	ZeroMemory(buffer, sizeof(buffer));

	//send size
	strcpy(buffer, "SIZE=");
	//cast int to the char array
	char fsizeString[sizeof(int) * 3 + 2];
	snprintf(fsizeString, sizeof fsizeString, "%d", fsize);
	strcat(buffer, fsizeString);
	printf("%s\n", buffer);
	sendto(socketC, buffer, sizeof(buffer), 0, (sockaddr*)&serverInfo, len);
	ZeroMemory(buffer, sizeof(buffer));

	//send START
	strcpy(buffer, "START");
	printf("%s\n", buffer);
	sendto(socketC, buffer, sizeof(buffer), 0, (sockaddr*)&serverInfo, len);
	ZeroMemory(buffer, sizeof(buffer));

	FILE *fp = fopen(fname, "rb");
	if (!fp)
	{
		printf("File reading error\n");
		return EXIT_FAILURE;
	}
	int data_to_read = fsize;
	uint32_t fposition = ftell(fp);
	while (!feof(fp) && fposition < fsize)
	{
		strcpy(buffer, "DATA{");
		//weird workaround
		FILE *tmp = fopen("tmp.txt", "wb");
		fwrite(&fposition, sizeof(uint32_t), 1, tmp);
		fclose(tmp);
		Sleep(50);

		FILE *read = fopen("tmp.txt", "rb");
		char tmpData[4];
		fread(tmpData, sizeof(uint32_t), 1, read);
		for (int i = 0; i < 4; i++)
		{
			buffer[i + 5] = tmpData[i];
		}
		fclose(read);

		strcat(buffer, "}{");
		char data[PACKET_MAX_LEN - 11];
		if (data_to_read > PACKET_MAX_LEN - 14)
		{
			data_to_read -= PACKET_MAX_LEN - 14;
			fread(data, 1, PACKET_MAX_LEN - 14, fp);
			data[PACKET_MAX_LEN - 14] = '}';
			data[PACKET_MAX_LEN - 13] = '\0';
		}
		else
		{
			fread(data, 1, data_to_read, fp);
			data[data_to_read] = '}';
			data[data_to_read + 1] = '\0';
		}
		strcat(buffer, data);

		//buffer[PACKET_MAX_LEN - 2] = '}';
		//buffer[PACKET_MAX_LEN -1] = '\0';
		//printf("%s\n", buffer);
		int bracket = 0;
		for (int i = 0; i<PACKET_MAX_LEN; i++)
		{
			if (buffer[i] == '}') bracket++;
			if (bracket >= 2)
			{
				printf("%c", buffer[i]);

				break;
			}
			printf("%c", buffer[i]);
		}

		printf("\n");
		//send socket
		sendto(socketC, buffer, sizeof(buffer), 0, (sockaddr*)&serverInfo, len);
		ZeroMemory(buffer, sizeof(buffer));
		fposition = ftell(fp);
	}
	fclose(fp);
	//send START
	strcpy(buffer, "STOP");
	printf("%s\n", buffer);
	sendto(socketC, buffer, sizeof(buffer), 0, (sockaddr*)&serverInfo, len);
	ZeroMemory(buffer, sizeof(buffer));

	scanf("%s", fname);
	closesocket(socketC);
	return EXIT_SUCCESS;

}

int server_mode()
{

	SOCKET socketS;

	InitWinsock();
	struct sockaddr_in local;
	struct sockaddr_in from;
	int fromlen = sizeof(from);

	//set outcomming socket
	local.sin_family = AF_INET;
	local.sin_port = htons(PORT);
	local.sin_addr.s_addr = INADDR_ANY;

	socketS = socket(AF_INET, SOCK_DGRAM, 0);
	bind(socketS, (sockaddr*)&local, sizeof(local));

	char *fname = nullptr;
	int fsize;
	FILE *fp;
	while (true)
	{
		char buffer[PACKET_MAX_LEN];
		//fill it with zeros
		ZeroMemory(buffer, sizeof(buffer));
		if (recvfrom(socketS, buffer, sizeof(buffer), 0, (sockaddr*)&from, &fromlen) != SOCKET_ERROR)
		{
			int stringLen = strlen(buffer);
			if (strstr(buffer, "NAME"))
			{
				fname = (char *)calloc(stringLen, sizeof(char));
				for (int i = 0; buffer[i + 4] != '\0'; i++)
				{
					fname[i] = buffer[i + 5];
				}
				printf("NAME=%s\n", fname);
				fp = fopen(fname, "wb");
				Sleep(50);
				if (fp) fclose(fp);
			}
			else if (strstr(buffer, "SIZE"))
			{
				char *file_size = (char*)calloc(stringLen, sizeof(char));
				for (int i = 0; buffer[i + 4] != '\0'; i++)
				{
					file_size[i] = buffer[i + 5];
				}
				fsize = atoi(file_size);
				free(file_size);
				printf("SIZE=%i\n", fsize);
			}
			else if (strstr(buffer, "STOP")) {
				break;
			}
			else if (strstr(buffer, "DATA"))
			{
				fp = fopen(fname, "ab");
				char *received_data = (char*)calloc(stringLen, sizeof(char));
				if (!received_data) {
					printf("MEMORY ERROR");
					scanf("%s", fname);
					return EXIT_FAILURE;
				}
				char *ptr = (char*)calloc(4, sizeof(char));
				int idx = 0;
				int ptrIdx = 0;
				int bracket = 0;
				for (int i = 0; i<sizeof(buffer); i++)
				{
					char c = buffer[i];
					if (c == '}' && bracket >= 2) break;
					if (c == '{')
					{
						bracket++;
						continue;
					}
					if (bracket >= 2)
					{
						received_data[idx++] = c;
					}
					else if (bracket == 1 && c != '}')
					{
						ptr[ptrIdx++] = c;
					}
				}
				int whence = atoi(ptr);
				printf("PTR - %i\n", whence);
				fseek(fp, 0, whence);
				fwrite(received_data, idx, 1, fp);
				free(received_data);
				fclose(fp);
			}

			printf("%s\n", buffer);
		}
	}
	fclose(fp);
	closesocket(socketS);

	if (fname) free(fname);


	printf("Hit Enter to end...");
	char wait_for_enter;
	scanf("%c", &wait_for_enter);
	return EXIT_SUCCESS;
}