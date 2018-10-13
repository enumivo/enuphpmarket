#include "ex.hpp"

#include <cmath>
#include <enulib/action.hpp>
#include <enulib/asset.hpp>
#include "enu.token.hpp"

using namespace enumivo;
using namespace std;

void ex::buyPHP(const currency::transfer &transfer) {
  if (transfer.to != _self) {
    return;
  }

  // get ENU balance
  auto enu_balance = enumivo::token(N(enu.token)).
	   get_balance(_self, enumivo::symbol_type(ENU_SYMBOL).name()).amount;

  // check purchase limit, not exceed 0.1% at each time
  auto amount = transfer.quantity.amount;
  enumivo_assert(amount * 1000 <= (enu_balance - amount), "Limit exceeded, should be less than 0.1% of PHP held.");

  // get PHP balance
  auto php_balance = enumivo::token(N(coin)).
	   get_balance(_self, enumivo::symbol_type(PHP_SYMBOL).name()).amount;


  // calculate PHP to buy
  auto php_buy = amount * php_balance / (enu_balance - amount);
  auto fee = php_buy / 500;
  auto php_transfer_amount = php_buy - fee;

  double amt = amount;
  double bal = enu_balance;
  double res = php_balance/10000;
  double tobuy = res*(pow(1+amt/bal,0.5)-1);


  auto to = transfer.from;

  auto quantity = asset(php_transfer_amount, PHP_SYMBOL);

  action(permission_level{_self, N(active)}, N(coin), N(transfer),
         std::make_tuple(_self, to, quantity,
                         std::string("Buy PHP with ENU ")+std::to_string(tobuy)))
      .send();
}

void ex::sellPHP(const currency::transfer &transfer) {
  if (transfer.to != _self) {
    return;
  }

  // get PHP balance
  auto php_balance = enumivo::token(N(coin)).
	   get_balance(_self, enumivo::symbol_type(PHP_SYMBOL).name()).amount;

  // check purchase limit, not exceed 0.1% at each time
  auto amount = transfer.quantity.amount;
  enumivo_assert(amount * 1000 <= (php_balance - amount), "Limit exceeded, should be less than 0.1% of ENU held.");

  // get ENU balance
  auto enu_balance = enumivo::token(N(enu.token)).
	   get_balance(_self, enumivo::symbol_type(ENU_SYMBOL).name()).amount;

  // calculate ENU to transfer
  auto enu_buy = amount * enu_balance / (php_balance - amount);
  auto fee = enu_buy / 500;
  auto enu_transfer_amount = enu_buy - fee;
  
  //auto enu_transfer_amount = enu_balance * ( pow( 1+(amount/php_balance), 0.5 ) -1 );

  auto to = transfer.from;

  auto quantity = asset(enu_transfer_amount, ENU_SYMBOL);

  action(permission_level{_self, N(active)}, N(enu.token), N(transfer),
         std::make_tuple(_self, to, quantity,
                         std::string("Sell PHP for ENU")))
      .send();
}

void ex::apply(account_name contract, action_name act) {
  if (contract == N(enu.token) && act == N(transfer)) {
    auto transfer = unpack_action_data<currency::transfer>();
    enumivo_assert(transfer.quantity.symbol == ENU_SYMBOL,
                 "must pay with ENU");
    buyPHP(transfer);
    return;
  }

  if (contract == N(coin) && act == N(transfer)) {
    auto transfer = unpack_action_data<currency::transfer>();
    enumivo_assert(transfer.quantity.symbol == PHP_SYMBOL,
                 "must pay with PHP");
    sellPHP(transfer);
    return;
  }

  if (contract != _self) return;

}

extern "C" {
[[noreturn]] void apply(uint64_t receiver, uint64_t code, uint64_t action) {
  ex enuphp(receiver);
  enuphp.apply(code, action);
  enumivo_exit(0);
}
}
