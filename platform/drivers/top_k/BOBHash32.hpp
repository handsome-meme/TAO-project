#pragma once

#include <stdio.h>
using namespace std;

typedef unsigned int uint;
typedef unsigned long long int uint64;


#define MAX_PRIME32 1229
#define MAX_BIG_PRIME32 50

class BOBHash32
{
public:
	BOBHash32();
	~BOBHash32();
	BOBHash32(uint prime32Num);
	void initialize(uint prime32Num);
	uint run(const char * str, uint len);
private:
	uint prime32Num;
};

#define mix(a,b,c) \
{ \
a -= b; a -= c; a ^= (c>>13); \
b -= c; b -= a; b ^= (a<<8); \
c -= a; c -= b; c ^= (b>>13); \
a -= b; a -= c; a ^= (c>>12);  \
b -= c; b -= a; b ^= (a<<16); \
c -= a; c -= b; c ^= (b>>5); \
a -= b; a -= c; a ^= (c>>3);  \
b -= c; b -= a; b ^= (a<<10); \
c -= a; c -= b; c ^= (b>>15); \
}

// #define 需要写在一行内，若内容太长，则可以使用 \ 表示换行
//define 声明需要回车几行，表示结束。