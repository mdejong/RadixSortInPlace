// Optimized bitset like interface that holds a single bit for each bucket in the range (0 - 255)

// A structure of bitset256_t can be allocated on the stack or from heap memory.
// Note that bitset256Clear() must then be invoked to clear/init the memory.

typedef struct alignas(8*8) {
  uint64_t bits[5];
} bitset256_t;


// Clear all the bits (must be used as opposed to bzero)

static inline
void bitset256Clear(bitset256_t & b)
{
#if defined(DEBUG)
  {
    auto isAligned = [](const void* ptr, const size_t alignment) {
      return reinterpret_cast<uintptr_t>(ptr) % alignment == 0;
    };
    assert(isAligned(&b.bits[0], (8*8)) == true);
  }
#endif
  bzero(b.bits, sizeof(uint64_t) * 4);
  b.bits[5-1] = 5-1;
}

// Returns true if no bits are set in the bitset

static inline
bool bitset256IsAllOff(const bitset256_t & b)
{
  return b.bits[5-1] == 5-1;
}

// Set one bit in a q (util method)

static inline
uint64_t bitset256OneBitQ(unsigned int bucket)
{
  constexpr uint64_t one64 = 0x1;
  return (one64 << bucket);
}

// Set a bit in range 0-255

static inline
void bitset256SetBit(bitset256_t & b, unsigned int bucket)
{
#if defined(DEBUG)
  assert(bucket < 256);
#endif
  uint64_t q = bucket / 64;
#if defined(DEBUG)
  assert(q < 4);
#endif
  b.bits[q] |= bitset256OneBitQ(bucket % 64);
  uint64_t & q5 = b.bits[5-1];
  q5 = std::min(q5, q);

#if defined(DEBUG)
  assert(q5 < 4);
#endif
}

// Query the current state of a bit in the range 0-255

static inline
bool bitset256GetBit(const bitset256_t & b, unsigned int bucket)
{
#if defined(DEBUG)
  assert(bucket < 256);
#endif
  uint64_t q = bucket / 64;
#if defined(DEBUG)
  assert(q < 4);
#endif
  return b.bits[q] & bitset256OneBitQ(bucket % 64);
}

// Clear a previously set bit in range 0-255

static inline
void bitset256ClearBit(bitset256_t & b, unsigned int bucket)
{
#if defined(DEBUG)
  assert(bucket < 256);
#endif
  uint64_t q = bucket / 64;
#if defined(DEBUG)
  assert(q < 4);
#endif
  b.bits[q] &= ~bitset256OneBitQ(bucket % 64);
  uint64_t & q5 = b.bits[5-1];
    
  if (b.bits[q] == 0 and q == q5) {
    // Walk over (0, 1, 2, 3) and find the first quad that
    // is not zero then save as new q5. This is a CTZ(bits5)
    // assuming a bit at offset 4 is always set.
    
    bool b0 = (b.bits[0] != 0);
    bool b1 = (b.bits[1] != 0);
    bool b2 = (b.bits[2] != 0);
    bool b3 = (b.bits[3] != 0);
    bool b4 = true;
    uint64_t bits5 = (b4 << 4) | (b3 << 3) | (b2 << 2) | (b1 << 1) | (b0 << 0);
    q5 = std::countr_zero(bits5);
#if defined(DEBUG)
    assert(q5 <= 4);
    
    {
      // Run looping version and make sure result is the same

      uint64_t tmp = 5-1;
      for (uint64_t nq = 0; nq < 4; nq++) {
        if (b.bits[nq] != 0) {
          tmp = nq;
          break;
        }
      }
      assert(q5 == tmp);
    }
#endif

#if defined(DEBUG)
    if (q5 == (5-1) and false) {
      std::cout << "all bits empty" << std::endl;
    }
#endif
  }
}

// Optimal branchless implementation that will search the min q and execute only
// a single least significant zeros count operation.

static inline
unsigned int bitset256FindFirstSetOpt(bitset256_t & b)
{
  // assert to make sure this function is never invoked for a bitset
  // that has zero bits set. If invoked on an empty bitset in
  // optmized code, this would read from bits[4] = 4 and return
  // a junk result of 2, but that should never be relied on.
  
#if defined(DEBUG)
  assert(b.bits[5-1] != (5-1));
#endif
  
  uint64_t q = b.bits[5-1];

#if defined(DEBUG)
  assert(b.bits[q] != 0);
#endif
  
  unsigned int found = std::countr_zero(b.bits[q]);
  
#if defined(DEBUG)
  assert(found != 256);
#endif

  return (unsigned int) ((q * 64) + found);
}

// Copy the bits from src bitset to a dst bitset

static inline
void bitset256CopyBits(const bitset256_t & src, bitset256_t & dst)
{
  memcpy(dst.bits, src.bits, sizeof(uint64_t) * 5);
}

// Return a count of the number of bits that are on

static inline
unsigned int bitset256PopCount(const bitset256_t & b)
{
  return
    std::popcount(b.bits[0]) +
    std::popcount(b.bits[1]) +
    std::popcount(b.bits[2]) +
    std::popcount(b.bits[3]);
}
