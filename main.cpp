#include<iostream>
#include"ads_set.h"
#include<vector>
using namespace std;

int main() {
  vector<int> vec {39, 38, 37, 31, 36, 32, 35, 33, 17};

  ADS_set<int, 2> set = ADS_set<int, 2>();
  set.insert(vec.begin(), vec.end());
  set.dump(cout);
}