#include "stdafx.h"

void send_data(char *buffer, size_t size_of_buffer);
int arq_stopnwait(long int segment_id);

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
		strcat(buffer, (char*)&fposition);
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

static uint32_t socket_id = 0;
static uint32_t same_packet = 0;

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
	printf("%" PRIu32 " - %s -- %s\n", socket_id, crc, buffer);
	sendto(socketC, packet, sizeof(packet), 0, (sockaddr*)&serverInfo, len);
	
	//TODO implement ARQ stop-and-wait
	if (arq_stopnwait(socket_id)) {
		ZeroMemory(buffer, size_of_buffer);
		ZeroMemory(packet, sizeof(packet));
		closesocket(socketC);
		socket_id++;
		same_packet = 0;
	}
	else {
		if (same_packet > PACKET_MAX_LOSS) {
			printf("Connection ERROR!\nMore then %d packets were lost!\n", PACKET_MAX_LOSS);
			exit(100);
		}
		same_packet++;
		send_data(buffer, size_of_buffer);
	}
}

int arq_stopnwait(long int segment_id) {
	SOCKET socketS;
	struct sockaddr_in local;
	struct sockaddr_in from;
	int fromlen = sizeof(from);

	//set outcomming socket
	local.sin_family = AF_INET;
	local.sin_port = htons(PORT_CLIENT);
	local.sin_addr.s_addr = INADDR_ANY;

	socketS = socket(AF_INET, SOCK_DGRAM, 0);
	bind(socketS, (sockaddr*)&local, sizeof(local));


	struct timeval tv;
	tv.tv_sec = TIMEOUT_S;
	tv.tv_usec = 0;

	// Set up the file descriptor set.
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(socketS, &fds);
	// Wait until timeout or data received.
	int n = select(socketS, &fds, NULL, NULL, &tv);
	if (n == 0)
	{
		printf("Timeout reached. Resending segment: %" PRIu32 "\n", segment_id);
		closesocket(socketS);
		return FALSE;
	}
	else if (n == -1)
	{
		printf("Error..\n");
		closesocket(socketS);
		return FALSE;
	}

	char packet[PACKET_MAX_LEN + CRC32_SIZE];
	if (recvfrom(socketS, packet, sizeof(packet), 0, (sockaddr*)&from, &fromlen) < 0) {
		//timeout reached
		printf("Timeout reached. Resending segment: %" PRIu32 "\n", segment_id);
		closesocket(socketS);
		return FALSE;
	}
	closesocket(socketS);
	return TRUE;
}
