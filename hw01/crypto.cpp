#include "stdafx.h"
#include "crypto.h"

//need to be freed
char * compute_crc32(const char* data) {
	CRC32 crc32;
	std::string myHash = crc32(data);     // std::string
	char *hash = (char*)calloc(sizeof(char), CRC32_SIZE);
	strcpy(hash, myHash.c_str());
	return hash;
}

//need to be freed
char* compute_sha256(const char *fname) {
	int fsize = get_file_size(fname);
	char * file = (char*)calloc(sizeof(char), fsize);
	FILE *fp = fopen(fname, "rb");
	if (!fp) exit(100);
	fread(file, fsize, 1, fp);

	SHA256 sha256;
	char* result = (char*)calloc(sizeof(char), SHA256_SIZE);
	std::string tmp = sha256(file, fsize);
	strcpy(result, tmp.c_str());
	free(file);
	return result;
}

int verify_sha256(const char*fname, const char* received_sha256) {
	char *file_sha256 = compute_sha256(fname);
	for (int i = 0; i < SHA256_SIZE; i++) {
		if (file_sha256[i] != received_sha256[i])
			return 0;
	}
	return 1;
}

int verify_crc32(const char*given_crc, const char* buffer) {
	char * new_crc = compute_crc32(buffer);
	for (int i = 0; i < CRC32_SIZE; i++) {
		if (new_crc[i] != given_crc[i])
			return 0;
	}
	return 1;
}
