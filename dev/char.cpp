#include <iostream>
#include <cstdlib>
#include <cstdio>

class NPC {
 public:
  std::string _name;

  float _to_hit;
  float _dodge;
  int _hp_max;
  int _hp_current;

  NPC(const float &h, const float &d, const float &max) : _to_hit(h) , _dodge(d) , _hp_max(max) , _hp_current(max) {}

  void heal() {
    this->_hp_current = this->_hp_max;
  }

  bool attack(NPC& enemy) {
    float to_hit = this->_to_hit * float(rand() % 1000);
    float to_dodge = enemy._dodge*float(rand()%1000);
    printf("%s hit %.2f | %s dodge %.3f",this->_name.c_str(),to_hit,enemy._name.c_str(),to_dodge);
    if ( to_hit > to_dodge ) {
      --enemy._hp_current;
      printf(" ... hit. %d HP left\n",enemy._hp_current);
    }
    else printf(" ... dodge!\n");
    return to_hit > to_dodge;
  }
};

int main(void) {
  srand(0);

  NPC a(0.7,0.5,10);
  NPC b(0.7,0.5,10);

  a._name = std::string("A1");
  b._name = std::string("B2");

  for ( int i = 0 ; i < 10 ; ++i ) {
    a.attack(b);
    b.attack(a);
  }

  return 0;
}
