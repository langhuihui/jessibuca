#pragma once
typedef unsigned char u8;
typedef signed char i8;
typedef unsigned short u16;
typedef signed short i16;
typedef unsigned int u32;
typedef signed int i32;
#define readu8 readB<1, u8>
#define readUInt16B readB<2, unsigned short>
#define readUInt24B readB<3, unsigned int>
#define readUInt32B readB<4, unsigned int>
#define readDoubleB readB<8, double>
#define read1 readB<1>
#define read2B readB<2>
#define read3B readB<3>
#define read4B readB<4>
#define read8B readB<8>
#define OPERATOR_CHARS(x) \
operator const char *()\
{\
	return (const char *)x;\
}\
operator char* ()\
{\
	return (char*)x;\
}\
operator const unsigned char* ()\
{\
	return (const unsigned char*)x; \
}\
operator unsigned char* ()\
{\
	return (unsigned char*)x; \
}

template <int N>
inline void reverse(char* a, char* b)
{
	*a = *b;
	reverse<N - 1>(a + 1, b - 1);
}
template <>
inline void reverse<1>(char* a, char* b)
{
	*a = *b;
}
struct SharedStr
{
    string data;
    int refCount;
	SharedStr(size_t size):refCount(1) {
		data.reserve(size);
	}
    SharedStr(string &&right) : data(move(right)),refCount(1)
    {
    }
    
    void free()
    {
        refCount--;
        if (refCount == 0)
        {
            delete this;
        }
    }
	void copy(SharedStr** target) {
		*target = this;
		refCount++;
	}
	void expend(size_t t) {
		data.resize(data.capacity() + t);
	}
	size_t capacity() {
		return data.capacity();
	}
	OPERATOR_CHARS(data.data())
};
struct Slice
{
	SharedStr* sharedStr;
	int p;
	int length;
	virtual ~Slice()
	{
		sharedStr->free();
	}
	Slice():Slice("") {

	}
	Slice(const Slice& right) : p(right.p),length(right.length)
	{
		right.sharedStr->copy(&sharedStr);
	}
	Slice& operator=(const Slice& right)  {
		right.sharedStr->copy(&sharedStr);
		p = right.p;
		length = right.length;
		return *this;
	}
	Slice(string&& data):p(0)
	{
		length = data.length();
		sharedStr = new SharedStr(move(data));
	}
	Slice(Slice* parent, int start, int end)
	{
		parent->sharedStr->copy(&sharedStr);
		p = parent->p + start;
		length = end - start;
	}
	Slice(int len, int cap) : length(len),p(0)
	{
		if (!cap)cap = len;
		sharedStr = new SharedStr(cap);
	}
	size_t cap() {
		return sharedStr->capacity() - p;
	}
	Slice operator()(int start, int end)
	{
		return Slice(this, start, end);
	}
	unsigned char operator[](int i)
	{
		return ((unsigned char*)(*sharedStr))[p+i];
	}
	Slice operator+(const Slice& right) {
		return append((void*)right.point(), right.length);
	}
	const char* point() const{
		return sharedStr->data.data() + p;
	}
	Slice append(void * data ,int len) {
		if (len > cap() - length) {
			string newStr;
			newStr.resize(p + length + len);
			memcpy((void*)newStr.data(), sharedStr->data.data(), p + length);
			memcpy((void*)(newStr.data() + p + length), data, len);
			return Slice(move(newStr));
		}
		memcpy((void*)(point() + length), data, len);
		return Slice(this, 0, length + len);
	}
	OPERATOR_CHARS((*sharedStr)+p)
};
struct IOBuffer :public Slice {
	using Slice::Slice;
	IOBuffer(const Slice& right):Slice(right)
	{
	}
	IOBuffer& operator=(const Slice& right) {
		right.sharedStr->copy(&sharedStr);
		p = right.p;
		length = right.length;
		return *this;
	}
	IOBuffer& operator>>=(const int& i)
	{
		p += i;
		length -= i;
		return *this;
	}
	IOBuffer& operator<<=(const int& i)
	{
		p -= i;
		length += i;
		return *this;
	}
	IOBuffer& operator<<(string&& right)
	{
		const auto rightLen = right.length();
		if (rightLen > cap() - length) {
			sharedStr->expend(rightLen - (cap() - length));
		}
		memcpy((void*)(point() + length), right.data(), rightLen);
		length += rightLen;
		return *this;
	}
	IOBuffer& operator<<(const IOBuffer& right)
	{
		const auto rightLen = right.length;
		if (rightLen > cap() - length) {
			sharedStr->expend(rightLen - (cap() - length));
		}
		memcpy((void*)(point() + length), right.point(), rightLen);
		length += rightLen;
		return *this;
	}
	string readString(size_t len = string::npos)
	{
		string result = sharedStr->data.substr(p, len);
		(*this) >>= result.length();
		return result;
	}
	template <int N, class T>
	IOBuffer& readB(T& out)
	{
		reverse<N>((char*)& out, (char*)point() + N - 1);
		(*this) >>= N;
		return *this;
	}
	template <int N, class T>
	T readB()
	{
		T out;
		reverse<N>((char*)& out, (char*)point() + N - 1);
		(*this) >>= N;
		return out;
	}
	void removeConsume()
	{
		memmove((void*)sharedStr->data.data(), point(), length);
		sharedStr->data.resize(length);
		p = 0;
	}
	void clear()
	{
		p = 0;
		length = 0;
	}
};