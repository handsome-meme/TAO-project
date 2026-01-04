#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <string>
#include <cstring>
#include <map>
#include <fstream>
#include "BOBHASH32.h"
#include "params.h"
#include "vx_ssummary.h"
#include "vx_spacesaving.h"
#include "software_sort.h"
using namespace std;

int main()
{
    software_sort test;
    freopen("test.in","r",stdin);
    uint64_t a,b,c;
    while (cin>>a>>b>>c)
    {
        test.insert(a,b,c);
    }
    return 0;
}