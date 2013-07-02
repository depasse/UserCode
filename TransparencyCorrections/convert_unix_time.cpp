#include <iostream>
#include <stdlib.h>
#include <time.h>

int main(int argc, char **argv) {
  unsigned long long  n = (unsigned long long)atoll(argv[1]);
  time_t tn = n;
  std::cout << "Last timestamp: " << asctime(gmtime(&tn));
}
