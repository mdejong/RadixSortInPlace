#include <iostream>
#include <cstdint>

#if defined(DEBUG)
#include <assert.h>
#endif

#include "bit_set_256.hpp"

#include <sys/sysctl.h>

static inline
size_t cache_line_size() {
    size_t line_size = 0;
    size_t sizeof_line_size = sizeof(line_size);
    sysctlbyname("hw.cachelinesize", &line_size, &sizeof_line_size, 0, 0);
    return line_size;
}

static inline
size_t l1_data_cache_size() {
    size_t l1_size = 0;
    size_t sizeof_l1_size = sizeof(l1_size);
    sysctlbyname("hw.l1dcachesize", &l1_size, &sizeof_l1_size, 0, 0);
    return l1_size;
}

// Return number of bytes in one memory page

static inline
size_t page_size() {
    size_t page_size = 0;
    size_t sizeof_page_size = sizeof(page_size);
    sysctlbyname("hw.pagesize", &page_size, &sizeof_page_size, 0, 0);
    return page_size;
}

// Given a 32 bit integer, extract a specific digit.
//
// uint32_t digit = extractDigitOpt<0>(v, digitOffset);

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
    bucketi = extractDigitOpt<D>(readVal);
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
    unsigned int bucketi0 = extractDigitOpt<D>(arr[readi+0]);
    unsigned int bucketi1 = extractDigitOpt<D>(arr[readi+1]);

#if defined(DEBUG)
    assert(bucketi0 < bucketMax);
    assert(bucketi1 < bucketMax);
#endif
    
    ++table1[bucketi0];
    ++table2[bucketi1];
  }
  for (; readi < endi; readi++) {
    bucketi = extractDigitOpt<D>(arr[readi]);
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
    bucketi = extractDigitOpt<D>(readVal);
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
    unsigned int bucketi0 = extractDigitOpt<D>(arr[readi+0]);
    unsigned int bucketi1 = extractDigitOpt<D>(arr[readi+1]);
    unsigned int bucketi2 = extractDigitOpt<D>(arr[readi+2]);
    unsigned int bucketi3 = extractDigitOpt<D>(arr[readi+3]);

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
    bucketi = extractDigitOpt<D>(arr[readi]);
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
    bucketi = extractDigitOpt<D>(readVal);
#if defined(DEBUG)
    assert(bucketi < bucketMax);
#endif
  }
#endif // UNROLL_HISTOGRAMS4
}

// D is digit 3,2,1,0 for 32 bit unsigned int inputs. This hybrid of American Flag sort and SkaSort
// significantly outperforms both earlier implementations.

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
          countingSortInPlaceOpt<D-1>(arr, starti, endi);
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
  // Init both to zero to support multiple uses.

  uint32_t counts[bucketMax] = {};
  uint32_t offsets[bucketMax] = {};
  
  // Histogram counts
  unsigned int histogramBucketi = bucketMax;
  
  histogramOpt<D, bucketMax>(arr, starti, endi, histogramBucketi, counts, offsets);
  
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
  
  if (histogramBucketi != bucketMax) {
    auto count = counts[histogramBucketi];
    if (count == n) {
      // Special case where all values scan into a single bucket.
      if (debugOut) {
        std::cout << "all " << n << " values in same bucket " << histogramBucketi << std::endl;
      }
      
      recurse(arr, starti, endi);
      
      return;
    }
  }
  
#if defined(DEBUG)
  // Double check bucket start/end offsets as as they become empty (before recursion)
  uint32_t checkBucketStart[bucketMax] = {};
  uint32_t checkBucketEnd[bucketMax] = {};
  bool checkBucketRecursion[bucketMax] = {};
#endif
  
  // Prefix Sum (given starting offset), note that after this loop is completed the
  // counts field is converted to an end of bucket offset.
  
  unsigned int psum = starti;
  for (unsigned int bucketi = 0; bucketi < bucketMax; bucketi++) {
    auto count = counts[bucketi];
    offsets[bucketi] = psum;
    psum += count;
    
#if defined(DEBUG)
    checkBucketStart[bucketi] = offsets[bucketi];
    checkBucketEnd[bucketi] = offsets[bucketi] + counts[bucketi];
#endif
    
    counts[bucketi] = psum;
  }
  
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
  
  // This nonEmptyBuckets bitset contains a flag for each bucket that starts out as
  // non-empty. When a bucket is discovered to be fully processed, the bit is cleared.

  bitset256_t nonEmptyBuckets;
  bitset256Clear(nonEmptyBuckets);
  
  for (unsigned int bucketi = 0; bucketi < bucketMax; bucketi++) {
    auto count = counts[bucketi] - offsets[bucketi];
    if (count > 0) {
      bitset256SetBit(nonEmptyBuckets, bucketi);
    }
  }
  
  if (debugDumpPrefixSum) {
    std::cout << "countingSortInPlace D = " << D << " non-empty buckets:" << std::endl;

    bitset256_t copy;
    bitset256CopyBits(nonEmptyBuckets, copy);
    
    for ( ; !bitset256IsAllOff(copy) ; ) {
      auto bucketi = bitset256FindFirstSetOpt(copy);
      bitset256ClearBit(copy, bucketi);
      
      std::cout << "[" << bucketi << "] = " << " true" << std::endl;
    }
    
    std::cout << "-------- " << std::endl;
  }
  
#if defined(DEBUG)
  bitset256_t nonEmptyBucketsCopy;
  bitset256CopyBits(nonEmptyBuckets, nonEmptyBucketsCopy);
#endif
  
  auto isBucketEmpty = [](
                          unsigned int bucketi,
                          uint32_t * offsets,
                          uint32_t * counts
                          ) {
    return offsets[bucketi] == counts[bucketi];
  };
  
  auto dump = [
               &arr,
               &starti,
               &endi
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
  
  constexpr bool debugDumpAllValuesOnIterations = false;
  
  constexpr bool debugDumpBucketBounds = false;
  
  constexpr bool debugDumpReloadedBucketN = false;
  
  if (debugDumpAllValuesOnIterations) {
    dump();
  }
  
  // foreach bucket, iterate over the elements in the bucket. When elements are swapped into
  // other buckets the iteration moves ahead so that processing need not wait for the result
  // of each swap operation. As a result, one bucket can be iterated over and become empty or
  // it may be defered and iterated over again after all buckets have been iterated over.

#if defined(DEBUG)
  unsigned int slotWrites = 0;
#endif
  
  bitset256_t bucketsThisIteration;
  bitset256CopyBits(nonEmptyBuckets, bucketsThisIteration);
  
  size_t bucketsThisIterationNum = bitset256PopCount(bucketsThisIteration);
  
#if defined(DEBUG)
  size_t totalNumberOfReloads = 0;
#endif
  
  // Reaload into bits by starting with the non-empty bits and keeping buckets that are not empty now
    
  auto reloadBuckets = [&]() {
    bitset256CopyBits(nonEmptyBuckets, bucketsThisIteration);
    bucketsThisIterationNum = bitset256PopCount(bucketsThisIteration);
    
#if defined(DEBUG)
    totalNumberOfReloads += 1;
#endif
    
    if (debugDumpReloadedBucketN) {
      std::cout << "reload reset numNonEmptyBuckets " << bucketsThisIterationNum << std::endl;
    }

    if (debugDumpReloadedBucketN) {
      // Dump an histogram count for each bucket that is not empty
      
      for (unsigned int bucketi = 0; bucketi < bucketMax; bucketi++) {
        auto count = counts[bucketi] - offsets[bucketi];
        
        if (count > 0) {
          std::cout << "(reloaded) hist [" << bucketi << "] = " << count << std::endl;
        }
      }
    }
    
    return;
  };
    
#if defined(DEBUG)
  size_t totalCountDown = endi - starti;
#endif
  
  while ( bucketsThisIterationNum != 0 ) {
#if defined(DEBUG)
    if (debugDumpBucketBounds) {
      std::cout << "totalNumberOfReloads " << totalNumberOfReloads << std::endl;
      std::cout << "bucketsThisIterationNum " << bucketsThisIterationNum << std::endl;
      std::cout << "numLeft " << totalCountDown << std::endl;
    }
#endif
    
    unsigned int currentBucketi = bitset256FindFirstSetOpt(bucketsThisIteration);
    
    unsigned int currentBucketOffset = offsets[currentBucketi];
    unsigned int currentBucketEndOffset = counts[currentBucketi];
    unsigned int currentBucketN = currentBucketEndOffset - currentBucketOffset;
    
    if (debugDumpBucketBounds) {
      std::cout << "process bucket [" << currentBucketi << "] at offset " << currentBucketOffset << " has N = " << currentBucketN << std::endl;
    }
    
    // Inner loop processes N elements in the bucket until the bucket is empty
    size_t bucketIterN = currentBucketN;
    
    while ( bucketIterN != 0 ) {
      if (debugDumpBucketBounds) {
        std::cout << "bucket [" << currentBucketi << "] inner loop over N = " << bucketIterN << " slots " << std::endl;
      }

      size_t loopCountDown = bucketIterN;
      
      // Iterate over the values in the current bucket (max bucketCountDown number of loops)
      
      // In the self swap special case, offsets[currentBucketi] is incremented by one and
      // currentBucketOffset would then match the offset.
      
      // In the write to other bucket case, offsets[currentBucketi] does not get incremented
      // but currentBucketOffset does, so the next inner loop reads a value to swap from
      // from the slot after the one in the previous loop (avoids data stall in next loop).
      
      // no loop unrolling seems to perform the best
      //#pragma unroll(4)
      for ( ; loopCountDown != 0; --loopCountDown) {
        if (debugDumpAllValuesOnIterations) {
          dump();
        }
        
#if defined(DEBUG)
        assert(currentBucketOffset < currentBucketEndOffset);
        assert(currentBucketOffset >= offsets[currentBucketi]);
#endif
        
        unsigned int writeBucketi = extractDigitOpt<D>(arr[currentBucketOffset]);
    
    #if defined(DEBUG)
        assert(writeBucketi < bucketMax);
    #endif
        
        unsigned int writei = offsets[writeBucketi]++; // increment offsets[writeBucketi] either way
    #if defined(DEBUG)
        assert(writei >= starti);
        assert(writei < endi);
    #endif

        if (debugDumpIterations) {
          std::cout << "reshuffle swap [" << currentBucketOffset << "] <-> [" << writei << "] into bucket " << writeBucketi << std::endl;
          std::cout << "reshuffle swap( " << arr[currentBucketOffset] << " <-> " << arr[writei] << " ) into bucket " << writeBucketi << std::endl;
        }
        
    #if defined(DEBUG)
        slotWrites += 1;
    #endif
        
        std::iter_swap(&arr[currentBucketOffset], &arr[writei]);
        
        
#if defined(DEBUG)
        bool selfSwap = currentBucketOffset == writei;
#endif
        
        ++currentBucketOffset;
        
#if defined(DEBUG)
        if (selfSwap) {
          assert(currentBucketOffset == offsets[currentBucketi]);
        } else {
          assert(currentBucketOffset > offsets[currentBucketi]);
        }
#endif
      } // while ( loopCountDown != 0 )

#if defined(DEBUG)
      if (bucketIterN > 0) {
        assert(totalCountDown > 0);
      }
      assert(totalCountDown >= bucketIterN);
      totalCountDown -= bucketIterN; // consumed bucketCountdown slots in inner loop
#endif
      
      // After the inner loop, the bucket iteration has advanced currentBucketOffset
      // to the end of the bucket. Start the bucket iteration over again since this
      // bucket can continue to be processed.
      
#if defined(DEBUG)
        assert(currentBucketOffset <= currentBucketEndOffset);
#endif
      
      currentBucketOffset = offsets[currentBucketi];
      bucketIterN = currentBucketEndOffset - currentBucketOffset;
      
      if (debugDumpBucketBounds) {
        std::cout << "bucket [" << currentBucketi << "] inner loop over N = " << bucketIterN << " slots done" << std::endl;
        std::cout << "";
      }
      
      switch (bucketIterN) {
        case 0: {
          // nop
          break;
        }
        case 1: {
          // The bucket contains a single item, so it is no longer possible
          // to invoke the inner loop.
          
          if (debugDumpBucketBounds) {
            std::cout << "bucket [" << currentBucketi << "] clear current iter bucket bit to move forward to next bucket" << std::endl;
          }

          bitset256ClearBit(bucketsThisIteration, currentBucketi);
          --bucketsThisIterationNum;
          bucketIterN = 0;
          
          break;
        }
        default: {
          // N > 1
          
          if (debugDumpBucketBounds) {
            std::cout << "bucket [" << currentBucketi << "] keep going" << std::endl;
            std::cout << "";
          }

          break;
        }
      }
       
    } // end while (bucketIterN != 0)
    
    // After a bucket iteration, the bucket may be empty. Once a bucket is empty, it will be recursed into
    // and it will be ignored in future iterations. Note the case where a bucket is ignored in one iteration
    // and then it becomes empty due to a different bucket writing into it, that bucket will still be processed here.
    
    if (isBucketEmpty(currentBucketi, offsets, counts)) {
      // All bucket slots consumed by inner loop, remove bucket from bucketsThisIteration and nonEmptyBuckets
      
      if (debugDumpBucketBounds) {
        std::cout << "bucket [" << currentBucketi << "] bit cleared" << std::endl;
      }
      
      bitset256ClearBit(bucketsThisIteration, currentBucketi);
      --bucketsThisIterationNum;
      
      {
        unsigned int currentBucketStartOffset = ((currentBucketi == 0) ? starti : counts[currentBucketi - 1]);
        unsigned int currentBucketEndOffset = counts[currentBucketi];
        
#if defined(DEBUG)
        {
          auto expectedStart = checkBucketStart[currentBucketi];
          auto expectedEnd = checkBucketEnd[currentBucketi];
          
          assert(currentBucketStartOffset == expectedStart);
          assert(currentBucketEndOffset == expectedEnd);
        }
#endif
        // Once bucket recursion has been executed, clear bit in nonEmptyBuckets
        
#if defined(DEBUG)
        {
          // This isBucketEmpty() check is the only place where a nonEmptyBuckets
          // bit can be flipped off and this can only happen once for a bucket.
          auto inNonEmpty = bitset256GetBit(nonEmptyBuckets, currentBucketi);
          assert(inNonEmpty == true);
        }
#endif
        
        if (1) {
          if (debugDumpBucketBounds) {
            std::cout << "recurse into bucket " << currentBucketi << std::endl;
          }
          
#if defined(DEBUG)
          auto recursedAlready = checkBucketRecursion[currentBucketi];
          assert(recursedAlready == false);
          checkBucketRecursion[currentBucketi] = true;
#endif
          
          bitset256ClearBit(nonEmptyBuckets, currentBucketi);
          
          recurse(arr, currentBucketStartOffset, currentBucketEndOffset);
        }
      }
    } // end isBucketEmpty()
    
    if (bucketsThisIterationNum == 0) {
      if (debugDumpBucketBounds) {
        std::cout << "buckets reloaded since bucketsThisIterationNum is zero" << std::endl;
      }

      reloadBuckets();
    }

  } // end while ( numNonEmptyBuckets != 0 )
  
#if defined(DEBUG)
    if (debugDumpBucketBounds) {
      std::cout << "all buckets now empty totalNumberOfReloads " << totalNumberOfReloads << std::endl;
    }
#endif

  // All buckets must be empty at this point
  
#if defined(DEBUG)
  {
    for (unsigned int bucketi = 0; bucketi < bucketMax; bucketi++) {
      auto count = counts[bucketi] - offsets[bucketi];
      assert (count == 0);
      
      auto wasNonEmpty = bitset256GetBit(nonEmptyBucketsCopy, bucketi);
      
      if (wasNonEmpty) {
        auto recursedAlready = checkBucketRecursion[bucketi];
        assert(recursedAlready == true);
        
        auto bitSet = bitset256GetBit(nonEmptyBuckets, bucketi);
        assert(bitSet == false);
      }
    }
  }
#endif
  
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
