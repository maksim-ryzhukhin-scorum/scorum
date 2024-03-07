#pragma once

#include <scorum/chain/services/service_base.hpp>
#include <scorum/chain/schema/account_objects.hpp>
#include <scorum/chain/dba/dba.hpp>

namespace scorum {
namespace chain {

struct accounts_total
{
    /// sum of all SCR balances
    asset scr = asset(0, SCORUM_SYMBOL);

    /// sum of all SP balances
    asset sp = asset(0, SP_SYMBOL);

    /// sum of all pending SCR balances
    asset pending_scr = asset(0, SCORUM_SYMBOL);

    /// sum of all pending SP balances
    asset pending_sp = asset(0, SP_SYMBOL);

    share_type vsf_votes = 0;
};

struct account_service_i : public base_service_i<account_object>
{
    using base_service_i<account_object>::get;
    using base_service_i<account_object>::is_exists;

    virtual const account_object& get(const account_id_type&) const = 0;

    virtual const account_object& get_account(const account_name_type&) const = 0;

    virtual bool is_exists(const account_name_type&) const = 0;

    virtual const account_authority_object& get_account_authority(const account_name_type&) const = 0;

    virtual void check_account_existence(const account_name_type&,
                                         const optional<const char*>& context_type_name
                                         = optional<const char*>()) const = 0;

    virtual void check_account_existence(const account_authority_map&,
                                         const optional<const char*>& context_type_name
                                         = optional<const char*>()) const = 0;

    virtual const account_object& create_initial_account(const account_name_type& new_account_name,
                                                         const public_key_type& memo_key,
                                                         const asset& balance_in_scorums,
                                                         const std::string& json_metadata)
        = 0;

    virtual const account_object& create_account(const account_name_type& new_account_name,
                                                 const account_name_type& creator_name,
                                                 const public_key_type& memo_key,
                                                 const std::string& json_metadata,
                                                 const authority& owner,
                                                 const authority& active,
                                                 const authority& posting)
        = 0;

    virtual const account_object& create_account(const account_name_type& new_account_name,
                                                 const account_name_type& creator_name,
                                                 const public_key_type& memo_key,
                                                 const std::string& json_metadata,
                                                 const authority& owner,
                                                 const authority& active,
                                                 const authority& posting,
                                                 const asset& fee_in_scorums)
        = 0;

    virtual const account_object& create_account_with_delegation(const account_name_type& new_account_name,
                                                                 const account_name_type& creator_name,
                                                                 const public_key_type& memo_key,
                                                                 const std::string& json_metadata,
                                                                 const authority& owner,
                                                                 const authority& active,
                                                                 const authority& posting,
                                                                 const asset& fee_in_scorums,
                                                                 const asset& delegation_in_scorumpower)
        = 0;

    virtual const account_object& create_account_with_bonus(const account_name_type& new_account_name,
                                                            const account_name_type& creator_name,
                                                            const public_key_type& memo_key,
                                                            const std::string& json_metadata,
                                                            const authority& owner,
                                                            const authority& active,
                                                            const authority& posting,
                                                            const asset& bonus)
        = 0;

    virtual void update_acount(const account_object& account,
                               const account_authority_object& account_authority,
                               const public_key_type& memo_key,
                               const std::string& json_metadata,
                               const optional<authority>& owner,
                               const optional<authority>& active,
                               const optional<authority>& posting)
        = 0;

    virtual void increase_balance(const account_object& account, const asset& amount) = 0;
    virtual void increase_balance(account_name_type account_name, const asset& amount) = 0;
    virtual void decrease_balance(const account_object& account, const asset& amount) = 0;
    virtual void burn_scr(const account_object& account, const asset& amount) = 0;

    virtual void increase_pending_balance(const account_object& account, const asset& amount) = 0;
    virtual void decrease_pending_balance(const account_object& account, const asset& amount) = 0;

    virtual void increase_scorumpower(const account_object& account, const asset& amount) = 0;
    virtual void decrease_scorumpower(const account_object& account, const asset& amount) = 0;

    virtual void increase_pending_scorumpower(const account_object& account, const asset& amount) = 0;
    virtual void decrease_pending_scorumpower(const account_object& account, const asset& amount) = 0;

    virtual const asset create_scorumpower(const account_object& to_account, const asset& scorum) = 0;

    virtual void increase_delegated_scorumpower(const account_object& account, const asset& amount) = 0;

    virtual void increase_received_scorumpower(account_name_type account_name, const asset& amount) = 0;
    virtual void increase_received_scorumpower(const account_object& account, const asset& amount) = 0;
    virtual void decrease_received_scorumpower(account_name_type account_name, const asset& amount) = 0;
    virtual void decrease_received_scorumpower(const account_object& account, const asset& amount) = 0;

    virtual void drop_challenged(const account_object& account) = 0;

    virtual void prove_authority(const account_object& account, bool require_owner) = 0;

    virtual void increase_witnesses_voted_for(const account_object& account) = 0;
    virtual void decrease_witnesses_voted_for(const account_object& account) = 0;

    virtual void add_post(const account_object& author_account, const account_name_type& parent_author_name) = 0;

    virtual void update_voting_power(const account_object& account, uint16_t voting_power) = 0;

    virtual void update_active_sp_holders_cashout_time(const account_object& account) = 0;

    virtual void update_owner_authority(const account_object& account, const authority& owner_authority) = 0;

    virtual void create_account_recovery(const account_name_type& account_to_recover_name,
                                         const authority& new_owner_authority)
        = 0;

    virtual void submit_account_recovery(const account_object& account_to_recover,
                                         const authority& new_owner_authority,
                                         const authority& recent_owner_authority)
        = 0;

    virtual void change_recovery_account(const account_object& account_to_recover,
                                         const account_name_type& new_recovery_account)
        = 0;

    virtual void update_voting_proxy(const account_object& account, const optional<account_object>& proxy_account) = 0;

    virtual void clear_witness_votes(const account_object& account) = 0;

    virtual void adjust_proxied_witness_votes(const account_object& account,
                                              const std::array<share_type, SCORUM_MAX_PROXY_RECURSION_DEPTH + 1>& delta,
                                              int depth = 0)
        = 0;

    virtual void adjust_proxied_witness_votes(const account_object& account, const share_type& delta, int depth = 0)
        = 0;

    using cref_type = typename base_service_i::object_cref_type;

    using account_refs_type = std::vector<cref_type>;

    virtual account_refs_type get_active_sp_holders() const = 0;

    using account_call_type = typename base_service_i::call_type;

    virtual void foreach_account(account_call_type&&) const = 0;

    virtual accounts_total accounts_circulating_capital() const = 0;

    virtual account_refs_type get_by_cashout_time(const fc::time_point_sec& until) const = 0;
};

// DB operations with account_*** objects
//
class dbs_account : public dbs_service_base<account_service_i>
{
    friend class dbservice_dbs_factory;

public:
    explicit dbs_account(dba::db_index&, dynamic_global_property_service_i&, witness_service_i&);

    using base_service_i<account_object>::get;
    using base_service_i<account_object>::is_exists;

    virtual const account_object& get(const account_id_type&) const override;

    virtual const account_object& get_account(const account_name_type&) const override;

    virtual bool is_exists(const account_name_type&) const override;

    virtual const account_authority_object& get_account_authority(const account_name_type&) const override;

    virtual void check_account_existence(const account_name_type&,
                                         const optional<const char*>& context_type_name
                                         = optional<const char*>()) const override;

    virtual void check_account_existence(const account_authority_map&,
                                         const optional<const char*>& context_type_name
                                         = optional<const char*>()) const override;

    virtual const account_object& create_initial_account(const account_name_type& new_account_name,
                                                         const public_key_type& memo_key,
                                                         const asset& balance_in_scorums,
                                                         const std::string& json_metadata) override;

    const account_object& create_account(const account_name_type& new_account_name,
                                         const account_name_type& creator_name,
                                         const public_key_type& memo_key,
                                         const std::string& json_metadata,
                                         const authority& owner,
                                         const authority& active,
                                         const authority& posting) override;

    virtual const account_object& create_account(const account_name_type& new_account_name,
                                                 const account_name_type& creator_name,
                                                 const public_key_type& memo_key,
                                                 const std::string& json_metadata,
                                                 const authority& owner,
                                                 const authority& active,
                                                 const authority& posting,
                                                 const asset& fee_in_scorums) override;

    virtual const account_object& create_account_with_delegation(const account_name_type& new_account_name,
                                                                 const account_name_type& creator_name,
                                                                 const public_key_type& memo_key,
                                                                 const std::string& json_metadata,
                                                                 const authority& owner,
                                                                 const authority& active,
                                                                 const authority& posting,
                                                                 const asset& fee_in_scorums,
                                                                 const asset& delegation_in_scorumpower) override;

    virtual const account_object& create_account_with_bonus(const account_name_type& new_account_name,
                                                            const account_name_type& creator_name,
                                                            const public_key_type& memo_key,
                                                            const std::string& json_metadata,
                                                            const authority& owner,
                                                            const authority& active,
                                                            const authority& posting,
                                                            const asset& bonus) override;

    virtual void update_acount(const account_object& account,
                               const account_authority_object& account_authority,
                               const public_key_type& memo_key,
                               const std::string& json_metadata,
                               const optional<authority>& owner,
                               const optional<authority>& active,
                               const optional<authority>& posting) override;

    virtual void increase_balance(const account_object& account, const asset& amount) override;
    virtual void increase_balance(account_name_type account_name, const asset& amount) override;
    virtual void decrease_balance(const account_object& account, const asset& amount) override;
    virtual void burn_scr(const account_object& account, const asset& amount) override;

    virtual void increase_pending_balance(const account_object& account, const asset& amount) override;
    virtual void decrease_pending_balance(const account_object& account, const asset& amount) override;

    virtual void increase_scorumpower(const account_object& account, const asset& amount) override;
    virtual void decrease_scorumpower(const account_object& account, const asset& amount) override;

    virtual void increase_pending_scorumpower(const account_object& account, const asset& amount) override;
    virtual void decrease_pending_scorumpower(const account_object& account, const asset& amount) override;

    virtual const asset create_scorumpower(const account_object& to_account, const asset& scorum) override;

    virtual void increase_delegated_scorumpower(const account_object& account, const asset& amount) override;

    virtual void increase_received_scorumpower(account_name_type account_name, const asset& amount) override;
    virtual void increase_received_scorumpower(const account_object& account, const asset& amount) override;
    virtual void decrease_received_scorumpower(account_name_type account_name, const asset& amount) override;
    virtual void decrease_received_scorumpower(const account_object& account, const asset& amount) override;

    virtual void drop_challenged(const account_object& account) override;

    virtual void prove_authority(const account_object& account, bool require_owner) override;

    virtual void increase_witnesses_voted_for(const account_object& account) override;
    virtual void decrease_witnesses_voted_for(const account_object& account) override;

    virtual void add_post(const account_object& author_account, const account_name_type& parent_author_name) override;

    virtual void update_voting_power(const account_object& account, uint16_t voting_power) override;

    virtual void update_active_sp_holders_cashout_time(const account_object& account) override;

    virtual void update_owner_authority(const account_object& account, const authority& owner_authority) override;

    virtual void create_account_recovery(const account_name_type& account_to_recover_name,
                                         const authority& new_owner_authority) override;

    virtual void submit_account_recovery(const account_object& account_to_recover,
                                         const authority& new_owner_authority,
                                         const authority& recent_owner_authority) override;

    virtual void change_recovery_account(const account_object& account_to_recover,
                                         const account_name_type& new_recovery_account) override;

    virtual void update_voting_proxy(const account_object& account,
                                     const optional<account_object>& proxy_account) override;

    /** clears all vote records for a particular account but does not update the
     * witness vote totals.  Vote totals should be updated first via a call to
     * adjust_proxied_witness_votes( a, -a.witness_vote_weight() )
     */
    virtual void clear_witness_votes(const account_object& account) override;

    /** this updates the votes for witnesses as a result of account voting proxy changing */
    virtual void adjust_proxied_witness_votes(const account_object& account,
                                              const std::array<share_type, SCORUM_MAX_PROXY_RECURSION_DEPTH + 1>& delta,
                                              int depth = 0) override;

    /** this updates the votes for all witnesses as a result of account SP changing */
    virtual void
    adjust_proxied_witness_votes(const account_object& account, const share_type& delta, int depth = 0) override;

    virtual account_refs_type get_active_sp_holders() const override;

    virtual void foreach_account(account_call_type&&) const override;

    virtual accounts_total accounts_circulating_capital() const override;

    virtual account_refs_type get_by_cashout_time(const fc::time_point_sec& until) const override;

private:
    dynamic_global_property_service_i& _dgp_svc;
    witness_service_i& _witness_svc;
};

} // namespace chain
} // namespace scorum
