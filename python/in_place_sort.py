def countingSortInPlace(arr, exp1, starti=-1, endi=-1, recurse=False): 
    n = len(arr)

    if starti == -1:
      starti = 0
    if endi == -1:
      endi = n

    print(f"countingSortInPlace : {arr} : {exp1} : {starti} up to {endi}")

    # Sorting [] or [1] is a nop
    if n < 2:
      print(f"countingSortInPlace early return from recursion {starti} up to {endi}")
      return
    
    bucketMax = 10
    # initialize count array as 0 
    counts = [0] * (bucketMax) 
    # initialize offset array
    offsets = [0] * (bucketMax)

    # hold value of last bucketi lookup after count loop
    bucketi = -1

    # Store count of occurrences
    readi = starti
    while readi < endi:
      index = (arr[readi]/exp1)
      bucketi = int((index % bucketMax))
      counts[bucketi] += 1
      readi += 1

    print(f"counts  {counts}")

    # Special case where all values map to the same bucket.
    # The count logic needs to be done, but prefix sum and
    # looping over all the values to calculate buckets will
    # result in no reordering.

    if bucketi != -1:
      c = counts[bucketi]
      if c == n:
        # Special case where all values scan into a single bucket.
        print(f"all {n} values in same bucket {bucketi}")

        if recurse:
          if exp1 >= 10:
            currentBucketStartOffset = starti
            readi = endi
            n = endi - currentBucketStartOffset
            if n > 1:
              countingSortInPlace(arr, exp1/10, currentBucketStartOffset, readi, recurse)

        return

    # Prefix sum written to offsets, note that starti is the minimum
    # offset in the indicated range.
    psum = starti
    for bucketi in range(0,bucketMax):
      c = counts[bucketi]
      offsets[bucketi] = psum
      psum += c

    print(f"offsets {offsets}")

    # Create a second "next bucket" table that given
    # a bucketi this finds the next bucket indicated
    # as being not-empty. If a bucket is not used
    # then this table entry is zero, and the final
    # used bucket contains a zero to indicate the end.

    nextBuckets = [0] * (bucketMax)
    nextNonEmptyBucket = 0

    for bucketi in range(bucketMax-1, -1, -1):
      #print(f"nextBuckets bucketi {bucketi}")
      c = counts[bucketi]
      #print(f"bucketi count {c}")
      if c > 0:
        nextBuckets[bucketi] = nextNonEmptyBucket
        nextNonEmptyBucket = bucketi
    
    print(f"nextNonEmptyBucket {nextNonEmptyBucket}")
    print(f"nextBuckets {nextBuckets}")

    # track total number of writes to slots

    slotWrites = 0

    # Walk forward through array and swap if the first
    # unprocessed element is not in the proper location.

    readi = starti

    # Initial load of readVal before while loop
    readVal = arr[readi]
    print(f"Initial Load {arr[readi]} at readi {readi}")

    print(f"Loop from {readi} up to {endi}")
    while readi < endi:
      print(arr)
      print(f"counts  (loop) {counts}")
      print(f"offsets (loop) {offsets}")

      print(f"Consider {readVal} at readi {readi}")
      #readVal = arr[readi]
      radixVal = (readVal / exp1)
      bucketi = int(radixVal % bucketMax)
      print(f"bucketi {bucketi}")
      # Digit should be written to this offset
      writei = offsets[bucketi]

      print(f"Current digit value {radixVal}")
      print(f"Current readi {readi}")
      print(f"Current writei {writei}")

      if readi == writei:
        arr[readi] = readVal
        slotWrites += 1

        # The value at readi happens to be in the correct spot already,
        # value was not previously swapped into this location.
        print(f"value already in correct spot at {writei}")
        
        readi += 1
        
        bucketCount = counts[bucketi]

        # The counts/offsets of the element not swappd is advanced
        # when it is in the correct location.
        print(f"update counts[ {bucketi} ] now {counts[bucketi]}")
        print(f"update offsets[ {bucketi} ] now {offsets[bucketi]}")
        # FIXME: could optimize as write (writei+1) instead of read/write, same for counts
        offsets[bucketi] += 1
        counts[bucketi] -= 1
        print(f"mod counts[ {bucketi} ] = {counts[bucketi]}")
        print(f"mod offsets[ {bucketi} ] = {offsets[bucketi]}")

        # Writing into the current bucket, so check for the final write
        # and if the bucket is empty then need to skip over any empty
        # buckets to the right until a non-empty bucket is found. Note
        # that current bucket could be the last one, skip to end in that case.

        isBucketEmpty = bucketCount == 1 

        if isBucketEmpty:
          # Skip over 0 to N empty buckets
          nextBucketi = nextBuckets[bucketi]
          print(f"possible bucket skip starting with {nextBucketi} since current bucket is empty")
          while nextBucketi != 0 and counts[nextBucketi] == 0:
            print(f"bucket to right {nextBucketi} is empty")
            nextBucketi = nextBuckets[nextBucketi]
          # A non-empty bucket found, skip partial bucket
          if nextBucketi != 0:
            readi = offsets[nextBucketi]
            print(f"advance readi to {readi} when a non-empty bucket found")
          else:
            readi = endi
            print(f"advance readi to {endi} when end of buckets found")

        # readVal = arr[(readi < endi) ? readi : writei];
        readVal = arr[readi if (readi < endi) else writei]
      else:
        print(f"Otherwise swap")
        # (writei > readi) so swap swap(readi, writei) to the right
        # and update the offsets and counts.
        #tmp1 = arr[readi]
        tmp1 = readVal
        tmp2 = arr[writei]
        print(f"swap values ( {tmp1} , {tmp2} )")
        print(f"swap offsets ( {readi} , {writei} )")
        #arr[readi] = tmp2
        arr[writei] = readVal
        slotWrites += 1
        # Update counts and offsets after swap
        print(f"update counts[ {bucketi} ] now {counts[bucketi]}")
        print(f"update offsets[ {bucketi} ] now {offsets[bucketi]}")
        offsets[bucketi] += 1
        counts[bucketi] -= 1
        print(f"mod counts[ {bucketi} ] = {counts[bucketi]}")
        print(f"mod offsets[ {bucketi} ] = {offsets[bucketi]}")
        # In optimal swap path, use tmp2 for readVal in next iteration
        readVal = tmp2
        print(f"Loop continue {readVal} at readi {readi}")
      
    # end while readi < endi loop
    print(f"end of foreach value in range loop")

    # process each non-empty bucket, starting from starti
    bucketi = nextNonEmptyBucket
    currentBucketStartOffset = starti

    while True:
      print(f"Subrange bucketi {bucketi} : range {currentBucketStartOffset} to {readi}")

      if recurse:
        if exp1 >= 10:
          readi = offsets[bucketi]
          n = readi - currentBucketStartOffset
          print(f"subrange bucket n for {bucketi} is {n}")
          if n > 1:
            countingSortInPlace(arr, exp1/10, currentBucketStartOffset, readi, recurse)

      currentBucketStartOffset = readi
      bucketi = nextBuckets[bucketi]
      if bucketi == 0:
        break

    print(f"countingSortInPlace returns : {arr} : {exp1} : {starti} up to {endi}")
    n = endi - starti
    print(f"n {n} : slotWrites {slotWrites}")
    if n != slotWrites:
      raise RuntimeError(f"Slot writes mismatch: expected {n} but got {slotWrites} writes")
