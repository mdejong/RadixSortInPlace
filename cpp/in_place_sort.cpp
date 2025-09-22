// https://www.onlinegdb.com/online_c++_compiler

#include <iostream>
#include <cstdint>

typedef struct {
  uint32_t count;
  uint32_t offset;
} CountOff;

// Given a 32 bit integer, extract a specific digit.
//
// uint32_t digit = extractDigit(v, digitOffset);

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
void countingSortInPlace(
  uint32_t * arr,
  unsigned int starti,
  unsigned int endi
  )
{
    constexpr bool debugOut = true;
    int n = endi - starti;

    // if (n < 2) {
    //   std::cout << "countingSortInPlace early return from recursion " << starti << " up to " << endi << std::endl;
    //   std::cout << "n " << n << std::endl;
    //   return;
    // }

    if (debugOut) {
      std::cout << "countingSortInPlace D = " << D << " input:" << std::endl;
      std::cout << "n " << n << std::endl;
      std::cout << "starti " << starti << std::endl;
      std::cout << "endi " << endi << std::endl;

      for (int i = starti; i < endi; i++) {
        std::cout << (unsigned int) arr[i] << std::endl;
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
      CountOff co = CO[bucketi];
      co.count += 1;
      CO[bucketi] = co;
    }

    if (debugOut) {
      std::cout << "countingSortInPlace D = " << D << " counts:" << std::endl;

      unsigned int bucketi;

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

        if constexpr (D > 0) {
          unsigned int currentBucketStartOffset = starti;
          readi = endi;
          n = endi - currentBucketStartOffset;
          if (n > 1) {
            countingSortInPlace<D-1>(arr, currentBucketStartOffset, readi);
          }
        }

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

    // Setup initial conditions and loop over all values in range
    
    unsigned int currentBucketi = bucketMax;
    unsigned int currentBucketStartOffset = 0;

    readi = starti;
    readVal = arr[readi];

    for ( ; readi < endi; ) {
      unsigned int bucketi = extractDigit<D>(readVal);

      CountOff co = CO[bucketi];

      unsigned int writei = co.offset;

      if (co.count == 0) {
        // Bucket is empty

        arr[readi] = readVal;

        currentBucketi = bucketi;
        currentBucketStartOffset = readi;
        readi = writei;

        if constexpr (D > 0) {
          int n = readi - currentBucketStartOffset;
          if (n > 1) {
            countingSortInPlace<D-1>(arr, currentBucketStartOffset, readi);
          }
        }
        
        // Loop reload readVal
        //if (readi < endi) {
        //  readVal = arr[readi];
        //}

        readVal = arr[(readi < endi) ? readi : currentBucketStartOffset];
      } else if (readi == writei) {
        // The value at readi happens to be in the correct spot already,
        // value was not previously swapped into this location.

        arr[readi] = readVal;

        if (bucketi != currentBucketi) {
          currentBucketi = bucketi;
          currentBucketStartOffset = readi;
        }

        readi += 1;

        // increment offset and decrement count (single write)
        {
          CountOff changed = co;
          changed.offset += 1;
          changed.count -= 1;
          CO[bucketi] = changed;
        }

        // If writing this single value finished off a bucket range, then
        // recurse into the now finished range.
        const bool isBucketEmpty = co.count == 1;
        if (isBucketEmpty) {
          if constexpr (D > 0) {
            int n = readi - currentBucketStartOffset;
            if (n > 1) {
              countingSortInPlace<D-1>(arr, currentBucketStartOffset, readi);
            }
          }
        }

        // FIXME: condition select to avoid branch : Rd = cond ? Rn : Rm
        // csel    Rd/zr, Rn/zr, Rm/zr, cond
        //
        // readVal = arr[readi < endi ? readi : starti]

        // Loop reload readVal
        //if (readi < endi) {
        //  readVal = arr[readi];
        //}

        readVal = arr[(readi < endi) ? readi : writei];
      } else {
        // Write value into position writei
        uint32_t tmp = arr[writei];
        arr[writei] = readVal;

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
    }

    if (debugOut) {
      std::cout << "countingSortInPlace D = " << D << " returns:" << std::endl;
      std::cout << "starti " << starti << std::endl;
      std::cout << "endi " << endi << std::endl;

      for (int i = starti; i < endi; i++) {
        std::cout << (unsigned int) arr[i] << std::endl;
      }
      std::cout << "-------- " << std::endl;
    }
}

int main() {
  uint32_t arr[] = {20, 10, 0, 12, 5, 0};

  countingSortInPlace<0>(arr, 0, sizeof(arr)/sizeof(arr[0]));

  std::cout << (char*) "sorted output:" << std::endl;

  for (int i = 0 ; i < sizeof(arr)/sizeof(arr[0]); i++) {
    std::cout << (unsigned int) arr[i] << std::endl;
  }

  return 0;
}

/*
int main() {
  uint32_t arr[] = {0xFF, 0xFF-1, 257, 256, 0xFF-2};

  countingSortInPlace<1>(arr, 0, sizeof(arr)/sizeof(arr[0]));

  std::cout << (char*) "sorted output:" << std::endl;

  for (int i = 0 ; i < sizeof(arr)/sizeof(arr[0]); i++) {
    std::cout << (unsigned int) arr[i] << std::endl;
  }

  return 0;
}
*/
