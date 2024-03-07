#pragma once

#include <scorum/protocol/scorum_operations.hpp>

#include <scorum/chain/evaluators/evaluator.hpp>

namespace scorum {
namespace chain {

using namespace scorum::protocol;

DEFINE_EVALUATOR(account_create)
DEFINE_EVALUATOR(account_create_with_delegation)
DEFINE_EVALUATOR(account_update)
DEFINE_EVALUATOR(account_witness_proxy)
DEFINE_EVALUATOR(account_witness_vote)
DEFINE_EVALUATOR(atomicswap_initiate)
DEFINE_EVALUATOR(atomicswap_redeem)
DEFINE_EVALUATOR(atomicswap_refund)
DEFINE_EVALUATOR(change_recovery_account)
DEFINE_EVALUATOR(comment)
DEFINE_EVALUATOR(comment_options)
DEFINE_EVALUATOR(decline_voting_rights)
DEFINE_EVALUATOR(delegate_scorumpower)
DEFINE_EVALUATOR(delete_comment)
DEFINE_EVALUATOR(escrow_approve)
DEFINE_EVALUATOR(escrow_dispute)
DEFINE_EVALUATOR(escrow_release)
DEFINE_EVALUATOR(escrow_transfer)
DEFINE_EVALUATOR(prove_authority)
DEFINE_EVALUATOR(recover_account)
DEFINE_EVALUATOR(request_account_recovery)
DEFINE_EVALUATOR(transfer)
DEFINE_EVALUATOR(transfer_to_scorumpower)
DEFINE_EVALUATOR(witness_update)
DEFINE_EVALUATOR(burn)
} // namespace chain
} // namespace scorum
