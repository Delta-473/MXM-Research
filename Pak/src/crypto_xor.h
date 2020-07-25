#pragma once
#include "base.h"

struct CryptoXOR
{
	u8* key = nullptr;
	i32 keySize;

	~CryptoXOR()
	{
		free(key);
	}

	void Init(const u8* key_, i32 keySize_);
	void Decrypt(void* data, const int dataSize, const int dataOffsetFromStart);
};
