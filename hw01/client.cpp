#include "stdafx.h"

int client_mode()
{
	SOCKET socketC;

	init_winsock();
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
	//Wrong name
	int fsize = get_file_size(fname);
	if (fsize == -1)
	{
		printf("Wrong file path!");
		scanf("%s", fname);
		return EXIT_FAILURE;
	}

	FILE *fp = fopen(fname, "rb");
	if (!fp)
	{
		printf("File reading error\n");
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

	//send sha256
	char *sha256 = compute_sha256(fname);
	strcpy(buffer, "SHA256=");
	strcat(buffer, sha256);
	printf("%s\n", buffer);
	sendto(socketC, buffer, sizeof(buffer), 0, (sockaddr*)&serverInfo, len);
	ZeroMemory(buffer, sizeof(buffer));

	//send START
	strcpy(buffer, "START");
	printf("%s\n", buffer);
	sendto(socketC, buffer, sizeof(buffer), 0, (sockaddr*)&serverInfo, len);
	ZeroMemory(buffer, sizeof(buffer));


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
	//send STOP
	strcpy(buffer, "STOP");
	printf("%s\n", buffer);
	sendto(socketC, buffer, sizeof(buffer), 0, (sockaddr*)&serverInfo, len);
	ZeroMemory(buffer, sizeof(buffer));

	closesocket(socketC);
	return EXIT_SUCCESS;

}
