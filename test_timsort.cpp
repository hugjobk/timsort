#include "timsort.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <stdlib.h>
#include <time.h>

#ifndef ARR_SIZE
#define ARR_SIZE 100000000
#endif

int main()
{
    srand(time(NULL));

    std::vector<int> v1(ARR_SIZE);
    std::vector<int> v2(ARR_SIZE);

    for (int i = 0; i < ARR_SIZE; ++i)
    {
        int r = rand();
        v1[i] = r;
        v2[i] = r;
    }

    auto start1 = std::chrono::high_resolution_clock::now();
    std::sort(v1.begin(), v1.end());
    auto end1 = std::chrono::high_resolution_clock::now();

    auto start2 = std::chrono::high_resolution_clock::now();
    timsort::tim_sort(v2.data(), v2.size()); // Faster
    // timsort::tim_sort(v2.begin(), v2.end()); // Slower
    auto end2 = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < ARR_SIZE; ++i)
    {
        if (v1[i] != v2[i])
        {
            std::cout << "Incorrrect Sort!!!" << std::endl;
            return 0;
        }
    }

    std::cout << "Sorted " << ARR_SIZE << " elements using std::sort in " << std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1).count() << "ms" << std::endl;
    std::cout << "Sorted " << ARR_SIZE << " elements using timsort::tim_sort in " << std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2).count() << "ms" << std::endl;
}