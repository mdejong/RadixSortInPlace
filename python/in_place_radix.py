from in_place_sort import countingSortInPlace

# Method to do Radix Sort
def radixSort(arr, exp):
    # Find the maximum number to know number of digits
    max1 = max(arr)

    # MSD radix for each digit
    #exp = 10
    countingSortInPlace(arr, exp, 0, len(arr), True)

# Driver code 
arr = [ 20, 10, 0, 12, 5, 0 ]
print(arr)
radixSort(arr, 10)

#for i in range(len(arr)):
#    print(arr[i],end=" ")

print()
print(arr)
