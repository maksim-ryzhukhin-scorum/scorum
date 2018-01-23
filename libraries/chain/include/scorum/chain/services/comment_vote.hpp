#pragma once

#include <scorum/chain/services/dbs_base.hpp>

#include <functional>

namespace scorum {
namespace chain {

class comment_vote_object;

struct comment_vote_service_i
{
    using modifier_type = std::function<void(comment_vote_object&)>;

    virtual const comment_vote_object& get(const comment_id_type& comment_id,
                                           const account_id_type& voter_id) const = 0;
    virtual bool is_exists(const comment_id_type& comment_id, const account_id_type& voter_id) const = 0;

    virtual const comment_vote_object& create(modifier_type modifier) = 0;
    virtual void update(const comment_vote_object& comment_vote, modifier_type modifier) = 0;
    virtual void remove(const comment_id_type& comment_id) = 0;
};

class dbs_comment_vote : public dbs_base, public comment_vote_service_i
{
    friend class dbservice_dbs_factory;

protected:
    explicit dbs_comment_vote(database& db);

public:
    const comment_vote_object& get(const comment_id_type& comment_id, const account_id_type& voter_id) const override;

    bool is_exists(const comment_id_type& comment_id, const account_id_type& voter_id) const override;

    const comment_vote_object& create(modifier_type modifier) override;

    void update(const comment_vote_object& comment_vote, modifier_type modifier) override;

    void remove(const comment_id_type& comment_id) override;
};
} // namespace chain
} // namespace scorum
