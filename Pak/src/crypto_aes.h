#pragma once
#include "base.h"


struct CryptoAES
{
	u8 key1[16];
	u8 key2[16];
	u32 key1Expansion1[44] = {0};
	u32 key1Expansion2[44] = {0};

	void Init(const u8* key1_, const u8* key2_);

	void Key2AddOffset(u8* out, i32 offset);
	void ComputeXorBlock(u32* key);
	void KeyAlteration(u8* key);
	void Decrypt(void* data, const int dataSize, const int dataOffsetFromStart);
};
