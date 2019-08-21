#include <iostream>
#include <stdlib.h>
using namespace std;
#include "../slice.h"
void test() {
	IOBuffer t("abcdefg");
	IOBuffer tt = t(2, 4);
	IOBuffer ttt = tt;
}
int main()
{
	test();
}