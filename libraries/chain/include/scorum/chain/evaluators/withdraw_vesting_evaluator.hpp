#pragma once

#include <scorum/protocol/scorum_operations.hpp>

#include <scorum/chain/evaluators/evaluator.hpp>

#include <scorum/chain/tasks_base.hpp>

#include <memory>

namespace scorum {
namespace chain {

class account_service_i;
class dynamic_global_property_service_i;
class withdraw_vesting_service_i;

class data_service_factory_i;

class account_object;

class withdraw_vesting_impl;

// This evaluator initiates withdraw vesting for account by operation withdraw_vesting_operation.
//
class withdraw_vesting_evaluator : public evaluator_impl<data_service_factory_i, withdraw_vesting_evaluator>
{
public:
    using operation_type = scorum::protocol::withdraw_vesting_operation;

    withdraw_vesting_evaluator(data_service_factory_i& services);
    ~withdraw_vesting_evaluator();

    void do_apply(const operation_type& op);

private:
    std::unique_ptr<withdraw_vesting_impl> _impl;

    account_service_i& _account_service;
    dynamic_global_property_service_i& _dprops_service;
};

using scorum::protocol::asset;

class withdraw_vesting_context
{
public:
    explicit withdraw_vesting_context(data_service_factory_i& services, const asset& scorumpower);

    data_service_factory_i& services() const
    {
        return _services;
    }

    const asset& scorumpower() const
    {
        return _scorumpower;
    }

private:
    data_service_factory_i& _services;
    asset _scorumpower;
};

// This task initiates for withdraw vesting for development pool withount any operation
// for development commitee purpose.
class withdraw_vesting_dev_pool_task : public task<withdraw_vesting_context>
{
public:
    void on_apply(withdraw_vesting_context& ctx);
};

} // namespace chain
} // namespace scorum
