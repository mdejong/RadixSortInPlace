import unittest

from in_place_sort import countingSortInPlace

class TestStringMethods(unittest.TestCase):

  # Trivial no swap and single swap cases

  def test_A_trivial_1(self):
    arr = [ ]
    expected = [ ]
    countingSortInPlace(arr, 1)
    self.assertEqual(arr, expected)

  def test_A_trivial_2(self):
    arr = [ 0 ]
    expected = [ 0 ]
    countingSortInPlace(arr, 1)
    self.assertEqual(arr, expected)

  def test_A_trivial_3(self):
    arr = [ 0, 0 ]
    expected = [ 0, 0 ]
    countingSortInPlace(arr, 1)
    self.assertEqual(arr, expected)

  def test_A_trivial_4(self):
    arr = [ 0, 1 ]
    expected = [ 0, 1 ]
    countingSortInPlace(arr, 1)
    self.assertEqual(arr, expected)
  
  def test_A_trivial_5(self):
    arr = [ 1, 0 ]
    expected = [ 0, 1 ]
    countingSortInPlace(arr, 1)
    self.assertEqual(arr, expected)

  def test_A_trivial_6(self):
    # 4 different values, swap (11, 21) and then
    # bucket1 is empty after the swap
    arr = [ 12, 0, 11, 21 ]
    expected = [ 0, 11, 21, 12 ]
    countingSortInPlace(arr, 1)
    self.assertEqual(arr, expected)

  # Simple duplicate symbols cases

  def test_B_dup_1(self):
    arr = [ 1, 0, 0 ]
    expected = [ 0, 0, 1 ]
    countingSortInPlace(arr, 1)
    self.assertEqual(arr, expected)

  def test_B_dup_2(self):
    arr = [ 0, 1, 0 ]
    expected = [ 0, 0, 1 ]
    countingSortInPlace(arr, 1)
    self.assertEqual(arr, expected)

  def test_B_dup_3(self):
    arr = [ 0, 0, 1 ]
    expected = [ 0, 0, 1 ]
    countingSortInPlace(arr, 1)
    self.assertEqual(arr, expected)

  def test_B_dup_4(self):
    arr = [ 0, 0, 2 ]
    expected = [ 0, 0, 2 ]
    countingSortInPlace(arr, 1)
    self.assertEqual(arr, expected)

  def test_B_dup_5(self):
    arr = [ 0, 0, 2, 0 ]
    expected = [ 0, 0, 0, 2 ]
    countingSortInPlace(arr, 1)
    self.assertEqual(arr, expected)

  def test_B_dup_4(self):
    arr = [ 1, 1, 1, 0 ]
    expected = [ 0, 1, 1, 1 ]
    countingSortInPlace(arr, 1)
    self.assertEqual(arr, expected)

  # Non-trivial duplicate symbols

  def test_C_nt_dup_1(self):
    arr = [ 2, 0, 0, 2 ]
    expected = [ 0, 0, 2, 2 ]
    countingSortInPlace(arr, 1)
    self.assertEqual(arr, expected)

  def test_C_nt_dup_2(self):
    arr = [ 2, 0, 1, 1 ]
    expected = [ 0, 1, 1, 2 ]
    countingSortInPlace(arr, 1)
    self.assertEqual(arr, expected)

  def test_C_nt_dup_3(self):
    arr = [ 2, 1, 0, 0, 2 ]
    expected = [ 0, 0, 1, 2, 2 ]
    countingSortInPlace(arr, 1)
    self.assertEqual(arr, expected)

  def test_C_nt_dup_4(self):
    arr =      [ 2, 1, 0, 1, 2 ]
    expected = [ 0, 1, 1, 2, 2 ]
    countingSortInPlace(arr, 1)
    self.assertEqual(arr, expected)

  def test_C_nt_dup_5(self):
    arr =      [ 2, 0, 1, 1, 2, 1 ]
    expected = [ 0, 1, 1, 1, 2, 2 ]
    countingSortInPlace(arr, 1)
    self.assertEqual(arr, expected)

  def test_C_nt_dup_6(self):
    arr =      [ 1, 1, 3, 3, 2, 2 ]
    expected = [ 1, 1, 2, 2, 3, 3 ]
    countingSortInPlace(arr, 1)
    self.assertEqual(arr, expected)

  def test_C_nt_dup_7(self):
    arr =      [ 3, 2, 1, 3, 2, 1 ]
    expected = [ 1, 1, 2, 2, 3, 3 ]
    countingSortInPlace(arr, 1)
    self.assertEqual(arr, expected)

   # Multiple passes for LSD powers of 1 and 10

  def test_D_two_pass_1(self):
    arr = [ 12, 0, 11, 21 ]
    expected = [ 0, 11, 12, 21 ]
    countingSortInPlace(arr, 1)
    # Now [0, 11, 21, 12]
    countingSortInPlace(arr, 10)
    # Now [0, 11, 12, 21]
    self.assertEqual(arr, expected)

  # Split (binary) into buckets based on 10 digit

  def test_E_ten_split_1(self):
    arr = [ 21, 10, 20, 11 ]
    expected = [ 11, 10, 21, 20 ]
    countingSortInPlace(arr, 10)
    self.assertEqual(arr, expected)

  def test_E_ten_split_2(self):
    arr = [ 31, 10, 30, 11 ]
    expected = [ 11, 10, 31, 30 ]
    countingSortInPlace(arr, 10)
    self.assertEqual(arr, expected)

  def test_E_ten_split_3(self):
    arr = [ 22, 21, 20, 12, 11, 10 ]
    expected = [ 10, 11, 12, 20, 21, 22 ]
    countingSortInPlace(arr, 10)
    # MSD recursion [0, 3] [3, 6]
    countingSortInPlace(arr, 1, 0, 3)
    countingSortInPlace(arr, 1, 3, 6)
    self.assertEqual(arr, expected)

  def test_E_all_same_bucket_1(self):
    # all 3 values in bucket 9, no recursion
    arr = [ 9, 8, 7 ]
    expected = [ 7, 8, 9 ]
    countingSortInPlace(arr, 1)
    self.assertEqual(arr, expected)

  def test_E_all_same_bucket_2(self):
    # all 3 values in bucket 9, no need
    # to iterate over all the values in
    # the 10 pass, but still need to
    # recurse into the 1 pass to sort
    # in terms of the LSD.
    arr = [ 99, 98, 97 ]
    expected = [ 97, 98, 99 ]
    countingSortInPlace(arr, 10, 0, 3, True)
    self.assertEqual(arr, expected)

if __name__ == '__main__':
    unittest.main()
