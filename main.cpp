#include<iostream>
#include"ads_set.h"
#include<vector>
using namespace std;

int main() {
  // size_t result = hash<string>{}("a");
  // cout << result << "\n";

  // int mask = (1 << 3) - 1;
  // int last_n_bits = 8 & mask;

  // cout << last_n_bits << "\n";
  vector<int> vec {39, 38, 37, 31, 36, 32, 35, 33};

  ADS_set<int> set = ADS_set<int>();
  set.insert(vec.begin(), vec.end());
  set.dump(cout);
}