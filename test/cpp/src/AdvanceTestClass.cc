#include "AdvanceTestClass.h"
#include <cstdio>

AdvanceTestClass::AdvanceTestClass(int A)
{
    printf("AdvanceTestClass::AdvanceTestClass(%d)", A);
}

void AdvanceTestClass::StdFunctionTest(std::function<int(int, int)> Func)
{
    int Ret = Func(88, 99);
    printf("AdvanceTestClass::StdFunctionTest Callback Ret %d\n", Ret);
}
