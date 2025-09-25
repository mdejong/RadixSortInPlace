#include <iostream>
#include <cstdint>

#if defined(DEBUG)
#include <assert.h>
#endif

typedef struct {
  uint32_t count;
  uint32_t offset;
} CountOff;

// Given a 32 bit integer, extract a specific digit.
//
// uint32_t digit = extractDigit<0>(v, digitOffset);

template <unsigned int D>
static inline
unsigned int extractDigit(uint32_t v) {
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

// D is digit 3,2,1,0 for 32 bits

template <unsigned int D>
__attribute__((noinline))
void countingSortInPlace(
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
  CountOff CO[bucketMax] = {}; // init array values to zero
  
  // for (unsigned int bucketi = 0; bucketi < bucketMax; bucketi++) {
  //     CountOff z;
  //     z.count = 0;
  //     z.offset = 0;
  //     CO[bucketi] = z;
  // }
  // memset(&CO[0], 0, sizeof(CO));
  // CountOff CO[bucketMax] = { 0 }; // init array values to zero
  
  // Histogram counts
  unsigned int readi;
  unsigned int bucketi = bucketMax;
  uint32_t readVal;
  
  for (readi = starti; readi < endi; readi++) {
    readVal = arr[readi];
    bucketi = extractDigit<D>(readVal);
#if defined(DEBUG)
    assert(bucketi < bucketMax);
#endif
    CountOff co = CO[bucketi];
    co.count += 1;
    CO[bucketi] = co;
  }
  
  if (debugDumpHistogram) {
    std::cout << "countingSortInPlace D = " << D << " counts:" << std::endl;
    
    for (unsigned int bucketi = 0; bucketi < bucketMax; bucketi++) {
      CountOff co = CO[bucketi];
      if (co.count != 0) {
        std::cout << "[" << bucketi << "] = " << co.count << std::endl;
      }
    }
    std::cout << "-------- " << std::endl;
  }
  
  // Special case where all values map to the same bucket.
  // The count logic needs to be done, but prefix sum and
  // looping over all the values to calculate buckets will
  // result in no reordering.
  
  if (bucketi != bucketMax) {
    CountOff co = CO[bucketi];
    if (co.count == n) {
      // Special case where all values scan into a single bucket.
      if (debugOut) {
        std::cout << "all " << n << " values in same bucket " << bucketi << std::endl;
      }
      
      recurse(arr, starti, endi);
      
      return;
    }
  }
  
  // Prefix Sum (given starting offset)
  
  unsigned int psum = starti;
  for (bucketi = 0; bucketi < bucketMax; bucketi++) {
    CountOff co = CO[bucketi];
    co.offset = psum;
    CO[bucketi] = co;
    psum += co.count;
  }
  
  // Create a second "next bucket" table that given
  // a bucketi this finds the next bucket indicated
  // as being not-empty. If a bucket is not used
  // then this table entry is zero, and the final
  // used bucket contains a zero to indicate the end.
  
  uint8_t nextBuckets[bucketMax] = {}; // init array values to zero
  uint8_t nextNonEmptyBucket = 0;
  
  for (int bucketi = bucketMax - 1; bucketi >= 0; bucketi--) {
    CountOff co = CO[bucketi];
    if (co.count != 0) {
      nextBuckets[bucketi] = nextNonEmptyBucket;
      nextNonEmptyBucket = bucketi;
    }
  }

  if (debugDumpPrefixSum) {
    std::cout << "countingSortInPlace D = " << D << " prefix sum:" << std::endl;
    
    for (unsigned int bucketi = 0; bucketi < bucketMax; bucketi++) {
      CountOff co = CO[bucketi];
      if (co.count != 0) {
        std::cout << "[" << bucketi << "] = " << co.offset << std::endl;
      }
    }
    std::cout << "-------- " << std::endl;
  }

  if (debugDumpPrefixSum) {
    std::cout << "countingSortInPlace D = " << D << " next buckets:" << std::endl;

    for (unsigned int bucketi = 0; bucketi < bucketMax; bucketi++) {
      CountOff co = CO[bucketi];
      if (co.count != 0) {
        std::cout << "[" << bucketi << "] = " << (unsigned int)nextBuckets[bucketi] << std::endl;
      }
    }
    std::cout << "-------- " << std::endl;
  }
  
  // Setup initial conditions and loop over all values in range
  
#if defined(DEBUG)
  unsigned int slotWrites = 0;
#endif
  
  readi = starti;
  readVal = arr[readi];
  
  for ( ; readi < endi ; ) {
    unsigned int bucketi = extractDigit<D>(readVal);
    
#if defined(DEBUG)
    assert(bucketi < bucketMax);
#endif
    CountOff co = CO[bucketi];
    
    unsigned int writei = co.offset;
#if defined(DEBUG)
    assert(writei >= starti);
    assert(writei < endi);
#endif
    
    if (readi == writei) {
      // The value at readi happens to be in the correct spot already,
      // value was not previously swapped into this location.
      
      arr[readi] = readVal;
#if defined(DEBUG)
      slotWrites += 1;
#endif
      readi += 1;
      
      // increment offset and decrement count (single write)
      {
        CountOff changed = co;
        changed.offset += 1;
        changed.count -= 1;
        CO[bucketi] = changed;
      }
      
      // If writing this single value finished off a bucket range
      const bool isBucketEmpty = co.count == 1;
      
      if (isBucketEmpty) {
        // Bucket now empty, skip 0 to N other empty buckets, then partial bucket
        
        unsigned int nextBucketi = nextBuckets[bucketi];
        
        while ((nextBucketi != 0) and (CO[nextBucketi].count == 0)) {
          nextBucketi = nextBuckets[nextBucketi];
        }
        
        // Post empty bucket loop, skip partial in non-empty bucket
        if (nextBucketi != 0) {
          readi = CO[nextBucketi].offset;
#if defined(DEBUG)
    assert(readi >= starti);
    assert(readi < endi);
#endif
        } else {
          readi = endi;
        }
      }
      
      // unconditional load, reloads from current slot on final iteration
      readVal = arr[(readi < endi) ? readi : writei];
    } else {
      // Write value into position writei
      uint32_t tmp = arr[writei];
      
      arr[writei] = readVal;
      
#if defined(DEBUG)
      slotWrites += 1;
#endif
      
      // increment offset and decrement count (single write)
      {
        CountOff changed = co;
        changed.offset += 1;
        changed.count -= 1;
        CO[bucketi] = changed;
      }
      
      // Optimal path, a series of writes into position
      // happen without needing to write tmp back to
      // arr[readi] or re-read from the same location.
      
      readVal = tmp;
    }
  } // end foreach starti -> endi
  
  // process each non-empty bucket, starting from starti
  
  if constexpr (D > 0)
  {
    unsigned int bucketi = nextNonEmptyBucket;
    unsigned int currentBucketStartOffset = starti;
    
    while (1) {
      readi = CO[bucketi].offset;
      
      recurse(arr, currentBucketStartOffset, readi);
      
      currentBucketStartOffset = readi;
      
      bucketi = nextBuckets[bucketi];
      
      if (bucketi == 0) {
        break;
      }
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
