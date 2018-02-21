#pragma once

#include <scorum/chain/tasks_base.hpp>

namespace scorum {
namespace chain {

class data_service_factory_i;
class genesis_state_type;

namespace genesis {

struct initializator_context
{
    explicit initializator_context(data_service_factory_i& services, const genesis_state_type& genesis_state);

    data_service_factory_i& services;
    const genesis_state_type& genesis_state;
};

class single_time_apply_guard : public task_reentrance_guard_i<initializator_context>
{
public:
    virtual bool is_allowed(initializator_context&)
    {
        return !_applied;
    }
    virtual void apply(initializator_context&)
    {
        _applied = true;
    }

private:
    bool _applied = false;
};

class initializator : public task<initializator_context, single_time_apply_guard>
{
protected:
    initializator()
    {
    }
};

} // genesis
}
}
