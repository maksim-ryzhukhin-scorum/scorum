#include <scorum/protocol/scorum_operations.hpp>
#include <fc/io/json.hpp>

#include <boost/rational.hpp>

#include <locale>

#include <scorum/protocol/atomicswap_helper.hpp>
#include <scorum/protocol/betting/invariants_validation.hpp>

namespace scorum {
namespace protocol {

bool inline is_asset_type(asset asset, asset_symbol_type symbol)
{
    return asset.symbol() == symbol;
}

template <class T> bool is_unique(const std::vector<T>& input)
{
    std::vector<T> data(input);

    sort(data.begin(), data.end());
    return adjacent_find(data.begin(), data.end()) == data.end();
}

void account_create_operation::validate() const
{
    validate_account_name(new_account_name);
    FC_ASSERT(is_asset_type(fee, SCORUM_SYMBOL), "Account creation fee must be SCR");
    owner.validate();
    active.validate();

    validate_json_metadata(json_metadata);

    FC_ASSERT(fee >= asset(0, SCORUM_SYMBOL), "Account creation fee cannot be negative");
}

void account_create_with_delegation_operation::validate() const
{
    validate_account_name(new_account_name);
    validate_account_name(creator);
    FC_ASSERT(is_asset_type(fee, SCORUM_SYMBOL), "Account creation fee must be SCR");
    FC_ASSERT(is_asset_type(delegation, SP_SYMBOL), "Delegation must be SP");

    owner.validate();
    active.validate();
    posting.validate();

    validate_json_metadata(json_metadata);

    FC_ASSERT(fee >= asset(0, SCORUM_SYMBOL), "Account creation fee cannot be negative");
    FC_ASSERT(delegation >= asset(0, SP_SYMBOL), "Delegation cannot be negative");
}

void account_create_by_committee_operation::validate() const
{
    validate_account_name(creator);
    validate_account_name(new_account_name);

    owner.validate();
    active.validate();
    posting.validate();

    validate_json_metadata(json_metadata);
}

void account_update_operation::validate() const
{
    validate_account_name(account);

    if (owner)
        owner->validate();
    if (active)
        active->validate();
    if (posting)
        posting->validate();

    validate_json_metadata(json_metadata);
}

void comment_operation::validate() const
{
    FC_ASSERT(title.size() < 256, "Title larger than size limit");
    FC_ASSERT(fc::is_utf8(title), "Title not formatted in UTF8");
    FC_ASSERT(body.size() > 0, "Body is empty");
    FC_ASSERT(fc::is_utf8(body), "Body not formatted in UTF8");

    if (parent_author.size())
        validate_account_name(parent_author);
    validate_account_name(author);
    validate_permlink(parent_permlink);
    validate_permlink(permlink);

    validate_json_metadata(json_metadata);
}

void comment_options_operation::validate() const
{
    validate_account_name(author);
    FC_ASSERT(max_accepted_payout.symbol() == SCORUM_SYMBOL, "Max accepted payout must be in SCR");
    FC_ASSERT(max_accepted_payout.amount.value >= 0, "Cannot accept less than 0 payout");
    validate_permlink(permlink);
    for (auto& e : extensions)
    {
        e.get<comment_payout_beneficiaries>().validate();
    }
}

void delete_comment_operation::validate() const
{
    validate_permlink(permlink);
    validate_account_name(author);
}

void prove_authority_operation::validate() const
{
    validate_account_name(challenged);
}

void vote_operation::validate() const
{
    validate_account_name(voter);
    validate_account_name(author);
    validate_permlink(permlink);
}

void transfer_operation::validate() const
{
    try
    {
        validate_account_name(from);
        validate_account_name(to);
        FC_ASSERT(amount.symbol() != SP_SYMBOL, "transferring of Scorum Power (STMP) is not allowed.");
        FC_ASSERT(amount.amount > 0, "Cannot transfer a negative amount (aka: stealing)");
        FC_ASSERT(memo.size() < SCORUM_MAX_MEMO_SIZE, "Memo is too large");
        FC_ASSERT(fc::is_utf8(memo), "Memo is not UTF8");
    }
    FC_CAPTURE_AND_RETHROW((*this))
}

void transfer_to_scorumpower_operation::validate() const
{
    validate_account_name(from);
    FC_ASSERT(is_asset_type(amount, SCORUM_SYMBOL), "Amount must be SCR");
    if (to != account_name_type())
        validate_account_name(to);
    FC_ASSERT(amount > asset(0, SCORUM_SYMBOL), "Must transfer a nonzero amount");
}

void withdraw_scorumpower_operation::validate() const
{
    validate_account_name(account);
    FC_ASSERT(is_asset_type(scorumpower, SP_SYMBOL), "Amount must be SP");
    FC_ASSERT(scorumpower.amount >= 0, "Can't withdraw negative amount");
}

void set_withdraw_scorumpower_route_to_account_operation::validate() const
{
    validate_account_name(from_account);
    validate_account_name(to_account);
    FC_ASSERT(0 <= percent && percent <= SCORUM_100_PERCENT, "Percent must be valid scorum percent");
}

void set_withdraw_scorumpower_route_to_dev_pool_operation::validate() const
{
    validate_account_name(from_account);
    FC_ASSERT(0 <= percent && percent <= SCORUM_100_PERCENT, "Percent must be valid scorum percent");
}

void witness_update_operation::validate() const
{
    validate_account_name(owner);
    FC_ASSERT(url.size() > 0, "URL size must be greater than 0");
    FC_ASSERT(fc::is_utf8(url), "URL is not valid UTF8");
    proposed_chain_props.validate();
}

void account_witness_vote_operation::validate() const
{
    validate_account_name(account);
    validate_account_name(witness);
}

void account_witness_proxy_operation::validate() const
{
    validate_account_name(account);
    if (proxy.size())
        validate_account_name(proxy);
    FC_ASSERT(proxy != account, "Cannot proxy to self");
}

void escrow_transfer_operation::validate() const
{
    validate_account_name(from);
    validate_account_name(to);
    validate_account_name(agent);
    FC_ASSERT(fee.amount >= 0, "fee cannot be negative");
    FC_ASSERT(scorum_amount.amount > 0, "scorum amount cannot be negative");
    FC_ASSERT(from != agent && to != agent, "agent must be a third party");
    FC_ASSERT(fee.symbol() == SCORUM_SYMBOL, "fee must be SCR");
    FC_ASSERT(scorum_amount.symbol() == SCORUM_SYMBOL, "scorum amount must contain SCR");
    FC_ASSERT(ratification_deadline < escrow_expiration, "ratification deadline must be before escrow expiration");
    if (json_meta.size() > 0)
    {
        FC_ASSERT(fc::is_utf8(json_meta), "JSON Metadata not formatted in UTF8");
        FC_ASSERT(fc::json::is_valid(json_meta), "JSON Metadata not valid JSON");
    }
}

void escrow_approve_operation::validate() const
{
    validate_account_name(from);
    validate_account_name(to);
    validate_account_name(agent);
    validate_account_name(who);
    FC_ASSERT(who == to || who == agent, "to or agent must approve escrow");
}

void escrow_dispute_operation::validate() const
{
    validate_account_name(from);
    validate_account_name(to);
    validate_account_name(agent);
    validate_account_name(who);
    FC_ASSERT(who == from || who == to, "who must be from or to");
}

void escrow_release_operation::validate() const
{
    validate_account_name(from);
    validate_account_name(to);
    validate_account_name(agent);
    validate_account_name(who);
    validate_account_name(receiver);
    FC_ASSERT(who == from || who == to || who == agent, "who must be from or to or agent");
    FC_ASSERT(receiver == from || receiver == to, "receiver must be from or to");
    FC_ASSERT(scorum_amount.amount >= 0, "scorum amount cannot be negative");
    FC_ASSERT(scorum_amount.symbol() == SCORUM_SYMBOL, "scorum amount must contain SCR");
}

void request_account_recovery_operation::validate() const
{
    validate_account_name(recovery_account);
    validate_account_name(account_to_recover);
    new_owner_authority.validate();
}

void recover_account_operation::validate() const
{
    validate_account_name(account_to_recover);
    FC_ASSERT(!(new_owner_authority == recent_owner_authority),
              "Cannot set new owner authority to the recent owner authority");
    FC_ASSERT(!new_owner_authority.is_impossible(), "new owner authority cannot be impossible");
    FC_ASSERT(!recent_owner_authority.is_impossible(), "recent owner authority cannot be impossible");
    FC_ASSERT(new_owner_authority.weight_threshold, "new owner authority cannot be trivial");
    new_owner_authority.validate();
    recent_owner_authority.validate();
}

void change_recovery_account_operation::validate() const
{
    validate_account_name(account_to_recover);
    validate_account_name(new_recovery_account);
}

void decline_voting_rights_operation::validate() const
{
    validate_account_name(account);
}

void delegate_scorumpower_operation::validate() const
{
    validate_account_name(delegator);
    validate_account_name(delegatee);
    FC_ASSERT(delegator != delegatee, "You cannot delegate SP to yourself");
    FC_ASSERT(is_asset_type(scorumpower, SP_SYMBOL), "Delegation must be SP");
    FC_ASSERT(scorumpower >= asset(0, SP_SYMBOL), "Delegation cannot be negative");
}

void delegate_sp_from_reg_pool_operation::validate() const
{
    validate_account_name(reg_committee_member);
    validate_account_name(delegatee);
    FC_ASSERT(reg_committee_member != delegatee, "You cannot delegate SP to yourself");
    FC_ASSERT(is_asset_type(scorumpower, SP_SYMBOL), "Delegation must be SP");
    FC_ASSERT(scorumpower.amount >= 0u, "Delegation cannot be negative");
    auto max_delegation = SCORUM_CREATE_ACCOUNT_REG_COMMITTEE_DELEGATION_MAX;
    FC_ASSERT(scorumpower <= max_delegation, "Delegation cannot be more than ${0}", ("0", max_delegation));
}

void create_budget_operation::validate() const
{
    validate_account_name(owner);
    validate_json_metadata(json_metadata);
    FC_ASSERT(is_asset_type(balance, SCORUM_SYMBOL), "Balance must be SCR");
    FC_ASSERT(balance > asset(0, SCORUM_SYMBOL), "Balance must be positive");
    FC_ASSERT(start <= deadline, "Deadline time must be greater or equal then start time");
}

void update_budget_operation::validate() const
{
    validate_account_name(owner);
    validate_json_metadata(json_metadata);
}

void close_budget_operation::validate() const
{
    validate_account_name(owner);
}

void atomicswap_initiate_operation::validate() const
{
    validate_account_name(owner);
    validate_account_name(recipient);
    FC_ASSERT(is_asset_type(amount, SCORUM_SYMBOL), "Amount must be SCR");
    FC_ASSERT(amount > asset(0, SCORUM_SYMBOL), "Amount must be positive");
    atomicswap::validate_contract_metadata(metadata);
    atomicswap::validate_secret_hash(secret_hash);
}

void atomicswap_redeem_operation::validate() const
{
    validate_account_name(from);
    validate_account_name(to);
    atomicswap::validate_secret(secret);
}

void atomicswap_refund_operation::validate() const
{
    validate_account_name(participant);
    validate_account_name(initiator);
    atomicswap::validate_secret_hash(secret_hash);
}

void close_budget_by_advertising_moderator_operation::validate() const
{
    validate_account_name(moderator);
}

void proposal_vote_operation::validate() const
{
    validate_account_name(voting_account);
}

void proposal_create_operation::validate() const
{
    validate_account_name(creator);

    operation_validate(operation);
}

void create_game_operation::validate() const
{
    validate_account_name(moderator);

    validate_json_metadata(json_metadata);

    fc::flat_set<market_type> set_of_markets(markets.begin(), markets.end());

    FC_ASSERT(set_of_markets.size() == markets.size(), "You provided duplicates in market list.",
              ("input_markets", markets) //
              ("set_of_markets", set_of_markets));

    validate_game(game, set_of_markets);
    validate_markets(set_of_markets);
}

void cancel_game_operation::validate() const
{
    validate_account_name(moderator);
}

void update_game_markets_operation::validate() const
{
    validate_account_name(moderator);
}

void update_game_start_time_operation::validate() const
{
    validate_account_name(moderator);
}

void post_game_results_operation::validate() const
{
    validate_account_name(moderator);

    const fc::flat_set<wincase_type> set_of_wincases(wincases.begin(), wincases.end());

    FC_ASSERT(set_of_wincases.size() == wincases.size(), "You provided duplicates in wincases list.",
              ("input_markets", wincases) //
              ("set_of_markets", set_of_wincases));

    validate_wincases(set_of_wincases);
}

void post_bet_operation::validate() const
{
    validate_account_name(better);
    validate_wincase(wincase);

    FC_ASSERT(is_asset_type(stake, SCORUM_SYMBOL), "Stake must be SCR");
    FC_ASSERT(stake >= SCORUM_MIN_BET_STAKE, "Stake must be greater  or equal then ${s}", ("s", SCORUM_MIN_BET_STAKE));
    FC_ASSERT(odds.numerator > 0, "odds numerator must be greater then zero");
    FC_ASSERT(odds.denominator > 0, "odds denominator must be greater then zero");
    FC_ASSERT(odds.numerator > odds.denominator, "odds must be greater then one");

    const auto min = SCORUM_MIN_ODDS.base();
    const auto max = SCORUM_MIN_ODDS.inverted();

    boost::rational<odds_value_type> bet_odds(odds.numerator, odds.denominator);
    boost::rational<odds_value_type> min_odds(min.numerator, min.denominator);
    boost::rational<odds_value_type> max_odds(max.numerator, max.denominator);

    // clang-format off
    FC_ASSERT(bet_odds <= max_odds, "Invalid odds value",
              ("numerator", odds.numerator)
              ("denominator", odds.denominator)
              ("min_odds", protocol::odds(min).to_string())
              ("max_odds", protocol::odds(max).to_string()));

    FC_ASSERT(bet_odds >= min_odds, "Invalid odds value",
              ("numerator", odds.numerator)
              ("denominator", odds.denominator)
              ("min_odds", protocol::odds(min).to_string())
              ("max_odds", protocol::odds(max).to_string()));
    // clang-format on
}

void cancel_pending_bets_operation::validate() const
{
    FC_ASSERT(bet_uuids.size() > 0, "List of bets is empty.");
    FC_ASSERT(is_unique<uuid_type>(bet_uuids), "You provided duplicates in bets list.", ("bets", bet_uuids));

    validate_account_name(better);
}

void create_nft_operation::validate() const
{
    FC_ASSERT(!uuid.is_nil(), "uuid must not be nil");
    validate_account_name(owner);
    validate_json_metadata(json_metadata);
}

void update_nft_meta_operation::validate() const
{
    FC_ASSERT(!uuid.is_nil(), "uuid must not be nil");
    FC_ASSERT(moderator == SCORUM_NFT_MODERATOR, "invalid moderator account");
    validate_json_metadata(json_metadata);
}

void adjust_nft_experience_operation::validate() const
{
    FC_ASSERT(!uuid.is_nil(), "uuid must not be nil");
    FC_ASSERT(moderator == SCORUM_NFT_MODERATOR, "invalid moderator account");
}

void update_nft_name_operation::validate() const
{
    FC_ASSERT(!uuid.is_nil(), "uuid must not be nil");
    FC_ASSERT(moderator == SCORUM_NFT_MODERATOR, "invalid moderator account");
    validate_account_name(name);
}

void create_game_round_operation::validate() const
{
    FC_ASSERT(!uuid.is_nil(), "uuid must not be nil");
//    FC_ASSERT(account == SCORUM_NFT_MODERATOR, "invalid moderator account");
    FC_ASSERT(verification_key.size() == 64, "verification_key should have 64 symbols length");
    FC_ASSERT(seed.size() == 64, "seed should have 64 symbols length");
}

void update_game_round_result_operation::validate() const
{
    FC_ASSERT(!uuid.is_nil(), "uuid must not be nil");
//    FC_ASSERT(account == SCORUM_NFT_MODERATOR, "invalid moderator account");
    FC_ASSERT(proof.size() == 160, "proof should have 160 symbols length");
    FC_ASSERT(vrf.size() == 128, "vrf should have 128 symbols length");
    FC_ASSERT(result >= 100, "result should be greater or equal 100");
}

void burn_operation::validate() const
{
    validate_account_name(owner);

    FC_ASSERT(is_asset_type(amount, SCORUM_SYMBOL), "Stake must be SCR");
    FC_ASSERT(amount.amount > 0, "Stake must be greater then 0");
}

} // namespace protocol
} // namespace scorum
