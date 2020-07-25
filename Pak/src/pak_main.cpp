#include <base.h>
#include <direct.h>
#include <shlwapi.h>
#include <brotli/decode.h>

#include "crypto_aes.h"
#include "crypto_xor.h"
#include "keys.h"

struct PakHeader
{
	u32 magic;
	u16 minorVersion;
	u16 majorVersion;

	i32 _unk1;
	i32 _unk2;

	i32 subHeaderSize;
	i32 destSizeRelated1;
	i32 destSizeRelated2;
	u16 encryptionRelated1;
	u16 encryptionRelated2;

	i32 _unk3;
};

STATIC_ASSERT(sizeof(PakHeader) == 36);

PUSH_PACKED
struct SubHeader
{
	i32 i1;
	i32 dataOffset;
	u32 extractedSize;
	u8 unk[8];
	i32 compressedSize;
	u16 u1;
	u16 u2;
	wchar_t name[1];
};
POP_PACKED

STATIC_ASSERT(sizeof(SubHeader)-2 == 28);

void PrintUsage()
{
	LOG("Usage: pak \"inputfile\" \"outputDir\"");
}

void PathGetFilename(const char* from, char* out)
{
	strcpy(out, from);
	PathStripPathA(out);
}

void PathGetDirectory(const char* from, char* out)
{
	strcpy(out, from);
	PathRemoveFileSpecA(out);
}

bool FileSaveBuff(const wchar_t* path, const void* data, const i32 dataSize)
{
	HANDLE hFile = CreateFileW(path, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == INVALID_HANDLE_VALUE) {
		LOG("ERROR(CreateFileW): %d", GetLastError());
		return false;
	}

	DWORD dwBytesToWrite = dataSize;
	BOOL bErrorFlag = WriteFile(hFile, data, dwBytesToWrite, &dwBytesToWrite, NULL);

	if(bErrorFlag == FALSE) {
		LOG("ERROR(WriteFile): %d", GetLastError());
		return false;
	}

	CloseHandle(hFile);
	return true;
}

bool UnpakSpecial_FileListXml(const char* outputDir, u8* fileData, i32 fileSize)
{
	LOG("Unpaking FileList.xml...");

	ConstBuffer fileBuff(fileData, fileSize);
	PakHeader& header = fileBuff.Read<PakHeader>();

	LOG("magic='%.4s' version=%d.%d", (char*)&header.magic, header.majorVersion, header.minorVersion);

	// decrypt
	CryptoAES ctx;

	const u8 key1[16] = {
		0xa0, 0x1d, 0x3d, 0x99,
		0x3b, 0x82, 0x0f, 0x1e,
		0x13, 0x0a, 0x89, 0x55,
		0x8d, 0xc0, 0xde, 0x22
	};

	const u8 key2[16] = {
		0x49, 0x59, 0x63, 0x55,
		0xfd, 0x61, 0x71, 0x01,
		0x00, 0xf3, 0xb9, 0xde,
		0xb6, 0x6f, 0xb2, 0xa5,
	};

	ctx.Init(key1, key2);
	ctx.Decrypt(&header._unk1, 28, 0);

	LOG("header = {");
	LOG("	subHeaderSize=%d", header.subHeaderSize);
	LOG("	destSizeRelated1=%d", header.destSizeRelated1);
	LOG("	destSizeRelated2=%d", header.destSizeRelated2);
	LOG("	encryptionRelated1=%d", header.encryptionRelated1);
	LOG("	encryptionRelated2=%d", header.encryptionRelated2);
	LOG("}");

	u8* subHeaderData = fileBuff.ReadRaw(header.subHeaderSize);
	ctx.Decrypt(subHeaderData, header.subHeaderSize, 28);

	const SubHeader& subHeader = *(SubHeader*)subHeaderData;
	LOG("subHeader = {");
	LOG("	i1=%#x", subHeader.i1);
	LOG("	dataOffset=%d", subHeader.dataOffset);
	LOG("	extractedSize=%d", subHeader.extractedSize);
	LOG_NNL("	unk=[ ");
	for(int i = 0; i < ARRAY_COUNT(subHeader.unk); i++) {
		LOG_NNL("%x,", subHeader.unk[i]);
	}
	LOG("]");
	LOG("	compressedSize=%d", subHeader.compressedSize);
	LOG("	u1=%d", subHeader.u1);
	LOG("	u2=%d", subHeader.u2);
	LOG("	name=%ws", subHeader.name);
	LOG("}");

	// reset file pointer
	fileBuff = ConstBuffer(fileData, fileSize);
	fileBuff.ReadRaw(subHeader.dataOffset); // skip offset

	u8* src = fileBuff.ReadRaw(subHeader.compressedSize); // read crypted data
	ctx.Decrypt(src, subHeader.compressedSize, 0);

	ASSERT(subHeader.extractedSize < ((u32)2 * (1024*1024*1024))); // < 2GB
	u8* dest = (u8*)malloc(subHeader.extractedSize);

	BrotliDecoderState* dec = BrotliDecoderCreateInstance(0, 0, 0);

	size_t decodedSize = subHeader.extractedSize;
	BrotliDecoderResult bdr = BrotliDecoderDecompress(subHeader.compressedSize, src, &decodedSize, dest);

	if(bdr == BrotliDecoderResult::BROTLI_DECODER_RESULT_SUCCESS) {
		wchar_t path[256] = {0};
		mbstowcs(path, outputDir, ARRAY_COUNT(path));

		_mkdir(outputDir);
		PathAppendW(path, subHeader.name);

		LOG("Writing '%ls'", path);
		bool r = FileSaveBuff(path, dest, decodedSize);
		if(!r) {
			LOG("ERROR(FileSaveBuff): failed to save file '%ls'", path);
			return false;
		}
	}
	else {
		LOG("ERROR(BrotliDecoderDecompress): could not decode input");
		return false;
	}

	BrotliDecoderDestroyInstance(dec);
	return true;
}

bool UnpakFile(const char* outputDir, u8* fileData, i32 fileSize)
{
	LOG("Unpaking...");

	CryptoXOR ctx;
	ctx.Init(g_XorKey, sizeof(g_XorKey));

	ConstBuffer fileBuff(fileData, fileSize);
	PakHeader& header = fileBuff.Read<PakHeader>();

	LOG("magic='%.4s' version=%d.%d", (char*)&header.magic, header.majorVersion, header.minorVersion);

	ctx.Decrypt(&header._unk1, 28, 0);

	LOG("header = {");
	LOG("	subHeaderSize=%d", header.subHeaderSize);
	LOG("	destSizeRelated1=%d", header.destSizeRelated1);
	LOG("	destSizeRelated2=%d", header.destSizeRelated2);
	LOG("	encryptionRelated1=%d", header.encryptionRelated1);
	LOG("	encryptionRelated2=%d", header.encryptionRelated2);
	LOG("}");

	return true;
}

int main(int argc, char** argv)
{
	LogInit("pak.log");
	LOG(".: Pak Unpacker :.");

	if(argc == 3) {
		const char* inputPath = argv[1];
		const char* outputDir = argv[2];

		// open and read input file
		i32 fileSize;
		u8* fileData = fileOpenAndReadAll(inputPath, &fileSize);
		if(!fileData) {
			LOG("ERROR: could not open file '%s'", inputPath);
			return 1;
		}

		char filename[256];
		PathGetFilename(inputPath, filename);
		if(strcmp(filename, "FileList.xml.pak") == 0) {
			UnpakSpecial_FileListXml(outputDir, fileData, fileSize);
		}
		else {
			UnpakFile(outputDir, fileData, fileSize);
		}

		LOG("Done.");
		return 0;
	}

	PrintUsage();
	return 1;
}
