//
//  RadixSortTests.m
//  RadixSortTests
//
//  Created by Moses DeJong on 9/21/25.
//

#import <XCTest/XCTest.h>

#include <random>
#include <cstddef>  // For std::ptrdiff_t

#include "in_place_sort.hpp"

#if defined(DEBUG)
# define PERFORMANCE_NUM_LOOPS_TEST 5000
#else

# if TARGET_OS_IPHONE
  // iOS, tvOS, or watchOS device
# define PERFORMANCE_NUM_LOOPS_TEST 50000
# else
# define PERFORMANCE_NUM_LOOPS_TEST 500000
# endif

#endif

// For large tests like pow(2,14)
#if defined(DEBUG)
# define PERFORMANCE_BIG_N_NUM_LOOPS_TEST 100
#else
# define PERFORMANCE_BIG_N_NUM_LOOPS_TEST 1000
#endif

// For very large tests like pow(2,20)
#if defined(DEBUG)
# define PERFORMANCE_VERY_BIG_N_NUM_LOOPS_TEST 10
#else
# define PERFORMANCE_VERY_BIG_N_NUM_LOOPS_TEST 100
#endif


@interface RadixSortTests : XCTestCase

@end

static
__attribute__((noinline))
void setupRandomPixelValues(std::vector<uint32_t> & inputValues, uint32_t maxNum) {
  const unsigned int nSrcValues = (unsigned int) inputValues.size();
  const uint32_t rmax = maxNum;
  
  const unsigned int range_from  = 0;
  const unsigned int range_to    = rmax;
  std::random_device                  rand_dev;
  std::mt19937                        generator(rand_dev());
  std::uniform_int_distribution<uint32_t>  distr(range_from, range_to);
    
  for ( int i = 0 ; i < nSrcValues; i++ ) {
    uint32_t v = distr(generator);
#if defined(DEBUG)
      assert(v <= rmax);
#endif // DEBUG
    inputValues[i] = v;
  }
    
  assert(inputValues.size() == nSrcValues);
}

@implementation RadixSortTests

- (void)testCSIP_two_digit_1 {
  constexpr unsigned int N = 5;
  std::vector<uint32_t> inWords{
    0xFF, 0xFF-1, 257, 256, 0xFF-2
  };
  
  uint32_t arr[N];
  memcpy(arr, inWords.data(), N * sizeof(uint32_t));
  
  auto printArray = [](uint32_t *arr, int n, char *msg) {
    int i;
    printf("%s\n",msg);
    printf("[%d",arr[0]);
    for (i=1; i < n; i++) {
      printf(",%d", arr[i]);
    }
    printf("]\n");
  };
  
  printArray(arr, N, (char*)"unsorted");
  
  countingSortInPlace<1>(arr, 0, N);
  
  printArray(arr, N, (char*)"sorted");
  
  XCTAssert(arr[0] == 253);
  XCTAssert(arr[1] == 254);
  XCTAssert(arr[2] == 255);
  XCTAssert(arr[3] == 256);
  XCTAssert(arr[4] == 257);
}

//constexpr unsigned int PERF_N = 100000; // 100 thousand numbers
//constexpr unsigned int PERF_N = 100000000; // 100 million numbers

// At 4Gb x 2, a test can take 20 minutes to finish
//constexpr unsigned int PERF_N =   1073741824; // 2*30 is very very large (4 Gb x 2)

constexpr unsigned int PERF_N =   1073741824 / 4; // (2*30)/4 is very very large (1 Gb x 2)

// Very large size tests will never finish unless this is 1
#undef PERFORMANCE_VERY_BIG_N_NUM_LOOPS_TEST
#define PERFORMANCE_VERY_BIG_N_NUM_LOOPS_TEST 1

- (void)testCSIPPerformanceExampleD0 {
  constexpr unsigned int N = PERF_N;
  
  auto sharedRandomWords = std::make_shared<std::vector<uint32_t>>(N);
  std::vector<uint32_t> & randomWordsVec = *sharedRandomWords;
  
  // Generating the random numbers seems to take up the vast majority of runtime at large sizes
  // Use u32 max so that randomWords are highly spread over whole int range
  //constexpr unsigned int maxU32 = (uint32_t)-1;
  constexpr unsigned int maxU32 = 256-1;
  setupRandomPixelValues(randomWordsVec, maxU32);
    
  auto sharedDstVec = std::make_shared<std::vector<uint32_t>>(N);
  std::vector<uint32_t> & dstVec = *sharedDstVec;
  uint32_t *outOrigArr = dstVec.data();
  memset(outOrigArr, 0, N * sizeof(uint32_t));
  
  [self measureBlock:^{
    for (int i = 0; i < PERFORMANCE_VERY_BIG_N_NUM_LOOPS_TEST; i++) {
      std::vector<uint32_t> & randomWords = *sharedRandomWords;
      uint32_t *inPtr = randomWords.data();
      std::vector<uint32_t> & dstVec = *sharedDstVec;
      uint32_t *outPtr = dstVec.data();
      
      memcpy(outPtr, inPtr, N * sizeof(uint32_t));
      
      countingSortInPlace<0>(outPtr, 0, N);
      
#if defined(DEBUG)
      {
        std::vector<uint32_t> expected;
        {
          std::vector<uint32_t> stdSorted = randomWords;
          std::sort(begin(stdSorted), end(stdSorted));
          expected = stdSorted;
        }
        bool passed = true;
        for (int exi = 0; exi < expected.size(); exi++) {
          if (expected[exi] != outPtr[exi]) {
            XCTAssert(false, "%d != %d : at exi %d", expected[exi], outPtr[exi], exi);
            passed = false;
            break;
          }
        }
        if (!passed) {
          break;
        }
      }
#endif // DEBUG
    }
  }];
    
}


- (void)testCSIPPerformanceExampleD1 {
  constexpr unsigned int N = PERF_N;

  auto sharedRandomWords = std::make_shared<std::vector<uint32_t>>(N);
  std::vector<uint32_t> & randomWordsVec = *sharedRandomWords;
  
  // Generating the random numbers seems to take up the vast majority of runtime at large sizes
  // Use u32 max so that randomWords are highly spread over whole int range
  //constexpr unsigned int maxU32 = (uint32_t)-1;
  constexpr unsigned int maxU32 = 0xFFFF;
  setupRandomPixelValues(randomWordsVec, maxU32);
    
  auto sharedDstVec = std::make_shared<std::vector<uint32_t>>(N);
  std::vector<uint32_t> & dstVec = *sharedDstVec;
  uint32_t *outOrigArr = dstVec.data();
  memset(outOrigArr, 0, N * sizeof(uint32_t));
  
  [self measureBlock:^{
    for (int i = 0; i < PERFORMANCE_VERY_BIG_N_NUM_LOOPS_TEST; i++) {
      std::vector<uint32_t> & randomWords = *sharedRandomWords;
      uint32_t *inPtr = randomWords.data();
      std::vector<uint32_t> & dstVec = *sharedDstVec;
      uint32_t *outPtr = dstVec.data();
      
      memcpy(outPtr, inPtr, N * sizeof(uint32_t));
      
      countingSortInPlace<1>(outPtr, 0, N);
      
#if defined(DEBUG)
      {
        std::vector<uint32_t> expected;
        {
          std::vector<uint32_t> stdSorted = randomWords;
          std::sort(begin(stdSorted), end(stdSorted));
          expected = stdSorted;
        }
        bool passed = true;
        for (int exi = 0; exi < expected.size(); exi++) {
          if (expected[exi] != outPtr[exi]) {
            XCTAssert(false, "%d != %d : at exi %d", expected[exi], outPtr[exi], exi);
            passed = false;
            break;
          }
        }
        if (!passed) {
          break;
        }
      }
#endif // DEBUG
    }
  }];
    
}

- (void)testCSIPPerformanceExampleD2 {
  constexpr unsigned int N = PERF_N;

  auto sharedRandomWords = std::make_shared<std::vector<uint32_t>>(N);
  std::vector<uint32_t> & randomWordsVec = *sharedRandomWords;
  
  // Generating the random numbers seems to take up the vast majority of runtime at large sizes
  // Use u32 max so that randomWords are highly spread over whole int range
  //constexpr unsigned int maxU32 = (uint32_t)-1;
  constexpr unsigned int maxU32 = 0xFFFFFF;
  setupRandomPixelValues(randomWordsVec, maxU32);
  
  auto sharedDstVec = std::make_shared<std::vector<uint32_t>>(N);
  std::vector<uint32_t> & dstVec = *sharedDstVec;
  uint32_t *outOrigArr = dstVec.data();
  memset(outOrigArr, 0, N * sizeof(uint32_t));
  
  [self measureBlock:^{
    for (int i = 0; i < PERFORMANCE_VERY_BIG_N_NUM_LOOPS_TEST; i++) {
      std::vector<uint32_t> & randomWords = *sharedRandomWords;
      uint32_t *inPtr = randomWords.data();
      std::vector<uint32_t> & dstVec = *sharedDstVec;
      uint32_t *outPtr = dstVec.data();
      
      memcpy(outPtr, inPtr, N * sizeof(uint32_t));
      
      countingSortInPlace<2>(outPtr, 0, N);
      
#if defined(DEBUG)
      {
        std::vector<uint32_t> expected;
        {
          std::vector<uint32_t> stdSorted = randomWords;
          std::sort(begin(stdSorted), end(stdSorted));
          expected = stdSorted;
        }
        bool passed = true;
        for (int exi = 0; exi < expected.size(); exi++) {
          if (expected[exi] != outPtr[exi]) {
            XCTAssert(false, "%d != %d : at exi %d", expected[exi], outPtr[exi], exi);
            passed = false;
            break;
          }
        }
        if (!passed) {
          break;
        }
      }
#endif // DEBUG
    }
  }];
    
}

- (void)testCSIPPerformanceExampleD3 {
  constexpr unsigned int N = PERF_N;

  auto sharedRandomWords = std::make_shared<std::vector<uint32_t>>(N);
  std::vector<uint32_t> & randomWordsVec = *sharedRandomWords;
  
  // Generating the random numbers seems to take up the vast majority of runtime at large sizes
  // Use u32 max so that randomWords are highly spread over whole int range
  //constexpr unsigned int maxU32 = (uint32_t)-1;
  constexpr unsigned int maxU32 = 0xFFFFFFFF;
  setupRandomPixelValues(randomWordsVec, maxU32);
    
  auto sharedDstVec = std::make_shared<std::vector<uint32_t>>(N);
  std::vector<uint32_t> & dstVec = *sharedDstVec;
  uint32_t *outOrigArr = dstVec.data();
  memset(outOrigArr, 0, N * sizeof(uint32_t));
  
  [self measureBlock:^{
    for (int i = 0; i < PERFORMANCE_VERY_BIG_N_NUM_LOOPS_TEST; i++) {
      std::vector<uint32_t> & randomWords = *sharedRandomWords;
      uint32_t *inPtr = randomWords.data();
      std::vector<uint32_t> & dstVec = *sharedDstVec;
      uint32_t *outPtr = dstVec.data();
      
      memcpy(outPtr, inPtr, N * sizeof(uint32_t));
      
      countingSortInPlace<3>(outPtr, 0, N);
      
#if defined(DEBUG)
      {
        std::vector<uint32_t> expected;
        {
          std::vector<uint32_t> stdSorted = randomWords;
          std::sort(begin(stdSorted), end(stdSorted));
          expected = stdSorted;
        }
        bool passed = true;
        for (int exi = 0; exi < expected.size(); exi++) {
          if (expected[exi] != outPtr[exi]) {
            XCTAssert(false, "%d != %d : at exi %d", expected[exi], outPtr[exi], exi);
            passed = false;
            break;
          }
        }
        if (!passed) {
          break;
        }
      }
#endif // DEBUG
    }
  }];
    
}

- (void)testStdSortExample {
  constexpr unsigned int N = PERF_N;
  
  auto sharedRandomWords = std::make_shared<std::vector<uint32_t>>(N);
  std::vector<uint32_t> & randomWordsVec = *sharedRandomWords;
  
  // Generating the random numbers seems to take up the vast majority of runtime at large sizes
  // Use u32 max so that randomWords are highly spread over whole int range
  //constexpr unsigned int maxU32 = (uint32_t)-1;
  constexpr unsigned int maxU32 = 0xFFFFFFFF;
  setupRandomPixelValues(randomWordsVec, maxU32);
    
  auto sharedDstVec = std::make_shared<std::vector<uint32_t>>(N);
  std::vector<uint32_t> & dstVec = *sharedDstVec;
  uint32_t *outOrigArr = dstVec.data();
  memset(outOrigArr, 0, N * sizeof(uint32_t));
  
  [self measureBlock:^{
    for (int i = 0; i < PERFORMANCE_VERY_BIG_N_NUM_LOOPS_TEST; i++) {
      std::vector<uint32_t> & randomWords = *sharedRandomWords;
      uint32_t *inPtr = randomWords.data();
      std::vector<uint32_t> & dstVec = *sharedDstVec;
      uint32_t *outPtr = dstVec.data();
      
      memcpy(outPtr, inPtr, N * sizeof(uint32_t));
      
      std::sort(begin(dstVec), end(dstVec));
      
#if defined(DEBUG)
      {
        std::vector<uint32_t> expected;
        {
          std::vector<uint32_t> stdSorted = randomWords;
          std::sort(begin(stdSorted), end(stdSorted));
          expected = stdSorted;
        }
        bool passed = true;
        for (int exi = 0; exi < expected.size(); exi++) {
          if (expected[exi] != outPtr[exi]) {
            XCTAssert(false, "%d != %d : at exi %d", expected[exi], outPtr[exi], exi);
            passed = false;
            break;
          }
        }
        if (!passed) {
          break;
        }
      }
#endif // DEBUG
    }
  }];
    
}


@end
