#include <iostream>
#include <string>
#include <vector>

class Faction {
 public:
  int id;
  std::string name;
};

class Organization {
 public:
  int id;
  std::string name;
};

std::vector<std::string> Goals;
std::vector<std::string> Traits;
std::vector<std::string> Actions;

void LoadVectorFromFile( const std::string &flName , std::vector<std::string> &vec ) {

}

class PoliticalActor {
 public:
  int id;
  std::string name;
  std::vector<std::string> traits;
  std::vector<std::string> goals;
  std::vector<int> factions;
  std::vector<int> organizations;
};

int main(void) {

}
