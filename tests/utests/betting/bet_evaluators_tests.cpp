#include <boost/test/unit_test.hpp>

#include <scorum/chain/dba/db_accessor.hpp>
#include <scorum/chain/evaluators/post_bet_evalulator.hpp>
#include <scorum/chain/evaluators/cancel_pending_bets_evaluator.hpp>

#include <boost/uuid/uuid_generators.hpp>

#include "betting_common.hpp"
#include "object_wrapper.hpp"

namespace {

using namespace scorum;
using namespace scorum::chain;
using namespace scorum::protocol;

struct post_bet_evaluator_fixture : public betting_common::betting_evaluator_fixture_impl
{
    post_bet_evaluator_fixture()
        : uuid_hist_dba(*mocks.Mock<dba::db_index>())
        , evaluator_for_test(*dbs_services, *betting_matcher_moc, *betting_service_moc, uuid_hist_dba)
    {
        better.scorum(ASSET_SCR(1e+9));
        account_service.add_actor(better);

        games.create([&](game_object& obj) {
            obj.uuid = uuid_gen("game");
            fc::from_string(obj.name, "test_ok");
        });

        test_op.better = better.name;
        test_op.uuid = uuid_gen("game");
        test_op.wincase = correct_score_home::yes();
        test_op.odds = { 3, 1 };
        test_op.stake = better.scr_amount;
    }

    pending_bet_object create_bet()
    {
        return create_object<pending_bet_object>(shm, [&](pending_bet_object& obj) {
            obj.data.better = test_op.better;
            obj.game = 0;
            obj.data.uuid = uuid_gen("bet");
            obj.data.wincase = test_op.wincase;
            obj.data.bet_odds = odds(test_op.odds.numerator, test_op.odds.denominator);
            obj.data.stake = test_op.stake;
            obj.market = create_market(test_op.wincase);
        });
    }

    dba::db_accessor<bet_uuid_history_object> uuid_hist_dba;

    post_bet_evaluator evaluator_for_test;

    post_bet_operation test_op;

    Actor better = "alice";

    uuid_type uuid_ns = boost::uuids::string_generator()("e629f9aa-6b2c-46aa-8fa8-36770e7a7a5f");
    boost::uuids::name_generator uuid_gen = boost::uuids::name_generator(uuid_ns);
};

BOOST_FIXTURE_TEST_SUITE(post_bet_evaluator_tests, post_bet_evaluator_fixture)

SCORUM_TEST_CASE(post_bet_evaluator_operation_validate_check)
{
    post_bet_operation op = test_op;

    BOOST_CHECK_NO_THROW(op.validate());

    op.better = "";
    BOOST_CHECK_THROW(op.validate(), fc::assert_exception);
    op = test_op;

    op.odds = { 1, 10 };
    BOOST_CHECK_THROW(op.validate(), fc::assert_exception);
    op = test_op;

    op.stake.amount = 0;
    BOOST_CHECK_THROW(op.validate(), fc::assert_exception);
    op = test_op;

    op.stake = ASSET_SP(1e+9);
    BOOST_CHECK_THROW(op.validate(), fc::assert_exception);
    op = test_op;
}

SCORUM_TEST_CASE(post_bet_evaluator_negative_check)
{
    // TODO: implement when db_accessors will be introduced
    // mocks.OnCallFunc((dba::detail::is_exists_by<bet_uuid_history_object, by_uuid, uuid_type>));
}

SCORUM_TEST_CASE(post_bet_evaluator_positive_check)
{
    // TODO: implement when db_accessors will be introduced
}

BOOST_AUTO_TEST_SUITE_END()

struct cancel_pending_bets_evaluator_fixture : public shared_memory_fixture
{
    using check_account_existence_ptr
        = void (account_service_i::*)(const account_name_type&, const fc::optional<const char*>&) const;
    using is_exists_ptr = bool (pending_bet_service_i::*)(const uuid_type&) const;
    using get_ptr = const pending_bet_object& (pending_bet_service_i::*)(const uuid_type&)const;

    MockRepository mocks;

    data_service_factory_i* dbs_factory = mocks.Mock<data_service_factory_i>();
    betting_service_i* betting_svc = mocks.Mock<betting_service_i>();
    account_service_i* acc_svc = mocks.Mock<account_service_i>();
    pending_bet_service_i* pending_bet_svc = mocks.Mock<pending_bet_service_i>();

    uuid_type uuid_ns = boost::uuids::string_generator()("e629f9aa-6b2c-46aa-8fa8-36770e7a7a5f");
    boost::uuids::name_generator uuid_gen = boost::uuids::name_generator(uuid_ns);

    cancel_pending_bets_evaluator_fixture()
    {
        mocks.OnCall(dbs_factory, data_service_factory_i::account_service).ReturnByRef(*acc_svc);
        mocks.OnCall(dbs_factory, data_service_factory_i::pending_bet_service).ReturnByRef(*pending_bet_svc);
    }
};

BOOST_FIXTURE_TEST_SUITE(cancel_pending_bets_evaluator_tests, cancel_pending_bets_evaluator_fixture)

SCORUM_TEST_CASE(cancel_pending_bets_operation_validate_check)
{
    cancel_pending_bets_operation op;
    op.better = "better";

    BOOST_CHECK_NO_THROW(op.validate());

    op.better = "";
    BOOST_CHECK_THROW(op.validate(), fc::assert_exception);
}

SCORUM_TEST_CASE(bet_id_existance_check_should_throw)
{
    cancel_pending_bets_operation op;
    op.better = "better";
    op.bet_uuids = { uuid_gen("0") };

    cancel_pending_bets_evaluator ev(*dbs_factory, *betting_svc);

    mocks.ExpectCallOverload(acc_svc, (check_account_existence_ptr)&account_service_i::check_account_existence);
    mocks.ExpectCallOverload(pending_bet_svc, (is_exists_ptr)&pending_bet_service_i::is_exists)
        .With(uuid_gen("0"))
        .Return(false);

    BOOST_CHECK_THROW(ev.do_apply(op), fc::assert_exception);
}

SCORUM_TEST_CASE(better_mismatch_should_throw)
{
    cancel_pending_bets_operation op;
    op.better = "better";
    op.bet_uuids = { uuid_gen("0") };

    auto obj = create_object<pending_bet_object>(shm, [](pending_bet_object& o) { o.data.better = "cartman"; });

    mocks.OnCallOverload(acc_svc, (check_account_existence_ptr)&account_service_i::check_account_existence);
    mocks.OnCallOverload(pending_bet_svc, (is_exists_ptr)&pending_bet_service_i::is_exists)
        .With(uuid_gen("0"))
        .Return(true);
    mocks.ExpectCallOverload(pending_bet_svc, (get_ptr)&pending_bet_service_i::get_pending_bet)
        .With(uuid_gen("0"))
        .ReturnByRef(obj);

    cancel_pending_bets_evaluator ev(*dbs_factory, *betting_svc);
    BOOST_CHECK_THROW(ev.do_apply(op), fc::assert_exception);
}

SCORUM_TEST_CASE(should_cancel_bets)
{
    cancel_pending_bets_operation op;
    op.better = "better";
    op.bet_uuids = { uuid_gen("0"), uuid_gen("1") };

    // clang-format off
    auto obj1 = create_object<pending_bet_object>(shm, [&](pending_bet_object& o){ o.data.better = "better"; o.id = 0; o.data.uuid = uuid_gen("0"); });
    auto obj2 = create_object<pending_bet_object>(shm, [&](pending_bet_object& o){ o.data.better = "better"; o.id = 1; o.data.uuid = uuid_gen("1"); });

    mocks.OnCallOverload(acc_svc, (check_account_existence_ptr)&account_service_i::check_account_existence);
    mocks.OnCallOverload(pending_bet_svc, (is_exists_ptr)&pending_bet_service_i::is_exists).With(uuid_gen("0")).Return(true);
    mocks.OnCallOverload(pending_bet_svc, (is_exists_ptr)&pending_bet_service_i::is_exists).With(uuid_gen("1")).Return(true);
    mocks.OnCallOverload(pending_bet_svc, (get_ptr)&pending_bet_service_i::get_pending_bet).With(uuid_gen("0")).ReturnByRef(obj1);
    mocks.OnCallOverload(pending_bet_svc, (get_ptr)&pending_bet_service_i::get_pending_bet).With(uuid_gen("1")).ReturnByRef(obj2);
    mocks.ExpectCall(betting_svc, betting_service_i::cancel_pending_bet).With(0);
    mocks.ExpectCall(betting_svc, betting_service_i::cancel_pending_bet).With(1);
    // clang-format on

    cancel_pending_bets_evaluator ev(*dbs_factory, *betting_svc);
    BOOST_CHECK_NO_THROW(ev.do_apply(op));
}

BOOST_AUTO_TEST_SUITE_END()
}
