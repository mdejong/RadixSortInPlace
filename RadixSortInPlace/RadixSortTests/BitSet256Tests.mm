//
//  BitSet256Tests.m
//
// BitSet supporting fast setting and checking of a bit flag in range 0-255.

#import <XCTest/XCTest.h>

#include <random>
#include <cstddef>  // For std::ptrdiff_t

#include <iostream>

#include "bit_set_256.hpp"

@interface BitSet256Tests : XCTestCase

@end

@implementation BitSet256Tests

// bitset basics

- (void)testBitsetBasics {
  bitset256_t bits;
  unsigned int bucketi;
  
  bitset256Clear(bits);
  
  bitset256SetBit(bits, 0);
  bucketi = bitset256FindFirstSetOpt(bits);
  XCTAssert(bucketi == 0);
  bitset256ClearBit(bits, 0);
  
  bitset256SetBit(bits, 1);
  bucketi = bitset256FindFirstSetOpt(bits);
  XCTAssert(bucketi == 1);
  bitset256ClearBit(bits, 1);
  
  bitset256SetBit(bits, 64+1);
  bitset256SetBit(bits, 63);
  bucketi = bitset256FindFirstSetOpt(bits);
  XCTAssert(bucketi == 63);
  
  bitset256Clear(bits);
  bitset256SetBit(bits, 64);
  bucketi = bitset256FindFirstSetOpt(bits);
  XCTAssert(bucketi == 64);
  
  bitset256Clear(bits);
  bitset256SetBit(bits, 64);
  bucketi = bitset256FindFirstSetOpt(bits);
  XCTAssert(bucketi == 64);
  
  for (int i = 0; i < 256; i++) {
    bitset256Clear(bits);
    bitset256SetBit(bits, i);
    bucketi = bitset256FindFirstSetOpt(bits);
    XCTAssert(bucketi == i);
  }
  
  for (int i = 256-64; i < (256-1); i++) {
    bitset256Clear(bits);
    bitset256SetBit(bits, i);
    bitset256SetBit(bits, 255);
    bucketi = bitset256FindFirstSetOpt(bits);
    XCTAssert(bucketi == i);
  }
}

//constexpr unsigned int PERF_N = 100;

//constexpr unsigned int PERF_N = 100000; // 100 thousand numbers
constexpr unsigned int PERF_N = 250000; // 250 thousand numbers
//constexpr unsigned int PERF_N = 100000000; // 100 million numbers

// At 4Gb x 2, a test can take 20 minutes to finish
//constexpr unsigned int PERF_N =   1073741824; // 2*30 is very very large (4 Gb x 2)

//constexpr unsigned int PERF_N =   1073741824 / 4; // (2*30)/4 is very very large (1 Gb x 2)

// Very large size tests will never finish unless this is 1
//#undef PERFORMANCE_VERY_BIG_N_NUM_LOOPS_TEST
//#define PERFORMANCE_VERY_BIG_N_NUM_LOOPS_TEST 1

#undef PERFORMANCE_VERY_BIG_N_NUM_LOOPS_TEST
#define PERFORMANCE_VERY_BIG_N_NUM_LOOPS_TEST 10000

static
__attribute__((noinline))
void setupRandomPixelValues(std::vector<uint32_t> & inputValues, uint32_t maxNum) {
  const unsigned int nSrcValues = (unsigned int) inputValues.size();
  const uint32_t rmax = maxNum;
  
  const unsigned int range_from  = 0;
  const unsigned int range_to    = rmax;
  std::random_device                  rand_dev;
  std::mt19937                        generator(rand_dev());
  
  std::uniform_int_distribution<uint32_t>  distr(range_from, range_to); // even dist between buckets
  
  //std::geometric_distribution<> distr; // heavily skew to first 3 or 4 buckets
  
  for ( int i = 0 ; i < nSrcValues; i++ ) {
    uint32_t v = distr(generator);
#if defined(DEBUG)
      assert(v <= rmax);
#endif // DEBUG
    inputValues[i] = v;
  }
    
  assert(inputValues.size() == nSrcValues);
}

- (void)testBitSetOptExample1 {
  auto bucketi = 216;
  
  bitset256_t bits;
  bitset256Clear(bits);

  bitset256SetBit(bits, bucketi);
  
  auto foundi = bitset256FindFirstSetOpt(bits);
  
  XCTAssert(foundi == bucketi, "%d != %d", bucketi, foundi);
}

- (void)testBitSetOptExample2 {
  bitset256_t bits;
  bitset256Clear(bits);

  {
    // q1
    auto bucketi = 64;
    bitset256SetBit(bits, bucketi);
    
    auto foundi = bitset256FindFirstSetOpt(bits);
    
    XCTAssert(foundi == bucketi, "%d != %d", bucketi, foundi);
  }
  
  {
    // q0
    auto bucketi = 63;
    bitset256SetBit(bits, bucketi);
    
    auto foundi = bitset256FindFirstSetOpt(bits);
    
    XCTAssert(foundi == bucketi, "%d != %d", bucketi, foundi);
  }

  {
    // q0
    auto bucketi = 1;
    bitset256SetBit(bits, bucketi);
    
    auto foundi = bitset256FindFirstSetOpt(bits);
    
    XCTAssert(foundi == bucketi, "%d != %d", bucketi, foundi);
    
    bitset256ClearBit(bits, bucketi);
  }
  
  bitset256ClearBit(bits, 63);
  
  {
    // q1
    auto bucketi = 64;
    
    auto foundi = bitset256FindFirstSetOpt(bits);
    
    XCTAssert(foundi == bucketi, "%d != %d", bucketi, foundi);
    
    bitset256ClearBit(bits, bucketi);
    
    XCTAssert(bits.bits[4] == 4); // all bits off
  }
}


// Optimized clear logic

- (void)testBitSetClearOpt1 {
  bitset256_t bits;
  bitset256Clear(bits);

  {
    // q0
    auto bucketi = 63;
    bitset256SetBit(bits, bucketi);
    
    auto foundi = bitset256FindFirstSetOpt(bits);
    
    XCTAssert(foundi == bucketi, "%d != %d", bucketi, foundi);
  }
  
  {
    // q1
    auto bucketi = 64;
    bitset256SetBit(bits, bucketi);
    
    auto foundi = bitset256FindFirstSetOpt(bits);
    
    XCTAssert(foundi == 63);
    XCTAssert(bits.bits[5-1] == 0);
    
    bitset256ClearBit(bits, 63);
    
    foundi = bitset256FindFirstSetOpt(bits);
    
    XCTAssert(foundi == bucketi, "%d != %d", bucketi, foundi);
    XCTAssert(bits.bits[5-1] == 1);
  }
}

- (void)testBitSetClearOpt2 {
  bitset256_t bits;
  bitset256Clear(bits);

  {
    // q1
    auto bucketi = 64;
    bitset256SetBit(bits, bucketi);
    
    auto foundi = bitset256FindFirstSetOpt(bits);
    
    XCTAssert(foundi == bucketi, "%d != %d", bucketi, foundi);
  }
  
  {
    // q2
    auto bucketi = 64+64+1;
    bitset256SetBit(bits, bucketi);
    
    auto foundi = bitset256FindFirstSetOpt(bits);
    
    XCTAssert(foundi == 64);
    XCTAssert(bits.bits[5-1] == 1);
    
    bitset256ClearBit(bits, 64);
    
    foundi = bitset256FindFirstSetOpt(bits);
    
    XCTAssert(foundi == bucketi, "%d != %d", bucketi, foundi);
    XCTAssert(bits.bits[5-1] == 2);
  }
}

- (void)testBitSetClearOpt3 {
  bitset256_t bits;
  bitset256Clear(bits);

  {
    // q0
    auto bucketi = 1;
    bitset256SetBit(bits, bucketi);
    
    auto foundi = bitset256FindFirstSetOpt(bits);
    
    XCTAssert(foundi == bucketi, "%d != %d", bucketi, foundi);
  }
  
  {
    // q3
    auto bucketi = 255;
    bitset256SetBit(bits, bucketi);
    
    auto foundi = bitset256FindFirstSetOpt(bits);
    
    XCTAssert(foundi == 1);
    XCTAssert(bits.bits[5-1] == 0);
    
    bitset256ClearBit(bits, 1);
    
    foundi = bitset256FindFirstSetOpt(bits);
    
    XCTAssert(foundi == bucketi, "%d != %d", bucketi, foundi);
    XCTAssert(bits.bits[5-1] == 3);
  }
}

- (void)testBitSetClearOpt4 {
  bitset256_t bits;
  bitset256Clear(bits);

  {
    // q0
    auto bucketi = 1;
    bitset256SetBit(bits, bucketi);
    
    auto foundi = bitset256FindFirstSetOpt(bits);
    
    XCTAssert(foundi == bucketi, "%d != %d", bucketi, foundi);
    
    bitset256ClearBit(bits, bucketi);
    
    XCTAssert(bits.bits[5-1] == (5-1));
  }
}

- (void)testBitSetCopy1 {
  bitset256_t bits;
  bitset256Clear(bits);

  bitset256_t copy;
  
  {
    // q0
    auto bucketi = 255;
    bitset256SetBit(bits, bucketi);
    
    auto foundi = bitset256FindFirstSetOpt(bits);
    
    XCTAssert(foundi == bucketi, "%d != %d", bucketi, foundi);
  }
  
  {
    // q0
    auto bucketi = 255;
    bitset256CopyBits(bits, copy);
    
    auto foundi = bitset256FindFirstSetOpt(bits);
    
    XCTAssert(foundi == bucketi, "%d != %d", bucketi, foundi);
  }
}

- (void)testBitPopCount1 {
  bitset256_t bits;
  bitset256Clear(bits);

  auto N = bitset256PopCount(bits);
  XCTAssert(N == 0);

  bitset256SetBit(bits, 0);
  
  N = bitset256PopCount(bits);
  XCTAssert(N == 1);
  
  bitset256SetBit(bits, 255);
  
  N = bitset256PopCount(bits);
  XCTAssert(N == 2);
  
  {
    bool isOn;
    
    isOn = bitset256GetBit(bits, 0);
    XCTAssert(isOn == true);
    
    isOn = bitset256GetBit(bits, 1);
    XCTAssert(isOn == false);
    
    isOn = bitset256GetBit(bits, 255);
    XCTAssert(isOn == true);
  }
}

// Setup a bitset 256 and then init some slots and run LSD detection logic.

- (void)testBitSetPerformanceExample {
  constexpr unsigned int N = 30;
  
  auto sharedRandomWords = std::make_shared<std::vector<uint32_t>>(N);
  std::vector<uint32_t> & randomWordsVec = *sharedRandomWords;
  
  // Generate 10 random numbers in the range 0 -> 255
  constexpr unsigned int maxU32 = 256-1;
  setupRandomPixelValues(randomWordsVec, maxU32);
    
  auto sharedDstVec = std::make_shared<std::vector<uint32_t>>(N);
  std::vector<uint32_t> & dstVec = *sharedDstVec;
  uint32_t *outOrigArr = dstVec.data();
  memset(outOrigArr, 0, N * sizeof(uint32_t));
  
  [self measureBlock:^{
    bitset256_t bits;
    bitset256Clear(bits);
    
    for (int i = 0; i < PERFORMANCE_VERY_BIG_N_NUM_LOOPS_TEST; i++) {
      std::vector<uint32_t> & randomWords = *sharedRandomWords;
      uint32_t *inPtr = randomWords.data();
      std::vector<uint32_t> & dstVec = *sharedDstVec;
      uint32_t *outPtr = dstVec.data();
      
      memcpy(outPtr, inPtr, N * sizeof(uint32_t));
      
      // Set next random bit
      
      for (int j = 0; j < PERFORMANCE_VERY_BIG_N_NUM_LOOPS_TEST; j++) {
        auto bucketi = inPtr[j % N];
        
        //std::cout << "bucketi " << bucketi << std::endl;
        
        bitset256SetBit(bits, bucketi);
        
        auto foundi = bitset256FindFirstSetOpt(bits);
        
        bitset256ClearBit(bits, bucketi);
        
#if defined(DEBUG)
      {
        bool same = bucketi == foundi;
        
        if (!same) {
          XCTAssert(same, "%d != %d : for bit %d", bucketi, foundi, i);
          break;
        }
      }
#endif // DEBUG
      }
    }
  }];
}

- (void)testBitSetPerformanceOptExample {
  constexpr unsigned int N = 30;
  
  auto sharedRandomWords = std::make_shared<std::vector<uint32_t>>(N);
  std::vector<uint32_t> & randomWordsVec = *sharedRandomWords;
  
  // Generate 10 random numbers in the range 0 -> 255
  constexpr unsigned int maxU32 = 256-1;
  setupRandomPixelValues(randomWordsVec, maxU32);
    
  auto sharedDstVec = std::make_shared<std::vector<uint32_t>>(N);
  std::vector<uint32_t> & dstVec = *sharedDstVec;
  uint32_t *outOrigArr = dstVec.data();
  memset(outOrigArr, 0, N * sizeof(uint32_t));
  
  [self measureBlock:^{
    bitset256_t bits;
    bitset256Clear(bits);
    
    for (int i = 0; i < PERFORMANCE_VERY_BIG_N_NUM_LOOPS_TEST; i++) {
      std::vector<uint32_t> & randomWords = *sharedRandomWords;
      uint32_t *inPtr = randomWords.data();
      std::vector<uint32_t> & dstVec = *sharedDstVec;
      uint32_t *outPtr = dstVec.data();
      
      memcpy(outPtr, inPtr, N * sizeof(uint32_t));
      
      // Set next random bit
      
      for (int j = 0; j < PERFORMANCE_VERY_BIG_N_NUM_LOOPS_TEST; j++) {
        auto bucketi = inPtr[j % N];
        
        //std::cout << "bucketi " << bucketi << std::endl;
        
        bitset256SetBit(bits, bucketi);
        
        auto foundi = bitset256FindFirstSetOpt(bits);
        
        bitset256ClearBit(bits, bucketi);
        
#if defined(DEBUG)
      {
        bool same = bucketi == foundi;
        
        if (!same) {
          XCTAssert(0, "%d != %d : for bit %d", bucketi, foundi, j);
          break;
        }
      }
#endif // DEBUG
      }
    }
  }];
}

@end
