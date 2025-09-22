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

    # Walk forward through array and swap if the first
    # unprocessed element is not in the proper location.

    currentBucketi = -1
    currentBucketStartOffset = -1
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

      print(f"currentBucketi is {currentBucketi}")

      bucketCount = counts[bucketi]
      isBucketEmpty = bucketCount == 0
      print(f"isBucketEmpty is {isBucketEmpty}")

      # FIXME: If the element to be swapped with has the same bucket value
      # then a swap could be skipped to avoid reordering.

      if isBucketEmpty:
        arr[readi] = readVal

        print(f"advance since bucket count is zero")
        print(f"empty bucket skip ahead to {writei} from {readi}")
        currentBucketi = bucketi
        currentBucketStartOffset = readi
        readi = writei

        # Check for empty bucket, when zero elements remain in
        # this bucket then recurse into another MSD for the subrange
        if recurse:
          if exp1 >= 10:
            n = readi - currentBucketStartOffset;
            if n > 1:
              countingSortInPlace(arr, exp1/10, currentBucketStartOffset, readi, recurse)
        
        # Loop reload readVal
        #if readi < endi:
        #  print(f"Loop Load {arr[readi]} at readi {readi}")
        #  readVal = arr[readi]

        # readVal = arr[(readi < endi) ? readi : currentBucketStartOffset];
        readVal = arr[readi if (readi < endi) else currentBucketStartOffset]
      elif readi == writei:
        arr[readi] = readVal

        # The value at readi happens to be in the correct spot already,
        # value was not previously swapped into this location.
        print(f"value already in correct spot at {writei}")
        print(f"currentBucketi is {currentBucketi}")

        if bucketi != currentBucketi:
          print(f"New bucket {bucketi} != {currentBucketi}")
          #print(f"New bucket skip ahead to {writei} from {readi}")
          #readi = writei
          currentBucketi = bucketi
          print(f"currentBucketi set to {currentBucketi}")
          currentBucketStartOffset = readi
          print(f"currentBucketStartOffset set to {currentBucketStartOffset}")
        
        readi += 1
        
        # The counts/offsets of the element not swappd is advanced
        # when it is in the correct location.
        print(f"update counts[ {bucketi} ] now {counts[bucketi]}")
        print(f"update offsets[ {bucketi} ] now {offsets[bucketi]}")
        # FIXME: could optimize as write (writei+1) instead of read/write, same for counts
        offsets[bucketi] += 1
        counts[bucketi] -= 1
        print(f"mod counts[ {bucketi} ] = {counts[bucketi]}")
        print(f"mod offsets[ {bucketi} ] = {offsets[bucketi]}")
        # Check for empty bucket and do MSG recursion into the subrange.
        # Note that bucketCount test ignores the counts[bucketi] subtract above
        isBucketEmpty = bucketCount == 1
        if recurse and isBucketEmpty:
          if exp1 >= 10:
            n = readi - currentBucketStartOffset;
            if n > 1:
              countingSortInPlace(arr, exp1/10, currentBucketStartOffset, readi, recurse)

        # Loop reload readVal
        #if readi < endi:
        #  print(f"Loop Load {arr[readi]} at readi {readi}")
        #  readVal = arr[readi]

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

    print(f"countingSortInPlace returns : {arr} : {exp1} : {starti} up to {endi}")
