#include "External/doctest/doctest.h"

int dummyFactorial(int number) {
    return number <= 1 ? number : dummyFactorial(number - 1) * number;
}

TEST_CASE("testing the factorial function") {
    CHECK(dummyFactorial(1) == 1);
    CHECK(dummyFactorial(2) == 2);
    CHECK(dummyFactorial(3) == 6);
    CHECK(dummyFactorial(10) == 3628800);
}
