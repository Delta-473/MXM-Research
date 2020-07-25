#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h> // memmove

extern FILE* g_LogFile;
void LogInit(const char* name);
#define LOG(fmt, ...) do { printf(fmt "\n", __VA_ARGS__); fprintf(g_LogFile, fmt "\n", __VA_ARGS__); } while(0)
#define LOG_NNL(fmt, ...) do { printf(fmt, __VA_ARGS__); fprintf(g_LogFile, fmt, __VA_ARGS__); } while(0)

#define STATIC_ASSERT(cond) static_assert(cond, #cond)

inline void __assertion_failed(const char* cond, const char* file, int line)
{
	LOG("Assertion failed (%s : %d): %s", file, line, cond);
	fflush(stdout);
	fflush(g_LogFile);
	__debugbreak();
}

#define ASSERT(cond) do { if(!(cond)) { __assertion_failed(#cond, __FILE__, __LINE__); } } while(0)
#define ASSERT_MSG(cond, msg) do { if(!(cond)) { __assertion_failed(#cond " (" #msg ")", __FILE__, __LINE__); } } while(0)
#ifdef CONF_DEBUG
	#define DBG_ASSERT(cond) ASSERT(cond)
#else
	#define DBG_ASSERT(cond)
#endif

#define ARRAY_COUNT(A) (sizeof(A)/sizeof(A[0]))
#define PUSH_PACKED __pragma(pack(push, 1))
#define POP_PACKED __pragma(pack(pop))

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float f32;

inline void* memAlloc(i64 size)
{
	//ProfileBlock("memAlloc");
	void* ptr = malloc(size);
	//ProfileMemAlloc(ptr, size);
	return ptr;
}

inline void* memRealloc(void* inPtr, i64 size)
{
	//ProfileBlock("memRealloc");
	//ProfileMemFree(inPtr);
	void* ptr = realloc(inPtr, size);
	//ProfileMemAlloc(ptr, size);
	return ptr;
}

inline void memFree(void* ptr)
{
	//ProfileBlock("memFree");
	//ProfileMemFree(ptr)
	return free(ptr);
}


struct Buffer
{
	u8* data;
	i32 size;
	i32 capacity;

	Buffer(i32 capacity_)
	{
		data = (u8*)memAlloc(capacity_);
		size = 0;
		capacity = capacity_;
	}

	~Buffer()
	{
		free(data);
	}

	void Clear()
	{
		size = 0;
	}

	void Append(const void* ap, i32 apSize)
	{
		ASSERT(size + apSize <= capacity);
		memmove(data + size, ap, apSize);
		size += apSize;
	}

	void AppendString(const char* str)
	{
		const i32 apSize = strlen(str);
		ASSERT(size + apSize <= capacity);
		memmove(data + size, str, apSize);
		size += apSize;
	}
};

inline u8* fileOpenAndReadAll(const char* filename, i32* pOutSize)
{
	FILE* f = fopen(filename, "rb");
	if(f) {
		fseek(f, 0, SEEK_END);
		int size = ftell(f);
		fseek(f, 0, SEEK_SET);

		u8* buff = (u8*)memAlloc(size + 1);
		fread(buff, size, 1, f);
		buff[size] = 0;
		*pOutSize = size;

		fclose(f);
		return buff;
	}

	return nullptr;
}

inline bool fileSaveBuff(const char* filename, const void* buff, i32 size)
{
	FILE* f = fopen(filename, "wb");
	if(f) {
		fwrite(buff, 1, size, f);
		fclose(f);
		return true;
	}

	return false;
}

static const char* _TempStrFormat(const char* fmt, ...)
{
	thread_local char buff[4096];

	va_list list;
	va_start(list, fmt);
	vsnprintf(buff, sizeof(buff), fmt, list);
	va_end(list);

	return buff;
}

#define FMT(...) _TempStrFormat(__VA_ARGS__)

// Dynamic array holding POD structures
template<typename T, u32 STACK_COUNT = 1>
struct Array
{
	T _stackData[STACK_COUNT];
	T* _data = _stackData;
	i64 _capacity = STACK_COUNT;
	i64 _count = 0;
	void* _memBlock = nullptr;

	Array() = default;

	template<u32 from_staticCount>
	Array(const Array<T, from_staticCount>& from) {
		Copy(from);
	}

	Array(const Array& from) {
		Copy(from);
	}

	// deep copy
	template<u32 from_staticCount>
	void Copy(const Array<T, from_staticCount>& from) {
		Clear();
		if(from._count > _capacity) {
			Reserve(from._count);
		}
		_count = from._count;
		memmove(_data, from._data, _count * sizeof(T));
	}

	inline Array& operator=(const Array& other) {
		Copy(other);
		return *this;
	}

	~Array() {
		release();
	}

	inline void release() {
		if(_memBlock) {
			memFree(_memBlock);
			_memBlock = nullptr;
		}
	}

	void Reserve(u32 newCapacity) {
		if(newCapacity <= _capacity) return;
		_memBlock = memRealloc(_memBlock, newCapacity * sizeof(T));
		ASSERT_MSG(_memBlock, "Out of memory");

		T* oldData = _data;
		_data = (T*)_memBlock;
		_capacity = newCapacity;

		if(oldData == _stackData) {
			memmove(_data, _stackData, sizeof(_stackData));
		}
	}

	T& Push(const T& elt) {
		if(_count + 1 > _capacity) {
			Reserve(_capacity * 2);
		}
		return (_data[_count++] = elt);
	}

	T& PushMany(const T* elements, const u32 eltCount) {
		if(_count + eltCount > _capacity) {
			Reserve(MAX(_capacity * 2, _count + eltCount));
		}

		i64 start = _count;
		_count += eltCount;
		memmove(_data + start, elements, eltCount * sizeof(T));
		return _data[start];
	}

	void Fill(const i32 eltCount, const T& elt = {}) {
		if(eltCount > _capacity) {
			Reserve(MAX(_capacity * 2, eltCount));
		}
		_count = eltCount;
		for(u32 i = 0; i < _count; ++i) {
			_data[i] = elt;
		}
	}

	inline void RemoveByID(i32 id) {
		ASSERT_MSG(_count > 0 && id < _count, "Element out of range");
		memmove(_data + id, _data + (_count -1), sizeof(T));
		--_count;
	}

	// Swap with last
	inline void Remove(const T& elt) {
		const T* eltPtr = &elt;
		ASSERT_MSG(eltPtr >= _data && eltPtr < _data + _count, "Element out of range");

		u32 id = eltPtr - _data;
		memmove(_data + id, _data + (_count -1), sizeof(T));
		--_count;
	}

	inline void Resize(i32 count_) {
		if(count_ > _capacity) {
			Reserve(MAX(_capacity * 2, count_));
		}
		_count = count_;
	}

	inline void Clear() {
		_count = 0;
	}

	inline u32 Capacity() const {
		return _capacity;
	}

	inline u32 Count() const {
		return _count;
	}

	inline u32 DataSize() const {
		return _count * sizeof(T);
	}

	inline T* Data() {
		return _data;
	}

	inline const T* Data() const {
		return _data;
	}

	inline T& operator[](u32 index) {
		ASSERT(index < _count);
		return _data[index];
	}

	inline const T& operator[](u32 index) const {
		ASSERT(index < _count);
		return _data[index];
	}

	inline void ShrinkTo(u32 num) {
		ASSERT(num < _count);
		_count = num;
	}
};

struct ConstBuffer
{
	u8* data;
	u8* cursor;
	i32 size;

	ConstBuffer(const void* ptr, i32 size_)
	{
		data = (u8*)ptr;
		cursor = (u8*)ptr;
		size = size_;
	}

	template<typename T>
	inline T& Read()
	{
		ASSERT((cursor - data) + sizeof(T) <= size);
		T& t = *(T*)cursor;
		cursor += sizeof(T);
		return t;
	}

	inline u8* ReadRaw(i32 len)
	{
		ASSERT((cursor - data) + len <= size);
		u8* r = cursor;
		cursor += len;
		return r;
	}
};

inline bool StringEquals(const char* str1, const char* str2)
{
	const i32 len = strlen(str1);
	if(len != strlen(str2)) {
		return false;
	}
	return strncmp(str1, str2, len) == 0;
}

inline void logAsHex(const void* data, int size)
{
	LOG_NNL("[ ");
	const u8* b = (u8*)data;
	for(int i = 0; i < size; i++) {
		LOG_NNL("%x, ", b[i]);
	}
	LOG_NNL("]");
}

// https://www.gingerbill.org/article/2015/08/19/defer-in-cpp/
template <typename F>
struct privDefer {
	F f;
	privDefer(F f) : f(f) {}
	~privDefer() { f(); }
};

template <typename F>
privDefer<F> defer_func(F f) {
	return privDefer<F>(f);
}

#define DEFER_1(x, y) x##y
#define DEFER_2(x, y) DEFER_1(x, y)
#define DEFER_3(x)    DEFER_2(x, __COUNTER__)
#define defer(code)   auto DEFER_3(_defer_) = defer_func([&](){code;})
