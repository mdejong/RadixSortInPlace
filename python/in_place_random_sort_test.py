import unittest
import random

from in_place_sort import countingSortInPlace

class TestStringMethods(unittest.TestCase):

  # Trivial no swap and single swap cases

  def test_A_sort_1(self):
    # Generate list of random integer values
    n = 9
    arr = random.sample(range(0, 9), n)
    print(arr)
    expected = sorted(arr)
    print(expected)
    countingSortInPlace(arr, 1)
    self.assertEqual(arr, expected)

  def test_B_sort_1(self):
    # Generate list of random integer values
    n = 9
    arr = random.sample(range(0, 99), n)
    print(arr)
    expected = sorted(arr)
    print(expected)
    countingSortInPlace(arr, 10, 0, len(arr), True)
    self.assertEqual(arr, expected)

  def test_B_sort_2(self):
    # Generate list of random integer non-duplicated values
    n = 999
    arr = random.sample(range(0, 999), n)
    #print(arr)
    expected = sorted(arr)
    #print(expected)
    countingSortInPlace(arr, 100, 0, len(arr), True)
    self.assertEqual(arr, expected)

  def test_C_sort_1(self):
    # Generate list of random integer non-duplicated values
    n = 10
    arr = random.sample(range(0, 100), n)
    for i in range(6):
      arr = arr + arr
    #print(arr)
    expected = sorted(arr)
    #print(expected)
    countingSortInPlace(arr, 100, 0, len(arr), True)
    self.assertEqual(arr, expected)

if __name__ == '__main__':
    unittest.main()
