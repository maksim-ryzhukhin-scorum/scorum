#include <scorum/protocol/get_config.hpp>
#include <scorum/protocol/config.hpp>
#include <scorum/protocol/asset.hpp>
#include <scorum/protocol/types.hpp>
#include <scorum/protocol/version.hpp>

namespace scorum {
namespace protocol {

fc::variant_object get_config()
{
    fc::mutable_variant_object result;

// clang-format off

#ifdef IS_TEST_NET
    result[ "IS_TEST_NET" ] = true;
#else
    result[ "IS_TEST_NET" ] = false;
#endif

    // clang-format on

    result["SCORUM_100_PERCENT"] = SCORUM_100_PERCENT;
    result["SCORUM_1_PERCENT"] = SCORUM_1_PERCENT;
    result["SCORUM_1_TENTH_PERCENT"] = SCORUM_1_TENTH_PERCENT;
    result["SCORUM_ACCOUNT_RECOVERY_REQUEST_EXPIRATION_PERIOD"] = SCORUM_ACCOUNT_RECOVERY_REQUEST_EXPIRATION_PERIOD;
    result["SCORUM_ADDRESS_PREFIX"] = SCORUM_ADDRESS_PREFIX;
    result["SCORUM_BANDWIDTH_AVERAGE_WINDOW_SECONDS"] = SCORUM_BANDWIDTH_AVERAGE_WINDOW_SECONDS;
    result["SCORUM_BANDWIDTH_PRECISION"] = SCORUM_BANDWIDTH_PRECISION;
    result["SCORUM_BLOCKCHAIN_HARDFORK_VERSION"] = SCORUM_BLOCKCHAIN_HARDFORK_VERSION;
    result["SCORUM_BLOCKCHAIN_VERSION"] = SCORUM_BLOCKCHAIN_VERSION;
    result["SCORUM_BLOCK_INTERVAL"] = SCORUM_BLOCK_INTERVAL;
    result["SCORUM_BLOCKS_PER_DAY"] = SCORUM_BLOCKS_PER_DAY;
    result["SCORUM_BLOCKS_PER_HOUR"] = SCORUM_BLOCKS_PER_HOUR;
    result["SCORUM_BLOCKS_PER_YEAR"] = SCORUM_BLOCKS_PER_YEAR;
    result["SCORUM_CASHOUT_WINDOW_SECONDS"] = SCORUM_CASHOUT_WINDOW_SECONDS;
    result["SCORUM_CONTENT_REWARD_PERCENT"] = SCORUM_CONTENT_REWARD_PERCENT;
    result["SCORUM_CREATE_ACCOUNT_DELEGATION_RATIO"] = SCORUM_CREATE_ACCOUNT_DELEGATION_RATIO;
    result["SCORUM_CREATE_ACCOUNT_DELEGATION_TIME"] = SCORUM_CREATE_ACCOUNT_DELEGATION_TIME;
    result["SCORUM_CREATE_ACCOUNT_WITH_SCORUM_MODIFIER"] = SCORUM_CREATE_ACCOUNT_WITH_SCORUM_MODIFIER;
    result["SCORUM_HARDFORK_REQUIRED_WITNESSES"] = SCORUM_HARDFORK_REQUIRED_WITNESSES;
    result["SCORUM_IRREVERSIBLE_THRESHOLD"] = SCORUM_IRREVERSIBLE_THRESHOLD;
    result["SCORUM_MAX_ACCOUNT_NAME_LENGTH"] = SCORUM_MAX_ACCOUNT_NAME_LENGTH;
    result["SCORUM_MAX_ACCOUNT_WITNESS_VOTES"] = SCORUM_MAX_ACCOUNT_WITNESS_VOTES;
    result["SCORUM_MAX_BLOCK_SIZE"] = SCORUM_MAX_BLOCK_SIZE;
    result["SCORUM_MAX_COMMENT_DEPTH"] = SCORUM_MAX_COMMENT_DEPTH;
    result["SCORUM_MAX_MEMO_SIZE"] = SCORUM_MAX_MEMO_SIZE;
    result["SCORUM_MAX_WITNESSES"] = SCORUM_MAX_WITNESSES;
    result["SCORUM_MAX_PERMLINK_LENGTH"] = SCORUM_MAX_PERMLINK_LENGTH;
    result["SCORUM_MAX_PROXY_RECURSION_DEPTH"] = SCORUM_MAX_PROXY_RECURSION_DEPTH;
    result["SCORUM_MAX_RESERVE_RATIO"] = SCORUM_MAX_RESERVE_RATIO;
    result["SCORUM_MAX_RUNNER_WITNESSES"] = SCORUM_MAX_RUNNER_WITNESSES;
    result["SCORUM_MAX_SHARE_SUPPLY"] = SCORUM_MAX_SHARE_SUPPLY;
    result["SCORUM_MAX_SIG_CHECK_DEPTH"] = SCORUM_MAX_SIG_CHECK_DEPTH;
    result["SCORUM_MAX_TIME_UNTIL_EXPIRATION"] = SCORUM_MAX_TIME_UNTIL_EXPIRATION;
    result["SCORUM_MAX_TRANSACTION_SIZE"] = SCORUM_MAX_TRANSACTION_SIZE;
    result["SCORUM_MAX_UNDO_HISTORY"] = SCORUM_MAX_UNDO_HISTORY;
    result["SCORUM_MAX_VOTE_CHANGES"] = SCORUM_MAX_VOTE_CHANGES;
    result["SCORUM_MAX_VOTED_WITNESSES"] = SCORUM_MAX_VOTED_WITNESSES;
    result["SCORUM_MAX_WITHDRAW_ROUTES"] = SCORUM_MAX_WITHDRAW_ROUTES;
    result["SCORUM_MAX_WITNESS_URL_LENGTH"] = SCORUM_MAX_WITNESS_URL_LENGTH;
    result["SCORUM_MIN_ACCOUNT_CREATION_FEE"] = SCORUM_MIN_ACCOUNT_CREATION_FEE;
    result["SCORUM_MIN_ACCOUNT_NAME_LENGTH"] = SCORUM_MIN_ACCOUNT_NAME_LENGTH;
    result["SCORUM_MIN_BLOCK_SIZE_LIMIT"] = SCORUM_MIN_BLOCK_SIZE_LIMIT;
    result["SCORUM_MIN_PERMLINK_LENGTH"] = SCORUM_MIN_PERMLINK_LENGTH;
    result["SCORUM_MIN_REPLY_INTERVAL"] = SCORUM_MIN_REPLY_INTERVAL;
    result["SCORUM_MIN_ROOT_COMMENT_INTERVAL"] = SCORUM_MIN_ROOT_COMMENT_INTERVAL;
    result["SCORUM_MIN_VOTE_INTERVAL_SEC"] = SCORUM_MIN_VOTE_INTERVAL_SEC;
    result["SCORUM_MIN_COMMENT_PAYOUT"] = SCORUM_MIN_COMMENT_PAYOUT;
    result["SCORUM_MIN_TRANSACTION_EXPIRATION_LIMIT"] = SCORUM_MIN_TRANSACTION_EXPIRATION_LIMIT;
    result["SCORUM_MIN_UNDO_HISTORY"] = SCORUM_MIN_UNDO_HISTORY;
    result["SCORUM_NUM_INIT_DELEGATES"] = SCORUM_NUM_INIT_DELEGATES;
    result["SCORUM_OWNER_AUTH_HISTORY_TRACKING_START_BLOCK_NUM"] = SCORUM_OWNER_AUTH_HISTORY_TRACKING_START_BLOCK_NUM;
    result["SCORUM_OWNER_AUTH_RECOVERY_PERIOD"] = SCORUM_OWNER_AUTH_RECOVERY_PERIOD;
    result["SCORUM_OWNER_UPDATE_LIMIT"] = SCORUM_OWNER_UPDATE_LIMIT;
    result["SCORUM_PROXY_TO_SELF_ACCOUNT"] = SCORUM_PROXY_TO_SELF_ACCOUNT;
    result["SCORUM_RECENT_RSHARES_DECAY_RATE"] = SCORUM_RECENT_RSHARES_DECAY_RATE;
    result["SCORUM_REVERSE_AUCTION_WINDOW_SECONDS"] = SCORUM_REVERSE_AUCTION_WINDOW_SECONDS;
    result["SCORUM_ROOT_POST_PARENT_ACCOUNT"] = SCORUM_ROOT_POST_PARENT_ACCOUNT;
    result["SCORUM_SAVINGS_WITHDRAW_REQUEST_LIMIT"] = SCORUM_SAVINGS_WITHDRAW_REQUEST_LIMIT;
    result["SCORUM_SAVINGS_WITHDRAW_TIME"] = SCORUM_SAVINGS_WITHDRAW_TIME;
    result["SCORUM_SOFT_MAX_COMMENT_DEPTH"] = SCORUM_SOFT_MAX_COMMENT_DEPTH;
    result["SCORUM_START_MINER_VOTING_BLOCK"] = SCORUM_START_MINER_VOTING_BLOCK;
    result["SCORUM_UPVOTE_LOCKOUT"] = SCORUM_UPVOTE_LOCKOUT;
    result["SCORUM_VESTING_WITHDRAW_INTERVALS"] = SCORUM_VESTING_WITHDRAW_INTERVALS;
    result["SCORUM_VESTING_WITHDRAW_INTERVAL_SECONDS"] = SCORUM_VESTING_WITHDRAW_INTERVAL_SECONDS;
    result["SCORUM_VOTE_DUST_THRESHOLD"] = SCORUM_VOTE_DUST_THRESHOLD;
    result["SCORUM_VOTE_REGENERATION_SECONDS"] = SCORUM_VOTE_REGENERATION_SECONDS;
    result["SCORUM_SYMBOL"] = SCORUM_SYMBOL;
    result["SP_SYMBOL"] = SP_SYMBOL;
    result["VIRTUAL_SCHEDULE_LAP_LENGTH"] = VIRTUAL_SCHEDULE_LAP_LENGTH;
    result["SCORUM_REWARDS_INITIAL_SUPPLY_PERIOD_IN_DAYS"] = SCORUM_REWARDS_INITIAL_SUPPLY_PERIOD_IN_DAYS;
    result["SCORUM_GUARANTED_REWARD_SUPPLY_PERIOD_IN_DAYS"] = SCORUM_GUARANTED_REWARD_SUPPLY_PERIOD_IN_DAYS;
    result["SCORUM_REWARD_INCREASE_THRESHOLD_IN_DAYS"] = SCORUM_REWARD_INCREASE_THRESHOLD_IN_DAYS;
    result["SCORUM_ADJUST_REWARD_PERCENT"] = SCORUM_ADJUST_REWARD_PERCENT;
    result["SCORUM_BUDGET_LIMIT_COUNT_PER_OWNER"] = SCORUM_BUDGET_LIMIT_COUNT_PER_OWNER;
    result["SCORUM_BUDGET_LIMIT_DB_LIST_SIZE"] = SCORUM_BUDGET_LIMIT_DB_LIST_SIZE;
    result["SCORUM_BUDGET_LIMIT_API_LIST_SIZE"] = SCORUM_BUDGET_LIMIT_API_LIST_SIZE;
    result["SCORUM_REGISTRATION_BONUS_LIMIT_PER_MEMBER_N_BLOCK"] = SCORUM_REGISTRATION_BONUS_LIMIT_PER_MEMBER_N_BLOCK;
    result["SCORUM_REGISTRATION_BONUS_LIMIT_PER_MEMBER_PER_N_BLOCK"]
        = SCORUM_REGISTRATION_BONUS_LIMIT_PER_MEMBER_PER_N_BLOCK;
    ;
    result["SCORUM_REGISTRATION_LIMIT_COUNT_COMMITTEE_MEMBERS"] = SCORUM_REGISTRATION_LIMIT_COUNT_COMMITTEE_MEMBERS;
    result["SCORUM_WITNESS_MISSED_BLOCKS_THRESHOLD"] = SCORUM_WITNESS_MISSED_BLOCKS_THRESHOLD;

    result["SCORUM_ATOMICSWAP_INITIATOR_REFUND_LOCK_SECS"] = SCORUM_ATOMICSWAP_INITIATOR_REFUND_LOCK_SECS;
    result["SCORUM_ATOMICSWAP_PARTICIPANT_REFUND_LOCK_SECS"] = SCORUM_ATOMICSWAP_PARTICIPANT_REFUND_LOCK_SECS;
    result["SCORUM_ATOMICSWAP_LIMIT_REQUESTED_CONTRACTS_PER_OWNER"]
        = SCORUM_ATOMICSWAP_LIMIT_REQUESTED_CONTRACTS_PER_OWNER;
    result["SCORUM_ATOMICSWAP_LIMIT_REQUESTED_CONTRACTS_PER_RECIPIENT"]
        = SCORUM_ATOMICSWAP_LIMIT_REQUESTED_CONTRACTS_PER_RECIPIENT;

    result["SCORUM_ATOMICSWAP_CONTRACT_METADATA_MAX_LENGTH"] = SCORUM_ATOMICSWAP_CONTRACT_METADATA_MAX_LENGTH;
    result["SCORUM_ATOMICSWAP_SECRET_MAX_LENGTH"] = SCORUM_ATOMICSWAP_SECRET_MAX_LENGTH;

    result["SCORUM_BLOCKID_POOL_SIZE"] = SCORUM_BLOCKID_POOL_SIZE;

    result["SCORUM_CURRENCY_PRECISION"] = SCORUM_CURRENCY_PRECISION;

    result["SCORUM_MAX_VOTES_PER_DAY_VOTING_POWER_RATE"] = SCORUM_MAX_VOTES_PER_DAY_VOTING_POWER_RATE;

    result["SCORUM_MIN_DELEGATE_VESTING_SHARES_MODIFIER"] = SCORUM_MIN_DELEGATE_VESTING_SHARES_MODIFIER;

    return result;
}
} // namespace protocol
} // namespace scorum
