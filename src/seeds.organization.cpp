#include <seeds.organization.hpp>
#include <eosio/system.hpp>


uint64_t organization::get_config(name key) {
    auto citr = config.find(key.value);
    check(citr != config.end(), ("settings: the "+key.to_string()+" parameter has not been initialized").c_str());
    return citr -> value;
}

void organization::check_owner(name organization, name owner) {
    require_auth(owner);
    auto itr = organizations.get(organization.value, "The organization does not exist.");
    check(itr.owner == owner, "Only the organization's owner can do that.");
    check_user(owner);
}

void organization::init_balance(name account) {
    auto itr = sponsors.find(account.value);
    if(itr == sponsors.end()){
        sponsors.emplace(_self, [&](auto & nbalance) {
            nbalance.account = account;
            nbalance.balance = asset(0, seeds_symbol);
        });
    }
}

void organization::check_user(name account) {
    auto uitr = users.find(account.value);
    check(uitr != users.end(), "organisation: no user.");
}

int64_t organization::getregenp(name account) {
    auto itr = users.find(account.value);
    return itr -> reputation; // suposing a 4 decimals reputation allocated in a uint64_t variable
}

uint64_t organization::get_beginning_of_day_in_seconds() {
    auto sec = eosio::current_time_point().sec_since_epoch();
    auto date = eosio::time_point_sec(sec / 86400 * 86400);
    return date.utc_seconds;
}

uint64_t organization::get_size(name id) {
    auto itr = sizes.find(id.value);
    if (itr == sizes.end()) {
        return 0;
    }
    return itr -> size;
}

void organization::increase_size_by_one(name id) {
    auto itr = sizes.find(id.value);
    if (itr != sizes.end()) {
        sizes.modify(itr, _self, [&](auto & s){
            s.size += 1;
        });
    } else {
        sizes.emplace(_self, [&](auto & s){
            s.id = id;
            s.size = 1;
        });
    }
}

void organization::decrease_size_by_one(name id) {
    auto itr = sizes.find(id.value);
    if (itr != sizes.end()) {
        sizes.modify(itr, _self, [&](auto & s){
            if (s.size > 0) {
                s.size -= 1;
            }
        });
    }
}

void organization::deposit(name from, name to, asset quantity, string memo) {
    if(to == _self){
        utils::check_asset(quantity);
        check_user(from);

        init_balance(from);
        init_balance(to);

        auto fitr = sponsors.find(from.value);
        sponsors.modify(fitr, _self, [&](auto & mbalance) {
            mbalance.balance += quantity;
        });

        auto titr = sponsors.find(to.value);
        sponsors.modify(titr, _self, [&](auto & mbalance) {
            mbalance.balance += quantity;
        });
    }
}


// this function is just for testing
/*
ACTION organization::createbalance(name user, asset quantity) {
    
    sponsors.emplace(_self, [&](auto & nbalance) {
        nbalance.account = user;
        nbalance.balance = quantity;
    });
}
*/


ACTION organization::reset() {
    require_auth(_self);

    auto itr = organizations.begin();
    while(itr != organizations.end()) {
        name org = itr -> org_name;
        members_tables members(get_self(), org.value);
        vote_tables votes(get_self(), org.value);

        auto mitr = members.begin();
        while(mitr != members.end()) {
            mitr = members.erase(mitr);
        }

        auto vitr = votes.begin();
        while(vitr != votes.end()){
            vitr = votes.erase(vitr);
        }

        itr = organizations.erase(itr);
    }

    auto aitr = apps.begin();
    while(aitr != apps.end()) {
        dau_tables daus(get_self(), aitr->app_name.value);
        dau_history_tables dau_history(get_self(), aitr->app_name.value);
        auto dauitr = daus.begin();
        while (dauitr != daus.end()) {
            dauitr = daus.erase(dauitr);
        }
        auto dau_history_itr = dau_history.begin();
        while (dau_history_itr != dau_history.end()) {
            dau_history_itr = dau_history.erase(dau_history_itr);
        }
        aitr = apps.erase(aitr);
    }

    auto bitr = sponsors.begin();
    while(bitr != sponsors.end()){
        bitr = sponsors.erase(bitr);
    }

    auto sitr = sizes.begin();
    while (sitr != sizes.end()) {
        sitr = sizes.erase(sitr);
    }

    auto regenitr = regenscores.begin();
    while (regenitr != regenscores.end()) {
        regenitr = regenscores.erase(regenitr);
    }

    auto cbsitr = cbsorgs.begin();
    while (cbsitr != cbsorgs.end()) {
        cbsitr = cbsorgs.erase(cbsitr);
    }

    auto txitr = txporgs.begin();
    while (txitr != txporgs.end()) {
        txitr = txporgs.erase(txitr);
    }
}


ACTION organization::create(name sponsor, name orgaccount, string orgfullname, string publicKey) 
{
    require_auth(sponsor);

    auto bitr = sponsors.find(sponsor.value);
    check(bitr != sponsors.end(), "The sponsor account does not have a balance entry in this contract.");

    auto feeparam = config.get(min_planted.value, "The org.minplant parameter has not been initialized yet.");
    asset quantity(feeparam.value, seeds_symbol);

    check(bitr->balance >= quantity, "The user does not have enough credit to create an organization" + bitr->balance.to_string() + " min: "+quantity.to_string());

    auto orgitr = organizations.find(orgaccount.value);
    check(orgitr == organizations.end(), "This organization already exists.");
    
    auto uitr = users.find(sponsor.value);
    check(uitr != users.end(), "Sponsor is not a Seeds account.");

    create_account(sponsor, orgaccount, orgfullname, publicKey);

    string memo =  "sow "+orgaccount.to_string();

    action(
        permission_level(_self, "active"_n),
        contracts::token,
        "transfer"_n,
        std::make_tuple(_self, contracts::harvest, quantity, memo)
    ).send();


    sponsors.modify(bitr, _self, [&](auto & mbalance) {
        mbalance.balance -= quantity;           
    });

    organizations.emplace(_self, [&](auto & norg) {
        norg.org_name = orgaccount;
        norg.owner = sponsor;
        norg.planted = quantity;
        norg.status = regular_org;
    });

    addmember(orgaccount, sponsor, sponsor, ""_n);
    increase_size_by_one(get_self());
}

void organization::create_account(name sponsor, name orgaccount, string orgfullname, string publicKey) 
{
    action(
        permission_level{contracts::onboarding, "active"_n},
        contracts::onboarding, "onboardorg"_n,
        make_tuple(sponsor, orgaccount, orgfullname, publicKey)
    ).send();
}

ACTION organization::destroy(name organization, name owner) {
    check_owner(organization, owner);

    auto orgitr = organizations.find(organization.value);
    check(orgitr != organizations.end(), "organisation: the organization does not exist.");

    auto bitr = sponsors.find(owner.value);
    sponsors.modify(bitr, _self, [&](auto & mbalance) {
        mbalance.balance += orgitr -> planted;
    });

    members_tables members(get_self(), organization.value);
    auto mitr = members.begin();
    while(mitr != members.end()){
        mitr = members.erase(mitr);
    }
    
    auto org = organizations.find(organization.value);
    organizations.erase(org);

    decrease_size_by_one(get_self());

    // refund(owner, planted); this method could be called if we want to refund as soon as the user destroys an organization
}


ACTION organization::refund(name beneficiary, asset quantity) {
    require_auth(beneficiary);
    
    utils::check_asset(quantity);

    auto itr = sponsors.find(beneficiary.value);
    check(itr != sponsors.end(), "organisation: user has no entry in the balance table.");
    check(itr -> balance >= quantity, "organisation: user has not enough balance.");

    string memo = "refund";

    action(
        permission_level(_self, "active"_n),
        contracts::token,
        "transfer"_n,
        std::make_tuple(_self, beneficiary, quantity, memo)
    ).send();

    auto bitr = sponsors.find(_self.value);
    sponsors.modify(bitr, _self, [&](auto & mbalance) {
        mbalance.balance -= quantity;
    });

    sponsors.modify(itr, _self, [&](auto & mbalance) {
        mbalance.balance -= quantity;
    });
}


ACTION organization::addmember(name organization, name owner, name account, name role) {
    check_owner(organization, owner);
    check_user(account);
    
    members_tables members(get_self(), organization.value);
    members.emplace(_self, [&](auto & nmember) {
        nmember.account = account;
        nmember.role = role;
    });
}


ACTION organization::removemember(name organization, name owner, name account) {
    check_owner(organization, owner);

    auto itr = organizations.find(organization.value);
    check(itr -> owner != account, "Change the organization's owner before removing this account.");

    members_tables members(get_self(), organization.value);
    auto mitr = members.find(account.value);
    members.erase(mitr);
}


ACTION organization::changerole(name organization, name owner, name account, name new_role) {
    check_owner(organization, owner);

    members_tables members(get_self(), organization.value);
    
    auto mitr = members.find(account.value);
    check(mitr != members.end(), "Member does not exist.");

    members.modify(mitr, _self, [&](auto & mmember) {
        mmember.role = new_role;
    });
}


ACTION organization::changeowner(name organization, name owner, name account) {
    check_owner(organization, owner);
    check_user(account);

    auto orgitr = organizations.find(organization.value);

    organizations.modify(orgitr, _self, [&](auto & morg) {
        morg.owner = account;
    });
}

void organization::revert_previous_vote(name organization, name account) {
    vote_tables votes(get_self(), organization.value);

    auto vitr = votes.find(account.value);
    
    if(vitr != votes.end()){
        auto itr = organizations.find(organization.value);
        check(itr != organizations.end(), "organisation does not exist.");
        organizations.modify(itr, _self, [&](auto & morg) {
            morg.regen -= vitr -> regen_points;
        });
        votes.erase(vitr);
        decrease_size_by_one(organization);
    }
}

void organization::vote(name organization, name account, int64_t regen) {
    vote_tables votes(get_self(), organization.value);

    auto itr = organizations.find(organization.value);
    check(itr != organizations.end(), "organisation does not exist.");
    
    organizations.modify(itr, _self, [&](auto & morg) {
        morg.regen += regen;
    });

    votes.emplace(_self, [&](auto & nvote) {
        nvote.account = account;
        nvote.timestamp = eosio::current_time_point().sec_since_epoch();
        nvote.regen_points = regen;
    });

    increase_size_by_one(organization);
}


ACTION organization::addregen(name organization, name account, uint64_t amount) {
    require_auth(account);
    check_user(account);

    uint64_t maxAmount = config.get(name("rgen.maxadd").value, "The parameter rgen.maxadd has not been initialized yet").value;
    amount = std::min(amount, maxAmount);

    revert_previous_vote(organization, account);
    vote(organization, account, amount * getregenp(account));
}


ACTION organization::subregen(name organization, name account, uint64_t amount) {
    require_auth(account);
    check_user(account);

    uint64_t minAmount = config.get(name("rgen.minsub").value, "The parameter rgen.minsub has not been initialized yet").value;
    amount = std::min(amount, minAmount);

    revert_previous_vote(organization, account);
    vote(organization, account, -1 * amount * getregenp(account));
}

int64_t organization::calculate_median_regen_points(name orgname) {
    int64_t median_regen = 0;
    uint64_t num_votes = get_size(orgname);
    uint64_t position = num_votes / 2;

    vote_tables votes(get_self(), orgname.value);
    auto votes_by_regen = votes.get_index<"byregen"_n>();

    auto sanity_check = votes_by_regen.begin();
    while (sanity_check != votes_by_regen.end()) {
        print("\nValue:", sanity_check -> regen_points);
        sanity_check++;
    }

    if (num_votes % 2 == 0) {
        auto value1 = votes_by_regen.begin();
        std::advance(value1, position - 1);
        auto value2 = votes_by_regen.begin();
        std::advance(value2, position);
        median_regen = (value1 -> regen_points + value2 -> regen_points) / 2;
        print("\nLook, 2 values:\n", "value1:", value1->regen_points, "\n", "value2:", value2->regen_points);
    } else {
        auto value1 = votes_by_regen.begin();
        std::advance(value1, position);
        median_regen = value1 -> regen_points;
        print("\nLook, 1 value:\n", "value1:", value1->regen_points);
    }

    print("\n----------------------\n");

    return median_regen;
}

ACTION organization::calcmregens() {
    require_auth(get_self());
    auto batch_size = config.get(name("batchsize").value, "The batchsize parameter has not been initialized yet");
    calcmregen((uint64_t)0, batch_size.value);
}

ACTION organization::calcmregen(uint64_t start, uint64_t chunksize) {
    require_auth(get_self());

    check(chunksize > 0, "chunk size must be > 0");
    
    auto itr_org = start == 0 ? organizations.begin() : organizations.lower_bound(start);
    int64_t min_regen = (int64_t)config.get(name("regen.min").value, "The regen.min parameter has not been initialized yet").value;
    uint64_t count = 0;

    while (itr_org != organizations.end() && count < chunksize) {
        auto itr_regen = regenscores.find((itr_org -> org_name).value);
        print("&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&");
        print("\norg:", itr_org -> org_name, "org regen:", itr_org -> regen, ", min_regen:", min_regen);
        if (itr_org -> regen >= min_regen) {
            int64_t median_regen_points = calculate_median_regen_points(itr_org -> org_name);
            // if (itr_regen != regenscores.end()) {
            //     regenscores.modify(itr_regen, _self, [&](auto & rs){
            //         rs.regen_median = median_regen_points;
            //     });
            // } else {
            //     regenscores.emplace(_self, [&](auto & rs){
            //         rs.org_name = itr_org -> org_name;
            //         rs.regen_median = median_regen_points;
            //         rs.rank = 0;
            //     });
            //     increase_size_by_one(regen_score_size);
            // }
        } else {
            // here it could lose the regen status if the org had it
            if (itr_regen != regenscores.end()) {
                regenscores.erase(itr_regen);
                decrease_size_by_one(regen_score_size);
            }
        }
        itr_org++;
        count++;
    }

    if (itr_org != organizations.end()) {
        action next_execution(
            permission_level("active"_n, get_self()),
            get_self(),
            "calcmregen"_n,
            std::make_tuple((itr_org -> org_name).value, chunksize)
        );
        transaction tx;
        tx.actions.emplace_back(next_execution);
        tx.delay_sec = 1;
        tx.send(regen_median.value, _self);
    }
}

ACTION organization::rankregens() {
    auto batch_size = config.get(name("batchsize").value, "The batchsize parameter has not been initialized yet");
    rankregen((uint64_t)0, (uint64_t)0, batch_size.value);
}

ACTION organization::rankregen(uint64_t start, uint64_t chunk, uint64_t chunksize) {
    require_auth(get_self());

    check(chunksize > 0, "chunk size must be > 0");

    uint64_t total = get_size(regen_score_size);
    if (total == 0) return;

    uint64_t current = chunk * chunksize;
    auto regen_score_by_median_regen = regenscores.get_index<"byregenmdian"_n>();
    auto rsitr = start == 0 ? regen_score_by_median_regen.begin() : regen_score_by_median_regen.lower_bound(start);
    uint64_t count = 0;

    while (rsitr != regen_score_by_median_regen.end() && count < chunksize) {

        uint64_t rank = (current * 100) / total;

        regen_score_by_median_regen.modify(rsitr, _self, [&](auto & item) {
            item.rank = rank;
        });

        current++;
        count++;
        rsitr++;
    }

    if (rsitr != regen_score_by_median_regen.end()) {
        action next_execution(
            permission_level("active"_n, get_self()),
            get_self(),
            "rankregen"_n,
            std::make_tuple((rsitr -> org_name).value, chunk + 1, chunksize)
        );
        transaction tx;
        tx.actions.emplace_back(next_execution);
        tx.delay_sec = 1;
        tx.send(regen_score_size.value, _self);
    }
}

ACTION organization::addcbpoints(name organization, uint32_t cbscore) {
    require_auth(get_self());
    auto itr_cbs = cbsorgs.find(organization.value);
    if (itr_cbs != cbsorgs.end()) {
        cbsorgs.modify(itr_cbs, _self, [&](auto & cbs){
            cbs.community_building_score += cbscore;
        });
    } else {
        cbsorgs.emplace(_self, [&](auto & cbs){
            cbs.org_name = organization;
            cbs.community_building_score = cbscore;
        });
        increase_size_by_one(cb_score_size);
    }
}

ACTION organization::subcbpoints(name organization, uint32_t cbscore) {
    require_auth(get_self());
    auto itr_cbs = cbsorgs.find(organization.value);
    if (itr_cbs != cbsorgs.end()) {
        if (itr_cbs -> community_building_score >= cbscore) {
            cbsorgs.modify(itr_cbs, _self, [&](auto & cbs){
                cbs.community_building_score -= cbscore;
            });
        } else {
            cbsorgs.erase(itr_cbs);
            decrease_size_by_one(cb_score_size);
        }
    }
}

ACTION organization::rankcbsorgs() {
    auto batch_size = config.get(name("batchsize").value, "The batchsize parameter has not been initialized yet");
    rankcbsorg((uint64_t)0, (uint64_t)0, batch_size.value);
}

ACTION organization::rankcbsorg(uint64_t start, uint64_t chunk, uint64_t chunksize) {
    require_auth(get_self());

    check(chunksize > 0, "chunk size must be > 0");
    
    uint64_t total = get_size(cb_score_size);

    print("\nrankcbsorgrankcbsorgrankcbsorgrankcbsorgrankcbsorgrankcbsorg\n");
    print("\ntotal:", total);

    if (total == 0) return;

    uint64_t current = chunk * chunksize;
    auto cbs_by_points = cbsorgs.get_index<"bycbs"_n>();
    auto cbsitr = start == 0 ? cbs_by_points.begin() : cbs_by_points.lower_bound(start);
    uint64_t count = 0;

    while (cbsitr != cbs_by_points.end() && count < chunksize) {

        uint64_t rank = (current * 100) / total;

        cbs_by_points.modify(cbsitr, _self, [&](auto & item) {
            item.rank = rank;
        });

        current++;
        count++;
        cbsitr++;
    }

    if (cbsitr != cbs_by_points.end()) {
        uint64_t next_value = (cbsitr -> org_name).value;
        action next_execution(
            permission_level("active"_n, get_self()),
            get_self(),
            "rankcbsorg"_n,
            std::make_tuple(next_value, chunk + 1, chunksize)
        );
        transaction tx;
        tx.actions.emplace_back(next_execution);
        tx.delay_sec = 1;
        tx.send(cb_score_size.value, _self);
    }

    print("\nrankcbsorgrankcbsorgrankcbsorgrankcbsorgrankcbsorgrankcbsorg\n");
}

// void accounts::makecitizen(name user)
// {
//     check_can_make_citizen(user);
    
//     auto new_status = name("citizen");

//     updatestatus(user, new_status);

//     rewards(user, new_status);
    
//     history_add_citizen(user);
// }

uint64_t organization::get_regen_score(name organization) {
    auto ritr = regenscores.find(organization.value);
    if (ritr == regenscores.end()) {
        return 0;
    }
    return ritr -> rank;
}

uint64_t organization::count_refs(name organization, uint32_t check_num_residents) {
    auto refs_by_referrer = refs.get_index<"byreferrer"_n>();
    if (check_num_residents == 0) {
      return std::distance(refs_by_referrer.lower_bound(organization.value), refs_by_referrer.upper_bound(organization.value));
    } else {
      uint64_t count = 0;
      int residents = 0;
      auto ritr = refs_by_referrer.lower_bound(organization.value);
      while (ritr != refs_by_referrer.end() && ritr->referrer == organization) {
        auto uitr = users.find(ritr->invited.value);
        if (uitr != users.end()) {
          if (uitr->status == "resident"_n || uitr->status == "citizen"_n) {
            residents++;
          }
        }
        ritr++;
        count++;
      }
      check(residents >= check_num_residents, "organization has not referred enough residents or citizens: "+std::to_string(residents));
      return count;
    }
}

void organization::check_can_make_regen(name organization) {
    auto oitr = organizations.find(organization.value);
    check(oitr != organizations.end(), "the organization does not exist");
    check(oitr->status == reputable_org, "the organization is not reputable");

    auto bitr = balances.find(organization.value);

    uint64_t planted_min = get_config(name("rgen.minplnt"));
    uint64_t regen_min_rank = get_config(name("rgen.minrank"));
    uint64_t min_invited = get_config(name("rgen.refrred"));
    uint64_t min_residents_invited = get_config(name("rgen.resref"));

    uint64_t invited_users_number = count_refs(organization, min_residents_invited);
    uint64_t regen_score = get_regen_score(organization);

    // TODO: check transations with reputable/regen orgs &/or citizens

    check(bitr -> planted.amount >= planted_min, "organization has less than the required amount of seeds planted");
    check(regen_score >= regen_min_rank, "organization has less than the required regenerative score");
    check(invited_users_number >= min_invited, "organization has less than required referrals. required: " + 
        std::to_string(min_invited) + " actual: " + std::to_string(invited_users_number));

}

void organization::update_status(name organization, name status) {
  auto oitr = organizations.find(organization.value);

  check(oitr != organizations.end(), "the organization does not exist");
  check(status == regenerative_org || status == reputable_org, "organization invalid status");

  organizations.modify(oitr, _self, [&](auto& org) {
    org.status = status;
  });
}

ACTION organization::makeregen(name organization) {
    check_can_make_regen(organization);
    update_status(organization, regenerative_org);
    // rewards?
    // history_add_citizen();
}

// ==================================================================================== //
// ==================================================================================== //
// ==================================================================================== //

// Calculate Transaction Points for a single organization
// Returns count of iterations
uint32_t organization::calc_transaction_points(name organization) {
  auto three_moon_cycles = utils::moon_cycle * 3;
  auto now = eosio::current_time_point().sec_since_epoch();
  auto cutoffdate = now - three_moon_cycles;

  print("\n[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[]\n");
  print("Calculate for:", organization);

  // get all transactions for this organization
  transaction_tables transactions(contracts::history, organization.value);

  auto transactions_by_to = transactions.get_index<"byto"_n>();
  auto tx_to_itr = transactions_by_to.rbegin();

  double result = 0;

  uint64_t max_quantity = 1777; // get this from settings // TODO verify this number
  uint64_t max_number_of_transactions = 26;

  name      current_to = name("");
  uint64_t  current_num = 0;
  double    current_rep_multiplier = 0.0;

  uint64_t  count = 0;
  uint64_t  limit = 200;
    
  //print("start " + organization.to_string());

  while(tx_to_itr != transactions_by_to.rend() && count < limit) {

    print("\nfrom:", organization, " to:", tx_to_itr -> to, "\n");

    if (tx_to_itr->timestamp < cutoffdate) {
      //print("date trigger ");

      // remove old transactions
      //tx_to_itr = transactions_by_to.erase(tx_to_itr);
      
      //auto it = transactions_by_to.erase(--tx_to_itr.base());// TODO add test for this
      //tx_to_itr = std::reverse_iterator(it);            
    } else {
      //print("update to ");

      // update "to"
      if (current_to != tx_to_itr->to) {
        current_to = tx_to_itr->to;
        current_num = 0;
        current_rep_multiplier = utils::get_rep_multiplier(current_to);
        print("\ncurrent rep multiplier:", current_rep_multiplier, "\n");
      } else {
        current_num++;
      }

      //print("iterating over "+std::to_string(tx_to_itr->id));

      if (current_num < max_number_of_transactions) {
        uint64_t volume = tx_to_itr->quantity.amount;

      //print("volume "+std::to_string(volume));

        // limit max volume
        if (volume > max_quantity * 10000) {
              //print("max limit "+std::to_string(max_quantity * 10000));
          volume = max_quantity * 10000;
        }


        // multiply by receiver reputation
        double points = (double(volume) / 10000.0) * current_rep_multiplier;
        print("\npoints:", points, ", volume:", volume, "\n");
        
        //print("tx points "+std::to_string(points));

        result += points;

      } 

    }
    tx_to_itr++;
    count++;
  }

  //print("set result "+std::to_string(result));

  // use ceil function so each schore is counted if it is > 0
    
  // DEBUG
  // if (result == 0) {
  //   result = 33.0;
  // }
  // enter into transaction points table
  auto titr = txporgs.find(organization.value);
  uint64_t points = ceil(result);

  print("\n", "points:", points);

  if (titr == txporgs.end()) {
    if (points > 0) {
      txporgs.emplace(_self, [&](auto& entry) {
        entry.org_name = organization;
        entry.points = points;
      });
      increase_size_by_one(tx_score_size);
    }
  } else {
    if (points > 0) {
      txporgs.modify(titr, _self, [&](auto& entry) {
        entry.points = points; 
      });
    } else {
      txporgs.erase(titr);
      decrease_size_by_one(tx_score_size);
    }
  }

  print("\n[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[]\n");

  return count;

}

ACTION organization::calctrxpts() {
    auto batch_size = config.get(name("batchsize").value, "The batchsize parameter has not been initialized yet");
    calctrxpt(0, 0, batch_size.value);
}

ACTION organization::calctrxpt(uint64_t start, uint64_t chunk, uint64_t chunksize) {
  require_auth(get_self());

  check(chunksize > 0, "chunk size must be > 0");

  uint64_t total = get_size(tx_score_size);
  auto orgitr = start == 0 ? organizations.begin() : organizations.lower_bound(start);
  uint64_t count = 0;

  while (orgitr != organizations.end() && count < chunksize) {
    uint32_t num = calc_transaction_points(orgitr -> org_name);
    count += 1 + num;
    orgitr++;
  }

  if (orgitr != organizations.end()) {
    uint64_t next_value = (orgitr -> org_name).value;
    action next_execution(
        permission_level{get_self(), "active"_n},
        get_self(),
        "calctrxpt"_n,
        std::make_tuple(next_value, chunk + 1, chunksize)
    );

    transaction tx;
    tx.actions.emplace_back(next_execution);
    tx.delay_sec = 1;
    tx.send(tx_score_size.value, _self);
  }
}

ACTION organization::ranktxs() {
    auto batch_size = config.get(name("batchsize").value, "The batchsize parameter has not been initialized yet");
    ranktx(0, 0, batch_size.value);
}

ACTION organization::ranktx(uint64_t start, uint64_t chunk, uint64_t chunksize) {
    require_auth(get_self());

    check(chunksize > 0, "chunk size must be > 0");
    
    print("\nranktxranktxranktxranktxranktxranktxranktxranktxranktx\n");

    uint64_t total = get_size(tx_score_size);
    if (total == 0) return;

    print("\ntotal:", total);

    uint64_t current = chunk * chunksize;
    auto tx_by_points = txporgs.get_index<"bypoints"_n>();
    auto txitr = start == 0 ? tx_by_points.begin() : tx_by_points.lower_bound(start);
    uint64_t count = 0;

    while (txitr != tx_by_points.end() && count < chunksize) {

        uint64_t rank = (current * 100) / total;

        print("\nrank:", rank, ", current:", current, "\n");

        tx_by_points.modify(txitr, _self, [&](auto & item) {
            item.rank = rank;
        });

        current++;
        count++;
        txitr++;
    }

    print("\nranktxranktxranktxranktxranktxranktxranktxranktxranktx\n");

    if (txitr != tx_by_points.end()) {
        uint64_t next_value = (txitr -> org_name).value;
        action next_execution(
            permission_level("active"_n, get_self()),
            get_self(),
            "rankcbsorg"_n,
            std::make_tuple(next_value, chunk + 1, chunksize)
        );
        transaction tx;
        tx.actions.emplace_back(next_execution);
        tx.delay_sec = 1;
        tx.send(cb_score_size.value, _self);
    }
}

// ==================================================================================== //
// ==================================================================================== //
// ==================================================================================== //

ACTION organization::registerapp(name owner, name organization, name appname, string applongname) {
    require_auth(owner);
    check_owner(organization, owner);

    auto orgitr = organizations.find(organization.value);
    check(orgitr != organizations.end(), "This organization does not exist.");

    auto appitr = apps.find(appname.value);
    check(appitr == apps.end(), "This application already exists.");

    apps.emplace(_self, [&](auto & app){
        app.app_name = appname;
        app.org_name = organization;
        app.app_long_name = applongname;
        app.is_banned = false;
        app.number_of_uses = 0;
    });
}

ACTION organization::banapp(name appname) {
    require_auth(get_self());

    auto appitr = apps.find(appname.value);
    check(appitr != apps.end(), "This application does not exists.");
    
    apps.modify(appitr, _self, [&](auto & app){
        app.is_banned = true;
    });
}

ACTION organization::appuse(name appname, name account) {
    require_auth(account);
    check_user(account);

    auto appitr = apps.find(appname.value);
    check(appitr != apps.end(), "This application does not exists.");
    check(!(appitr -> is_banned), "Can not use a banned app.");
    
    dau_tables daus(get_self(), appname.value);

    uint64_t today_timestamp = get_beginning_of_day_in_seconds();
    
    auto dauitr = daus.find(account.value);
    if (dauitr != daus.end()) {
        if (dauitr -> date == today_timestamp) {
            daus.modify(dauitr, _self, [&](auto & dau){
                dau.number_app_uses += 1;
            });
        } else {
            if (dauitr -> number_app_uses != 0) {
                dau_history_tables dau_history(get_self(), appname.value); 
                dau_history.emplace(_self, [&](auto & dau_h){
                    dau_h.dau_history_id = dau_history.available_primary_key();
                    dau_h.account = dauitr -> account;
                    dau_h.date = dauitr -> date;
                    dau_h.number_app_uses = dauitr -> number_app_uses;
                });
            }
            daus.modify(dauitr, _self, [&](auto & dau){
                dau.date = today_timestamp;
                dau.number_app_uses = 1;
            }); 
        }
    } else {
        daus.emplace(_self, [&](auto & dau){
            dau.account = account;
            dau.date = today_timestamp;
            dau.number_app_uses = 1;
        });
    }

    apps.modify(appitr, _self, [&](auto & app){
        app.number_of_uses += 1;
    });
}

ACTION organization::cleandaus () {
    require_auth(get_self());

    uint64_t today_timestamp = get_beginning_of_day_in_seconds();

    auto appitr = apps.begin();
    while (appitr != apps.end()) {
        if (appitr -> is_banned) {
            appitr++;
            continue; 
        }
        action clean_dau_action(
            permission_level{get_self(), "active"_n},
            get_self(),
            "cleandau"_n,
            std::make_tuple(appitr -> app_name, today_timestamp, (uint64_t)0)
        );

        transaction tx;
        tx.actions.emplace_back(clean_dau_action);
        tx.delay_sec = 1;
        tx.send((appitr -> app_name).value + 1, _self);

        appitr++;
    }
}

ACTION organization::cleandau (name appname, uint64_t todaytimestamp, uint64_t start) {
    require_auth(get_self());

    auto appitr = apps.get(appname.value, "This application does not exist.");
    if (appitr.is_banned) { return; }

    auto batch_size = config.get(name("batchsize").value, "The batchsize parameter has not been initialized yet.").value;

    dau_tables daus(get_self(), appname.value);
    dau_history_tables dau_history(get_self(), appname.value); 

    auto dauitr = start == 0 ? daus.begin() : daus.lower_bound(start);
    uint64_t count = 0;

    while (dauitr != daus.end() && count < batch_size) {
        if (dauitr -> date != todaytimestamp) {
            dau_history.emplace(_self, [&](auto & dau_h){
                dau_h.dau_history_id = dau_history.available_primary_key();
                dau_h.account = dauitr -> account;
                dau_h.date = dauitr -> date;
                dau_h.number_app_uses = dauitr -> number_app_uses;
            });
            daus.modify(dauitr, _self, [&](auto & dau){
                dau.date = todaytimestamp;
                dau.number_app_uses = 0;
            });
        }
        count++;
        dauitr++;
    }

    if (dauitr != daus.end()) {
        action clean_dau_action(
            permission_level{get_self(), "active"_n},
            get_self(),
            "cleandau"_n,
            std::make_tuple(appname, todaytimestamp, (dauitr -> account).value)
        );

        transaction tx;
        tx.actions.emplace_back(clean_dau_action);
        tx.delay_sec = 1;
        tx.send((appitr.app_name).value + 1, _self);
    }
}

