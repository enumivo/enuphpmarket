#include <enulib/currency.hpp>
#include <enulib/enu.hpp>
#include <vector>

#define ENU_SYMBOL S(4, ENU)  
#define PHP_SYMBOL S(4, PHP)  

using namespace enumivo;

class ex : public contract {
 public:
  ex(account_name self)
      : contract(self) {}

  void buy(const currency::transfer& transfer);
  void sell(const currency::transfer& transfer);

  void apply(account_name contract, action_name act);

};
