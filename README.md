Optimized implementation of in-place Radix sort.

A rough draft was first implemented in Python and this code was then converted to C++. While many examples of LSD Radix Sort can be found easily, a LSD Radix sort that depends on copying values into a second full size array is not effective for large data sets. An in-place MSD Radix sort requires only minimal memory for count/offset table.

Based on:

https://duvanenko.tech.blog/2022/04/10/in-place-n-bit-radix-sort/

"Parallel In-Place Radix Sort Simplified" : From drdobbs.com via wayback machine.

Related:

https://github.com/albicilla/simple_paradis

https://github.com/fenilgmehta/Fastest-Integer-Sort

C++ Now 2017: M. Skarupke “Sorting in less than O(n log n): Generalizing and optimizing radix sort"

https://www.youtube.com/watch?v=zqs87a_7zxw&t=1080s

https://probablydance.com/2016/12/27/i-wrote-a-faster-sorting-algorithm/

