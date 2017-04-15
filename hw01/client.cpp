#include "stdafx.h"


void send_data(char *buffer, size_t size_of_buffer);

int client_mode()
{
	init_winsock();

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
	
	//sedn file name
	strcpy(buffer, "NAME=");
	strcat(buffer, fname);
	send_data(buffer, sizeof(buffer));

	//send size
	strcpy(buffer, "SIZE=");
	//cast int to the char array
	char fsizeString[sizeof(int) * 3 + 2];
	snprintf(fsizeString, sizeof fsizeString, "%d", fsize);
	strcat(buffer, fsizeString);	
	send_data(buffer, sizeof(buffer));

	//send sha256
	char *sha256 = compute_sha256(fname);
	strcpy(buffer, "SHA256=");
	strcat(buffer, sha256);
	send_data(buffer, sizeof(buffer));

	//send START
	strcpy(buffer, "START");	
	send_data(buffer, sizeof(buffer));

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
		send_data(buffer, sizeof(buffer));
	
		fposition = ftell(fp);
	}
	fclose(fp);
	//send STOP
	strcpy(buffer, "STOP");
	send_data(buffer, sizeof(buffer));
	
	return EXIT_SUCCESS;
}


void send_data(char *buffer, size_t size_of_buffer) {
	char packet[PACKET_MAX_LEN + CRC32_SIZE];
	SOCKET socketC;

	struct sockaddr_in serverInfo;
	int len = sizeof(serverInfo);
	serverInfo.sin_family = AF_INET;
	serverInfo.sin_port = htons(PORT_SERVER);
	serverInfo.sin_addr.s_addr = inet_addr(IP_ADDRESS_SERVER);

	socketC = socket(AF_INET, SOCK_DGRAM, 0);

	char * crc = compute_crc32(buffer);
	strcpy(packet, crc);
	strcat(packet, buffer);
	printf("%s -- %s\n", crc, buffer);
	sendto(socketC, packet, sizeof(packet), 0, (sockaddr*)&serverInfo, len);
	
	//TODO implement ARQ stop-and-wait

	ZeroMemory(buffer, size_of_buffer);
	ZeroMemory(packet, sizeof(packet));
	closesocket(socketC);
}
