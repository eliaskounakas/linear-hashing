#include<iostream>
#include"ADS_set.h"
#include<vector>
using namespace std;

int main() {
  ADS_set<int, 2> mySet = ADS_set<int, 2>();



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

  for(int i = 1; i < 30; i = i *2) {
    vector<int> inpv;
    inpv.push_back(i);
    mySet.insert(inpv.begin(), inpv.end());
  }

  mySet.erase(16);
  mySet.erase(2);

  auto it = mySet.insert(2).first;

  cout << "\n test Iterator: ";
  for_each(it, mySet.end(), [](const int& number) {
    cout << number << " ";
  });
  cout << "\n";

  mySet.dump();

  ADS_set<int, 2> mySecondSet = mySet;
  mySet.clear();

  mySet.dump();
}