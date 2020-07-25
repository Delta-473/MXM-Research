#include "crypto_xor.h"

void CryptoXOR::Init(const u8* key_, i32 keySize_)
{
	key = (u8*)malloc(keySize_);
	keySize = keySize_;
	memmove(key, key_, keySize_);
}

void CryptoXOR::Decrypt(void* data, const int dataSize, const int dataOffsetFromStart)
{
	u8* data1 = (u8*)data;
	for(int i = 0; i < dataSize; i++) {
		data1[i] ^= key[(i + dataOffsetFromStart) % keySize]; // TODO: @Speed. We know keySize is 0x4000, we can use & here instead of %
	}
}
