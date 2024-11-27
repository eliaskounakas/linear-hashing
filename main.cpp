#include<iostream>
#include<random>
#include<algorithm>
#include"ADS_set.h"
#include<vector>
#include <bits/stl_algo.h>
using namespace std;

int main() {

  ADS_set<int, 2> mySet;
  

  // while (true) {
  //   mySet.dump();
  //   vector<int> inpv;
  //   string inp;
  //   cin >> inp;
  //   switch (stoi(inp)) {
  //     case 1:
  //       cin >> inp;
  //       cout << inp << (mySet.count(stoi(inp)) ? " EXISTS" : " DOESNT EXIST") << "\n";
  //       break;
  //     case 2:
  //       cin >> inp;
  //       inpv.push_back(stoi(inp));
  //       mySet.insert(inpv.begin(), inpv.end());
  //       break;
  //     default:
  //       break;
  //   }

  //   cout << "\n test Iterator: ";
  //   for_each(mySet.begin(), mySet.end(), [](const int& number) {
  //    cout << number << " ";
  //   });
  //   cout << "\n";
  // } 

  for(int i = 1; i < 15; i++) {
    vector<int> inpv;
    inpv.push_back(i);
    mySet.insert(inpv.begin(), inpv.end());
      mySet.dump();
  }

  cout << "\nTest Iterator: ";
  for_each(mySet.begin(), mySet.end(), [](const int& number) {
    cout << number << " ";
  });
  cout << "\n\n";

  mySet.insert(1024);


}