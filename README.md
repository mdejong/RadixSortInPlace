Optimized implementation of in-place Radix sort.

A rough draft was first implemented in Python and this code was then converted to C++. While many examples of LSD Radix Sort can be found easily, a LSD Radix sort that depends on copying values into a second full size array is not effective for large data sets. This in-place MSD Radix sort operates on 32 bit unsigned integers and depends on only minimal stack space for a count/offset table.

See in_place_sort_opt.hpp for C++ source code of the optimized approach based on a hybrid of American flag sort and the iteration approach from SkaSort. The file in_place_sort.hpp contains an American flag sort implementation for comparison purposes.

Performance:

The Xcode test file RadixSortTests contains a series of tests of interest. The following performance examples (be sure to run in Release mode) show performance on a huge pow(2,28) size random number buffer.

testCSIPPerformanceExampleD3    average: 8.9 s (American Flag)

testSkaSortPerformanceExample   average: 8.3 s (SkaSort)

testCSIPPerformanceExampleD3Opt average: 6.3 s (Hybrid)

Based on:

https://duvanenko.tech.blog/2022/04/10/in-place-n-bit-radix-sort/

"Parallel In-Place Radix Sort Simplified" : From drdobbs.com via wayback machine.

Related:

https://github.com/albicilla/simple_paradis

https://github.com/fenilgmehta/Fastest-Integer-Sort

C++ Now 2017: M. Skarupke “Sorting in less than O(n log n): Generalizing and optimizing radix sort"

https://www.youtube.com/watch?v=zqs87a_7zxw&t=1080s

https://probablydance.com/2016/12/27/i-wrote-a-faster-sorting-algorithm/

https://publications.scss.tcd.ie/theses/diss/2024/TCD-SCSS-DISSERTATION-2024-012.pdf

