#include<iostream>
#include"ADS_set.h"
#include<vector>
using namespace std;

int main() {
  ADS_set<int, 2> mySet = ADS_set<int, 2>();



  while (true) {
    mySet.dump();
    vector<int> inpv;
    string inp;
    cin >> inp;
    switch (stoi(inp)) {
      case 1:
        cin >> inp;
        cout << inp << (mySet.count(stoi(inp)) ? " EXISTS" : " DOESNT EXIST") << "\n";
        break;
      case 2:
        cin >> inp;
        inpv.push_back(stoi(inp));
        mySet.insert(inpv.begin(), inpv.end());
        break;
      default:
        break;
    }
  } 

}