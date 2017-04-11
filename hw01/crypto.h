#pragma once
#include "hash-library\sha256.h"
#include "hash-library\crc32.h"

#define SHA256_SIZE 64
#define CRC32_SIZE 8

char * compute_sha256(const char *fname);
char * compute_crc32(const char* data);

int verify_sha256(const char*fname, const char* received_sha256);