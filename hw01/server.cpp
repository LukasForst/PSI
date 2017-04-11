#include "stdafx.h"

int server_mode()
{

	SOCKET socketS;

	init_winsock();
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
	char sha256[SHA256_SIZE];
	int fsize;
	FILE *fp;
	while (true)
	{
		char packet[PACKET_MAX_LEN + CRC32_SIZE];
		//fill it with zeros
		ZeroMemory(packet, sizeof(packet));
		if (recvfrom(socketS, packet, sizeof(packet), 0, (sockaddr*)&from, &fromlen) != SOCKET_ERROR)
		{
			//compute crc
			char crc[CRC32_SIZE];
			for (int i = 0; i < CRC32_SIZE; i++) {
				printf("%c", packet[i]);
				crc[i] = packet[i];
			}
			printf(" -- ");
			char buffer[PACKET_MAX_LEN];
			ZeroMemory(buffer, sizeof(buffer));

			//make buffer, without crc
			for (int i = 0; i < PACKET_MAX_LEN; i++) {
				buffer[i] = packet[i + CRC32_SIZE];
			}
			//parse data
			int stringLen = strlen(buffer);
			if (strstr(buffer, "NAME"))
			{
				fname = (char *)calloc(stringLen, sizeof(char));
				for (int i = 0; buffer[i + 4] != '\0'; i++)
				{
					fname[i] = buffer[i + 5];
				}
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
			}
			else if (strstr(buffer, "STOP")) {
				printf("STOP\n");
				break;
			}
			else if (strstr(buffer, "DATA"))
			{
				fp = fopen(fname, "ab");
				if (!fp) {
					printf("FILE ERROR!!\n");
					return EXIT_FAILURE;
				}
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
				fseek(fp, 0, whence);
				fwrite(received_data, idx, 1, fp);
				free(received_data);
				fclose(fp);
			}
			//SHA256=
			else if (strstr(buffer, "SHA256"))
			{
				for (int i = 0; i < SHA256_SIZE; i++) {
					sha256[i] = buffer[i + 7];
				}
			}
			printf("%s\n", buffer);
			printf("Verify CRC: %d\n", verify_crc32(crc, buffer));
		}
	}
	fclose(fp);
	closesocket(socketS);

	printf("Transfer: %d\n", verify_sha256(fname, sha256));

	//if (fname) free(fname);
	return EXIT_SUCCESS;
}