#include <eosiolib/eosio.hpp>

using namespace eosio;
using std::string;

CONTRACT policy : public contract {
  public:
    using contract::contract;
    policy(name receiver, name code, datastream<const char*> ds)
      : contract(receiver, code, ds)
      {}

    ACTION create(name account, string uuid, string signature, string policy);

    ACTION update(name account, string uuid, string signature, string policy);
  private:
    TABLE policy_table {
      name account;
      string uuid;
      string signature;
      string policy;
      uint64_t primary_key()const { return account.value; }
    };

    typedef eosio::multi_index<"policies"_n, policy_table> policy_tables;
};

EOSIO_DISPATCH(policy, (create)(update));