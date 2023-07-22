#pragma once

#include <string>
#include <functional>

class AdvanceTestClass
{
public:
    AdvanceTestClass(int A);

    void StdFunctionTest(std::function<int(int, int)> Func);
};