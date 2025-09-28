#include <iostream>
#include <cstdint>

#if defined(DEBUG)
#include <assert.h>
#endif

//typedef struct {
//  uint32_t count;
//  uint32_t offset;
//} CountOff;

// Given a 32 bit integer, extract a specific digit.
//
// uint32_t digit = extractDigit<0>(v, digitOffset);

template <unsigned int D>
static inline
unsigned int extractDigitOpt(uint32_t v) {
  if constexpr (D == 3) {
    return v >> (3 * 8);
  } else if constexpr (D == 2) {
    return (v >> (2 * 8)) & 0xFF;
  } else if constexpr (D == 1) {
    return (v >> (1 * 8)) & 0xFF;
  } else if constexpr (D == 0) {
    return v & 0xFF;
  }
}

// Extract histogram logic into util method so profiling visibility.
// Note that bucketi writes back into caller stack because of
// special case of all values in same bucket.

template <unsigned int D, unsigned int M>
static inline
void histogramOpt(
                  uint32_t * arr,
                  unsigned int starti,
                  unsigned int endi,
                  unsigned int & bucketi,
                  uint32_t * table1,
                  uint32_t * table2
                  )
{
//#define UNROLL_HISTOGRAMS2
//#define UNROLL_HISTOGRAMS4

  constexpr unsigned int bucketMax = M;
  
#if !defined(UNROLL_HISTOGRAMS2) && !defined(UNROLL_HISTOGRAMS4)
  for (auto readi = starti; readi < endi; readi++) {
    auto readVal = arr[readi];
    bucketi = extractDigit<D>(readVal);
#if defined(DEBUG)
    assert(bucketi < bucketMax);
#endif
    ++table1[bucketi];
  }
#elif defined(UNROLL_HISTOGRAMS2)
  constexpr size_t unroll_count = 2;
  unsigned int unrolledLoops = (endi - starti) / unroll_count;
  unsigned int unrolledEnd = unrolledLoops * unroll_count;
  
  unsigned int readi = starti;
  for (; readi < unrolledEnd; readi += unroll_count) {
    unsigned int bucketi0 = extractDigit<D>(arr[readi+0]);
    unsigned int bucketi1 = extractDigit<D>(arr[readi+1]);

#if defined(DEBUG)
    assert(bucketi0 < bucketMax);
    assert(bucketi1 < bucketMax);
#endif
    
    ++table1[bucketi0];
    ++table2[bucketi1];
  }
  for (; readi < endi; readi++) {
    bucketi = extractDigit<D>(arr[readi]);
#if defined(DEBUG)
    assert(bucketi < bucketMax);
#endif
    ++table1[bucketi];
  }
  
  for (unsigned int bucketi = 0; bucketi < bucketMax; bucketi++) {
    table1[bucketi] = table1[bucketi] + table2[bucketi];
  }

  if (bucketi == bucketMax) {
    // Wacky case of no cleanup loops, grab last bucketi explicitly
    readi -= 1;
    auto readVal = arr[readi];
    bucketi = extractDigit<D>(readVal);
#if defined(DEBUG)
    assert(bucketi < bucketMax);
#endif
  }
#else // UNROLL_HISTOGRAMS4
  uint32_t table3[bucketMax] = {};
  uint32_t table4[bucketMax] = {};
  
  constexpr size_t unroll_count = 4;
  unsigned int unrolledLoops = (endi - starti) / unroll_count;
  unsigned int unrolledEnd = unrolledLoops * unroll_count;
  
  unsigned int readi = starti;
  for (; readi < unrolledEnd; readi += unroll_count) {
    unsigned int bucketi0 = extractDigit<D>(arr[readi+0]);
    unsigned int bucketi1 = extractDigit<D>(arr[readi+1]);
    unsigned int bucketi2 = extractDigit<D>(arr[readi+2]);
    unsigned int bucketi3 = extractDigit<D>(arr[readi+3]);

#if defined(DEBUG)
    assert(bucketi0 < bucketMax);
    assert(bucketi1 < bucketMax);
    assert(bucketi2 < bucketMax);
    assert(bucketi3 < bucketMax);
#endif
    
    ++table1[bucketi0];
    ++table2[bucketi1];
    ++table3[bucketi2];
    ++table4[bucketi3];
  }
  for (; readi < endi; readi++) {
    bucketi = extractDigit<D>(arr[readi]);
#if defined(DEBUG)
    assert(bucketi < bucketMax);
#endif
    ++table1[bucketi];
  }
  
  for (unsigned int bucketi = 0; bucketi < bucketMax; bucketi++) {
    table1[bucketi] = table1[bucketi] + table2[bucketi] + table3[bucketi] + table4[bucketi];
  }

  if (bucketi == bucketMax) {
    // Wacky case of no cleanup loops, grab last bucketi explicitly
    readi -= 1;
    auto readVal = arr[readi];
    bucketi = extractDigit<D>(readVal);
#if defined(DEBUG)
    assert(bucketi < bucketMax);
#endif
  }
#endif // UNROLL_HISTOGRAMS4
}
// D is digit 3,2,1,0 for 32 bits

template <unsigned int D>
__attribute__((noinline))
void countingSortInPlaceOpt(
  uint32_t * arr,
  unsigned int starti,
  unsigned int endi)
{
  constexpr bool debugOut = false;
  constexpr bool debugDumpInOutValues = false;
  constexpr bool debugDumpHistogram = false;
  constexpr bool debugDumpPrefixSum = false;
  int n = endi - starti;
    
  // if (n < 2) {
  //   std::cout << "countingSortInPlace early return from recursion " << starti << " up to " << endi << std::endl;
  //   std::cout << "n " << n << std::endl;
  //   return;
  // }
  
  auto recurse = [](
                    uint32_t *arr,
                    unsigned int starti,
                    unsigned int endi
                    )
  {
    if constexpr (D > 0) {
      unsigned int n = endi - starti;
      switch (n) {
        case 1: {
          // nop
          break;
        }
        case 2: {
          // Trivial in-place swap if needed
          auto v0 = arr[starti];
          auto v1 = arr[starti+1];
          if (v0 > v1) {
            std::swap(v0, v1);
          }
          arr[starti] = v0;
          arr[starti+1] = v1;
          break;
        }
        case 3 ... 128: {
          // Small bucket subrange can be sorted without recursion
          std::sort(arr+starti, arr+endi);
          break;
        }
        default: {
          countingSortInPlace<D-1>(arr, starti, endi);
          break;
        }
      }
    }
  };
	
  if (debugOut) {
    std::cout << "countingSortInPlace D = " << D << " input:" << std::endl;
    std::cout << "n " << n << std::endl;
    std::cout << "starti " << starti << std::endl;
    std::cout << "endi " << endi << std::endl;

    if (debugDumpInOutValues) {
      for (int i = starti; i < endi; i++) {
        std::cout << (unsigned int) arr[i] << std::endl;
      }
    }
    std::cout << "-------- " << std::endl;
  }
    
  constexpr unsigned int bucketMax = 256;

  // counts and offsets can both be used for bucket counts and then start/end offsets.
  // Init bothj to zero to support multiple uses.

  uint32_t counts[bucketMax] = {};
  uint32_t offsets[bucketMax] = {};
  
  // Histogram counts
  unsigned int bucketi = bucketMax;
  
  histogramOpt<D, bucketMax>(arr, starti, endi, bucketi, counts, offsets);
  
  if (debugDumpHistogram) {
    std::cout << "countingSortInPlace D = " << D << " counts:" << std::endl;
    
    for (unsigned int bucketi = 0; bucketi < bucketMax; bucketi++) {
      auto count = counts[bucketi];
      if (count != 0) {
        std::cout << "[" << bucketi << "] = " << count << std::endl;
      }
    }
    std::cout << "-------- " << std::endl;
  }
  
  // Special case where all values map to the same bucket.
  // The count logic needs to be done, but prefix sum and
  // looping over all the values to calculate buckets will
  // result in no reordering.
  
  if (bucketi != bucketMax) {
    auto count = counts[bucketi];
    if (count == n) {
      // Special case where all values scan into a single bucket.
      if (debugOut) {
        std::cout << "all " << n << " values in same bucket " << bucketi << std::endl;
      }
      
      recurse(arr, starti, endi);
      
      return;
    }
  }
  
  // Prefix Sum (given starting offset), note that after this loop is completed the
  // counts field is converted to an end of bucket offset.
  
  unsigned int psum = starti;
  for (bucketi = 0; bucketi < bucketMax; bucketi++) {
    auto count = counts[bucketi];
    offsets[bucketi] = psum;
    psum += count;
    counts[bucketi] = psum;
  }
  
  // Create nextBuckets table that will optimize
  // looping over used buckets.
  
  uint8_t nextBuckets[bucketMax] = {}; // init array values to zero

  uint8_t nextNonEmptyBucket = 0;
  
  for (int bucketi = bucketMax - 1; bucketi >= 0; bucketi--) {
    auto count = counts[bucketi] - offsets[bucketi];
    if (count != 0) {
      nextBuckets[bucketi] = nextNonEmptyBucket;
      nextNonEmptyBucket = bucketi;
    }
  }
  
  uint8_t minNonEmptyBucket = nextNonEmptyBucket;

  if (debugDumpPrefixSum) {
    std::cout << "countingSortInPlace D = " << D << " prefix sum:" << std::endl;
    
    for (unsigned int bucketi = 0; bucketi < bucketMax; bucketi++) {
      auto count = counts[bucketi] - offsets[bucketi];
      auto offset = offsets[bucketi];
      if (count != 0) {
        std::cout << "[" << bucketi << "] = " << offset << std::endl;
      }
    }
    std::cout << "-------- " << std::endl;
  }

  if (debugDumpPrefixSum) {
    std::cout << "countingSortInPlace D = " << D << " next buckets:" << std::endl;

    for (unsigned int bucketi = 0; bucketi < bucketMax; bucketi++) {
      auto count = counts[bucketi] - offsets[bucketi];
      if (count != 0) {
        std::cout << "[" << bucketi << "] = " << (unsigned int)nextBuckets[bucketi] << std::endl;
      }
    }
    std::cout << "-------- " << std::endl;
  }
  
  // Setup initial conditions and loop over all values in range
  
#if defined(DEBUG)
  unsigned int slotWrites = 0;
#endif
  
  auto reshuffle = [
#if defined(DEBUG)
    &slotWrites,
#endif
     recurse
  ](
    uint32_t * arr,
    uint32_t readVal,
    uint32_t * counts,
    uint32_t * offsets,
    unsigned int starti,
    unsigned int endi,
    unsigned int & currentBucketStartOffset,
    unsigned int & currentBucketOffset,
    unsigned int & currentBucketi,
    uint8_t * nextBuckets
  ) -> uint32_t {
    unsigned int bucketi = extractDigit<D>(readVal);
    
#if defined(DEBUG)
    assert(bucketi < bucketMax);
#endif
    
    unsigned int writei = offsets[bucketi];
#if defined(DEBUG)
    assert(writei >= starti);
    assert(writei < endi);
#endif
    
    // increment for either branch below

    ++offsets[bucketi];
    
    if (writei == currentBucketOffset) {
      // Cached readVal should be written into the current bucket.
      
      arr[currentBucketOffset] = readVal;
#if defined(DEBUG)
      slotWrites += 1;
#endif
      ++currentBucketOffset;
      
      // If writing this single value finished off current bucket range
      
      auto isBucketEmpty = [](
                              unsigned int bucketi,
                              uint32_t * offsets,
                              uint32_t * counts
                              ) {
        return offsets[bucketi] == counts[bucketi];
      };
      
      if (isBucketEmpty(bucketi, offsets, counts)) {
        // Bucket now empty, skip 0 to N other empty buckets, then partial bucket.
        // Note that current bucket and following empty buckets are recursed into
        // now as opposed to after all values have been processed as that is more
        // cache friendly for small buckets.
        
        if constexpr (D > 0) {
          auto readi = offsets[bucketi];
          
          recurse(arr, currentBucketStartOffset, readi);
          
          currentBucketStartOffset = readi;
        }
        
        unsigned int nextBucketi = nextBuckets[bucketi];
        
        while ((nextBucketi != 0) and isBucketEmpty(nextBucketi, offsets, counts)) {
          if constexpr (D > 0) {
            unsigned int bucketi = nextBucketi;
            
            auto readi = offsets[bucketi];
            
            recurse(arr, currentBucketStartOffset, readi);
            
            currentBucketStartOffset = readi;
          }
          
          nextBucketi = nextBuckets[nextBucketi];
        }
        
        // Post empty bucket loop, skip partial in non-empty bucket
        if (nextBucketi != 0) {
          currentBucketOffset = offsets[nextBucketi];
#if defined(DEBUG)
          assert(currentBucketOffset >= starti);
          assert(currentBucketOffset < endi);
#endif
        } else {
          currentBucketOffset = endi;
        }
        
        currentBucketi = nextBucketi;
      }
      
      // unconditional load, reloads from current slot on final iteration
      readVal = arr[(currentBucketOffset < endi) ? currentBucketOffset : writei];
      
#if defined(DEBUG)
      if (currentBucketOffset < endi) {
        arr[currentBucketOffset] = 0xFFFFFFFF;
      }
#endif
    } else {
      // Write into a different bucket at writei
      
      uint32_t tmp = readVal;
      readVal = arr[writei];
      arr[writei] = tmp;
      
#if defined(DEBUG)
      slotWrites += 1;
#endif
      
#if defined(DEBUG)
      arr[currentBucketOffset] = 0xFFFFFFFF;
#endif
    }
                        
    return readVal;
  }; // end reshuffle()
  
  auto dump = [
               arr,
               starti,
               endi
               ]()
  {
    for ( unsigned int i = starti ; i < endi ; i++ ) {
      if (arr[i] == 0xFFFFFFFF) {
        std::cout << "-";
      } else {
        std::cout << arr[i];
      }
      if (i <= (endi - 1)) {
        std::cout << " ";
      }
    }
    std::cout << std::endl;
  };

  // Setup initial conditions and loop over all values in range
  
  constexpr bool debugDumpIterations = false;
  
  unsigned int currentBucketi = minNonEmptyBucket;
  unsigned int currentBucketStartOffset = offsets[currentBucketi];
  unsigned int currentBucketOffset = currentBucketStartOffset; // offset of "hole" in bucket
  
  uint32_t readVal = arr[currentBucketOffset];
  
  if (debugDumpIterations) {
    dump();
  }
  
  for ( ; currentBucketOffset < endi ; ) {
    if (debugDumpIterations) {
      std::cout << "reshuffle bucket [" << currentBucketi << "] at offset " << currentBucketOffset << "caches " << readVal << std::endl;
    }
    
    readVal = reshuffle(arr, readVal, counts, offsets, starti, endi, currentBucketStartOffset, currentBucketOffset, currentBucketi, nextBuckets);
    
    if (debugDumpIterations) {
      dump();
    }
  }
  
  if (debugOut) {
    std::cout << "countingSortInPlace D = " << D << " returns:" << std::endl;
    std::cout << "n " << n << std::endl;
    std::cout << "starti " << starti << std::endl;
    std::cout << "endi " << endi << std::endl;
    
    if (debugDumpInOutValues) {
      for (int i = starti; i < endi; i++) {
        std::cout << (unsigned int) arr[i] << std::endl;
      }
    }
    
    std::cout << "-------- " << std::endl;
  }
  
#if defined(DEBUG)
  // A bug would cause these two to not match
  assert(n == slotWrites);
#endif
}
