// Demo: add two integers and print the result
#include <iostream>

// Pure function: returns the sum of two ints
int add(int a, int b)
{
    return a + b;
}

int main()
{
    int s = add(1, 2);
    // Only print positive results
    if (s > 0)
    {
        std::cout << s << std::endl;
    }
    return 0;
}
