#include "stdafx.h"
#define DEBUG 0

int start_it();	

int main()
{
	char *hash = compute_crc32("Hello World");

	for (int i = 0; i < 8; i++) {
		printf("%c", hash[i]);
	} 
	printf("\n");


	int return_value = DEBUG ? 0 : start_it();
	printf("Hit Enter to end...");
	char wait_for_enter;
	scanf("%c", &wait_for_enter);
	return return_value;
	//int ret = start_it();
	//return ret;
}
int start_it() {
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

