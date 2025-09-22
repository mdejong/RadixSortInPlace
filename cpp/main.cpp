#include "in_place_sort.cpp"

int main() {
  uint32_t arr[] = {0xFF, 0xFF-1, 257, 256, 0xFF-2};

  countingSortInPlace<1>(arr, 0, sizeof(arr)/sizeof(arr[0]));

  std::cout << (char*) "sorted output:" << std::endl;

  for (int i = 0 ; i < sizeof(arr)/sizeof(arr[0]); i++) {
    std::cout << (unsigned int) arr[i] << std::endl;
  }

  return 0;
}

/*
int main() {
  uint32_t arr[] = {20, 10, 0, 12, 5, 0};

  countingSortInPlace<0>(arr, 0, sizeof(arr)/sizeof(arr[0]));

  std::cout << (char*) "sorted output:" << std::endl;

  for (int i = 0 ; i < sizeof(arr)/sizeof(arr[0]); i++) {
    std::cout << (unsigned int) arr[i] << std::endl;
  }

  return 0;
}
*/
