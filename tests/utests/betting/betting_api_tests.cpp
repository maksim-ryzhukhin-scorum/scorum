#include <boost/test/unit_test.hpp>

#include <scorum/protocol/betting/market.hpp>

#include <scorum/app/betting_api_impl.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/algorithm/cxx11/is_sorted.hpp>

#include <hippomocks.h>

#include "object_wrapper.hpp"
#include "service_wrappers.hpp"

namespace betting_api_tests {

using namespace scorum;
using namespace scorum::app;

class fixture : public shared_memory_fixture
{
public:
    MockRepository mocks;
    data_service_factory_i* factory = mocks.Mock<data_service_factory_i>();

    game_service_i* game_service = mocks.Mock<game_service_i>();
    pending_bet_service_i* pending_bet_service = mocks.Mock<pending_bet_service_i>();
    matched_bet_service_i* matched_bet_service = mocks.Mock<matched_bet_service_i>();
    betting_property_service_i* betting_property_service = mocks.Mock<betting_property_service_i>();

    database* db_mock = mocks.Mock<database>();
    dba::db_accessor<game_object> game_dba;
    dba::db_accessor<matched_bet_object> matched_bet_dba;

    fixture()
        : game_dba(*db_mock)
        , matched_bet_dba(*db_mock)
    {
    }

    void init()
    {
        mocks.OnCall(factory, data_service_factory_i::game_service).ReturnByRef(*game_service);
        mocks.OnCall(factory, data_service_factory_i::pending_bet_service).ReturnByRef(*pending_bet_service);
        mocks.OnCall(factory, data_service_factory_i::matched_bet_service).ReturnByRef(*matched_bet_service);
        mocks.OnCall(factory, data_service_factory_i::betting_property_service).ReturnByRef(*betting_property_service);
    }
};

BOOST_AUTO_TEST_SUITE(betting_api_tests)

BOOST_FIXTURE_TEST_CASE(get_services_in_constructor, fixture)
{
    game_service_i* game_service = mocks.Mock<game_service_i>();
    pending_bet_service_i* pending_bet_service = mocks.Mock<pending_bet_service_i>();
    matched_bet_service_i* matched_bet_service = mocks.Mock<matched_bet_service_i>();

    mocks.ExpectCall(factory, data_service_factory_i::game_service).ReturnByRef(*game_service);
    mocks.ExpectCall(factory, data_service_factory_i::pending_bet_service).ReturnByRef(*pending_bet_service);
    mocks.ExpectCall(factory, data_service_factory_i::matched_bet_service).ReturnByRef(*matched_bet_service);
    mocks.ExpectCall(factory, data_service_factory_i::betting_property_service).ReturnByRef(*betting_property_service);

    BOOST_REQUIRE_NO_THROW(betting_api_impl api(*factory, game_dba, matched_bet_dba));
}

BOOST_FIXTURE_TEST_CASE(get_games_dont_throw, fixture)
{
    init();

    betting_api_impl api(*factory, game_dba, matched_bet_dba);

    std::vector<game_object> objects;

    mocks
        .ExpectCallOverload(game_service,
                            (game_service_i::view_type(game_service_i::*)() const) & game_service_i::get_games)
        .Return({ objects.begin(), objects.end() });

    BOOST_REQUIRE_NO_THROW(api.get_games(game_filter::all));
}

struct get_game_winners_fixture : public fixture
{
    get_game_winners_fixture()
    {
        init();
    }

    scorum::uuid_type uuid_ns = boost::uuids::string_generator()("00000000-0000-0000-0000-000000000001");
    boost::uuids::name_generator uuid_gen = boost::uuids::name_generator(uuid_ns);
};

BOOST_FIXTURE_TEST_CASE(unknown_uuid_should_throw, get_game_winners_fixture)
{
    mocks.ExpectCallFunc((dba::detail::is_exists_by<game_object, by_uuid, uuid_type>)).Return(false);

    betting_api_impl api(*factory, game_dba, matched_bet_dba);
    BOOST_CHECK_THROW(api.get_game_winners(uuid_gen("unknown")), fc::assert_exception);
}

BOOST_FIXTURE_TEST_CASE(non_finished_game_should_throw, get_game_winners_fixture)
{
    betting_api_impl api(*factory, game_dba, matched_bet_dba);

    mocks.OnCallFunc((dba::detail::is_exists_by<game_object, by_uuid, uuid_type>)).Return(true);

    {
        auto game = create_object<game_object>(shm, [](game_object& g) { g.status = game_status::created; });
        mocks.ExpectCallFunc((dba::detail::get_by<game_object, by_uuid, uuid_type>)).ReturnByRef(game);
        BOOST_CHECK_THROW(api.get_game_winners(uuid_gen("")), fc::assert_exception);
    }
    {
        auto game = create_object<game_object>(shm, [](game_object& g) { g.status = game_status::started; });
        mocks.ExpectCallFunc((dba::detail::get_by<game_object, by_uuid, uuid_type>)).ReturnByRef(game);
        BOOST_CHECK_THROW(api.get_game_winners(uuid_gen("")), fc::assert_exception);
    }
}

BOOST_FIXTURE_TEST_CASE(get_game_winners_positive, get_game_winners_fixture)
{
    namespace dd = dba::detail;

    auto game_uuid = uuid_gen("game");
    auto game = create_object<game_object>(shm, [&](game_object& g) {
        g.id = 0;
        g.uuid = game_uuid;
        g.status = game_status::finished;
        g.results = { handicap::under{ 500 }, correct_score::yes{ 3, 3 }, goal_home::no{} };
    });
    // clang-format off
    auto matched_bets = {
        create_object<matched_bet_object>(shm, [this](matched_bet_object& o) {
            o.market = handicap{ 500 };
            o.bet1_data = { uuid_gen("m1b1"), {}, "m1b1", handicap::over{ 500 }, ASSET_SCR(1000) };
            o.bet2_data = { uuid_gen("m1b2"), {}, "m1b2", handicap::under{ 500 }, ASSET_SCR(500) }; // winner
        }),
        create_object<matched_bet_object>(shm, [this](matched_bet_object& o) { // no result in game_object for this one
            o.market = handicap{1000};
            o.bet1_data = { uuid_gen("trd_result1"), {}, "trd_result1", handicap::over{ 1000 }, ASSET_SCR(2000) };
            o.bet2_data = { uuid_gen("trd_result2"), {}, "trd_result2", handicap::under{ 1000 }, ASSET_SCR(1000) };
        }),
        create_object<matched_bet_object>(shm, [this](matched_bet_object& o) {
            o.market = correct_score{ 3, 3 };
            o.bet1_data = { uuid_gen("m2b1"), {}, "m2b1", correct_score::yes{ 3, 3, }, ASSET_SCR(3000) }; // winner
            o.bet2_data = { uuid_gen("m2b2"), {}, "m2b2", correct_score::no{ 3, 3 }, ASSET_SCR(1500) };
        }),
        create_object<matched_bet_object>(shm, [this](matched_bet_object& o) {
            o.market = goal_home{};
            o.bet1_data = { uuid_gen("m3b1"), {}, "m3b1", goal_home::yes{}, ASSET_SCR(4000) };
            o.bet2_data = { uuid_gen("m3b2"), {}, "m3b2", goal_home::no{}, ASSET_SCR(2000) }; // winner
        }),
        create_object<matched_bet_object>(shm, [this](matched_bet_object& o) { // no result in game_object for this one
            o.market = total{ 2000 };
            o.bet1_data = { uuid_gen("trd_result1_2"), {}, "trd_result1_2", total::over{ 2000 }, ASSET_SCR(5000) };
            o.bet2_data = { uuid_gen("trd_result2_2"), {}, "trd_result2_2", total::under{ 2000 }, ASSET_SCR(2500) };
        })
    };
    BOOST_REQUIRE(boost::algorithm::is_sorted(matched_bets, [](const auto& l, const auto& r) { return l.market < r.market; }));
    // clang-format on

    mocks.OnCallFunc((dd::is_exists_by<game_object, by_uuid, uuid_type>)).Return(true);
    mocks.ExpectCallFunc((dd::get_by<game_object, by_uuid, uuid_type>)).With(_, game_uuid).ReturnByRef(game);
    mocks.ExpectCallFunc((dd::get_range_by<matched_bet_object, by_game_id_market, game_id_type>)).Return(matched_bets);

    betting_api_impl api(*factory, game_dba, matched_bet_dba);

    auto winners = api.get_game_winners(game_uuid);

    BOOST_REQUIRE_EQUAL(winners.size(), 3u);

    BOOST_CHECK_EQUAL(winners[0].market.which(), (int)chain::market_type::tag<handicap>::value);
    BOOST_CHECK_EQUAL(winners[0].profit.amount, 1000u);
    BOOST_CHECK_EQUAL(winners[0].income.amount, 1500u);
    BOOST_CHECK_EQUAL(winners[0].winner.wincase.which(), (int)chain::wincase_type::tag<handicap::under>::value);
    BOOST_CHECK_EQUAL(boost::uuids::to_string(winners[0].winner.uuid), boost::uuids::to_string(uuid_gen("m1b2")));
    BOOST_CHECK_EQUAL(winners[0].winner.name, "m1b2");
    BOOST_CHECK_EQUAL(winners[0].loser.wincase.which(), (int)chain::wincase_type::tag<handicap::over>::value);
    BOOST_CHECK_EQUAL(boost::uuids::to_string(winners[0].loser.uuid), boost::uuids::to_string(uuid_gen("m1b1")));
    BOOST_CHECK_EQUAL(winners[0].loser.name, "m1b1");

    BOOST_CHECK_EQUAL(winners[1].market.which(), (int)chain::market_type::tag<correct_score>::value);
    BOOST_CHECK_EQUAL(winners[1].profit.amount, 1500u);
    BOOST_CHECK_EQUAL(winners[1].income.amount, 4500u);
    BOOST_CHECK_EQUAL(winners[1].winner.wincase.which(), (int)chain::wincase_type::tag<correct_score::yes>::value);
    BOOST_CHECK_EQUAL(boost::uuids::to_string(winners[1].winner.uuid), boost::uuids::to_string(uuid_gen("m2b1")));
    BOOST_CHECK_EQUAL(winners[1].winner.name, "m2b1");
    BOOST_CHECK_EQUAL(winners[1].loser.wincase.which(), (int)chain::wincase_type::tag<correct_score::no>::value);
    BOOST_CHECK_EQUAL(boost::uuids::to_string(winners[1].loser.uuid), boost::uuids::to_string(uuid_gen("m2b2")));
    BOOST_CHECK_EQUAL(winners[1].loser.name, "m2b2");

    BOOST_CHECK_EQUAL(winners[2].market.which(), (int)chain::market_type::tag<goal_home>::value);
    BOOST_CHECK_EQUAL(winners[2].profit.amount, 4000u);
    BOOST_CHECK_EQUAL(winners[2].income.amount, 6000u);
    BOOST_CHECK_EQUAL(winners[2].winner.wincase.which(), (int)chain::wincase_type::tag<goal_home::no>::value);
    BOOST_CHECK_EQUAL(boost::uuids::to_string(winners[2].winner.uuid), boost::uuids::to_string(uuid_gen("m3b2")));
    BOOST_CHECK_EQUAL(winners[2].winner.name, "m3b2");
    BOOST_CHECK_EQUAL(winners[2].loser.wincase.which(), (int)chain::wincase_type::tag<goal_home::yes>::value);
    BOOST_CHECK_EQUAL(boost::uuids::to_string(winners[2].loser.uuid), boost::uuids::to_string(uuid_gen("m3b1")));
    BOOST_CHECK_EQUAL(winners[2].loser.name, "m3b1");
}

struct get_games_fixture : public fixture
{
    get_games_fixture()
    {
        init();

        objects.push_back(
            create_object<game_object>(shm, [&](game_object& game) { game.status = game_status::created; }));

        objects.push_back(
            create_object<game_object>(shm, [&](game_object& game) { game.status = game_status::started; }));

        objects.push_back(
            create_object<game_object>(shm, [&](game_object& game) { game.status = game_status::finished; }));
    }

    std::vector<game_object> objects;
};

BOOST_FIXTURE_TEST_CASE(get_games_return_all_games, get_games_fixture)
{
    mocks
        .ExpectCallOverload(game_service,
                            (game_service_i::view_type(game_service_i::*)() const) & game_service_i::get_games)
        .Return({ objects.begin(), objects.end() });

    betting_api_impl api(*factory, game_dba, matched_bet_dba);
    std::vector<game_api_object> games = api.get_games(game_filter::all);

    BOOST_CHECK_EQUAL(games.size(), 3u);
}

BOOST_FIXTURE_TEST_CASE(get_games_does_not_change_order, get_games_fixture)
{
    mocks
        .ExpectCallOverload(game_service,
                            (game_service_i::view_type(game_service_i::*)() const) & game_service_i::get_games)
        .Return({ objects.begin(), objects.end() });

    betting_api_impl api(*factory, game_dba, matched_bet_dba);
    std::vector<game_api_object> games = api.get_games(game_filter::all);

    BOOST_CHECK(games[0].status == game_status::created);
    BOOST_CHECK(games[1].status == game_status::started);
    BOOST_CHECK(games[2].status == game_status::finished);
}

BOOST_FIXTURE_TEST_CASE(return_games_with_created_status, get_games_fixture)
{
    mocks
        .ExpectCallOverload(game_service,
                            (game_service_i::view_type(game_service_i::*)() const) & game_service_i::get_games)
        .Return({ objects.begin(), objects.end() });

    betting_api_impl api(*factory, game_dba, matched_bet_dba);
    std::vector<game_api_object> games = api.get_games(game_filter::created);

    BOOST_REQUIRE_EQUAL(games.size(), 1);
    BOOST_CHECK(games[0].status == game_status::created);
}

BOOST_FIXTURE_TEST_CASE(return_games_with_started_status, get_games_fixture)
{
    mocks
        .ExpectCallOverload(game_service,
                            (game_service_i::view_type(game_service_i::*)() const) & game_service_i::get_games)
        .Return({ objects.begin(), objects.end() });

    betting_api_impl api(*factory, game_dba, matched_bet_dba);
    std::vector<game_api_object> games = api.get_games(game_filter::started);

    BOOST_REQUIRE_EQUAL(games.size(), 1);
    BOOST_CHECK(games[0].status == game_status::started);
}

BOOST_FIXTURE_TEST_CASE(return_games_with_finished_status, get_games_fixture)
{
    mocks
        .ExpectCallOverload(game_service,
                            (game_service_i::view_type(game_service_i::*)() const) & game_service_i::get_games)
        .Return({ objects.begin(), objects.end() });

    betting_api_impl api(*factory, game_dba, matched_bet_dba);
    std::vector<game_api_object> games = api.get_games(game_filter::finished);

    BOOST_REQUIRE_EQUAL(games.size(), 1);
    BOOST_CHECK(games[0].status == game_status::finished);
}

BOOST_FIXTURE_TEST_CASE(return_two_games_with_finished_status, get_games_fixture)
{
    objects.push_back(create_object<game_object>(shm, [&](game_object& game) { game.status = game_status::finished; }));

    mocks
        .ExpectCallOverload(game_service,
                            (game_service_i::view_type(game_service_i::*)() const) & game_service_i::get_games)
        .Return({ objects.begin(), objects.end() });

    betting_api_impl api(*factory, game_dba, matched_bet_dba);
    std::vector<game_api_object> games = api.get_games(game_filter::finished);

    BOOST_REQUIRE_EQUAL(games.size(), 2);
    BOOST_CHECK(games[0].status == game_status::finished);
    BOOST_CHECK(games[1].status == game_status::finished);
}

BOOST_FIXTURE_TEST_CASE(return_games_not_finished_status, get_games_fixture)
{
    mocks
        .ExpectCallOverload(game_service,
                            (game_service_i::view_type(game_service_i::*)() const) & game_service_i::get_games)
        .Return({ objects.begin(), objects.end() });

    betting_api_impl api(*factory, game_dba, matched_bet_dba);
    std::vector<game_api_object> games = api.get_games(game_filter::not_finished);

    BOOST_REQUIRE_EQUAL(games.size(), 2);
    BOOST_CHECK(games[0].status == game_status::created);
    BOOST_CHECK(games[1].status == game_status::started);
}

BOOST_FIXTURE_TEST_CASE(throw_exception_when_limit_is_negative, get_games_fixture)
{
    betting_api_impl api(*factory, game_dba, matched_bet_dba);

    BOOST_REQUIRE_THROW(api.get_pending_bets(0, -1), fc::assert_exception);
    BOOST_REQUIRE_THROW(api.get_matched_bets(0, -1), fc::assert_exception);
}

BOOST_FIXTURE_TEST_CASE(throw_exception_when_limit_gt_than_max_limit, get_games_fixture)
{
    const auto max_limit = 100;

    betting_api_impl api(*factory, game_dba, matched_bet_dba, max_limit);

    BOOST_REQUIRE_THROW(api.get_pending_bets(0, max_limit + 1), fc::assert_exception);
    BOOST_REQUIRE_THROW(api.get_matched_bets(0, max_limit + 1), fc::assert_exception);
}

BOOST_FIXTURE_TEST_CASE(dont_throw_when_limit_is_zero, get_games_fixture)
{
    betting_api_impl api(*factory, game_dba, matched_bet_dba);

    std::vector<pending_bet_object> pbets;
    std::vector<matched_bet_object> mbets;

    mocks
        .OnCallOverload(pending_bet_service,
                        (pending_bet_service_i::view_type(pending_bet_service_i::*)(pending_bet_id_type) const)
                            & pending_bet_service_i::get_bets)
        .With(_)
        .ReturnByRef({ pbets.begin(), pbets.end() });

    mocks
        .OnCallOverload(matched_bet_service,
                        (matched_bet_service_i::view_type(matched_bet_service_i::*)(matched_bet_id_type) const)
                            & matched_bet_service_i::get_bets)
        .With(_)
        .ReturnByRef({ mbets.begin(), mbets.end() });

    BOOST_REQUIRE_NO_THROW(api.get_pending_bets(0, 0));
    BOOST_REQUIRE_NO_THROW(api.get_matched_bets(0, 0));
}

BOOST_FIXTURE_TEST_CASE(dont_throw_when_limit_eq_max, get_games_fixture)
{
    const auto max_limit = 100;

    betting_api_impl api(*factory, game_dba, matched_bet_dba, max_limit);

    std::vector<pending_bet_object> pbets;
    std::vector<matched_bet_object> mbets;

    mocks
        .OnCallOverload(pending_bet_service,
                        (pending_bet_service_i::view_type(pending_bet_service_i::*)(pending_bet_id_type) const)
                            & pending_bet_service_i::get_bets)
        .With(_)
        .ReturnByRef({ pbets.begin(), pbets.end() });

    mocks
        .OnCallOverload(matched_bet_service,
                        (matched_bet_service_i::view_type(matched_bet_service_i::*)(matched_bet_id_type) const)
                            & matched_bet_service_i::get_bets)
        .With(_)
        .ReturnByRef({ mbets.begin(), mbets.end() });

    BOOST_REQUIRE_NO_THROW(api.get_pending_bets(0, max_limit));
    BOOST_REQUIRE_NO_THROW(api.get_matched_bets(0, max_limit));
}

template <typename T> struct get_bets_fixture : public fixture
{
    get_bets_fixture()
    {
        init();

        objects.push_back(create_object<T>(shm, [&](auto& bet) { bet.id = 0; }));
        objects.push_back(create_object<T>(shm, [&](auto& bet) { bet.id = 1; }));
        objects.push_back(create_object<T>(shm, [&](auto& bet) { bet.id = 2; }));
    }

    std::vector<T> objects;
};

BOOST_FIXTURE_TEST_CASE(check_get_pending_bets_from_arg, get_bets_fixture<pending_bet_object>)
{
    pending_bet_id_type from = 0;
    mocks
        .ExpectCallOverload(pending_bet_service,
                            (pending_bet_service_i::view_type(pending_bet_service_i::*)(pending_bet_id_type) const)
                                & pending_bet_service_i::get_bets)
        .With(from)
        .Return({ objects.begin(), objects.end() });

    betting_api_impl api(*factory, game_dba, matched_bet_dba);
    api.get_pending_bets(from, 1);
}

BOOST_FIXTURE_TEST_CASE(get_one_pending_bet, get_bets_fixture<pending_bet_object>)
{
    mocks
        .ExpectCallOverload(pending_bet_service,
                            (pending_bet_service_i::view_type(pending_bet_service_i::*)(pending_bet_id_type) const)
                                & pending_bet_service_i::get_bets)
        .With(_)
        .Return({ objects.begin(), objects.end() });

    betting_api_impl api(*factory, game_dba, matched_bet_dba);
    auto bets = api.get_pending_bets(0, 1);

    BOOST_REQUIRE_EQUAL(bets.size(), 1);

    BOOST_CHECK(bets[0].id == 0u);
}

BOOST_FIXTURE_TEST_CASE(get_all_pending_bets, get_bets_fixture<pending_bet_object>)
{
    mocks
        .ExpectCallOverload(pending_bet_service,
                            (pending_bet_service_i::view_type(pending_bet_service_i::*)(pending_bet_id_type) const)
                                & pending_bet_service_i::get_bets)
        .With(_)
        .Return({ objects.begin(), objects.end() });

    betting_api_impl api(*factory, game_dba, matched_bet_dba);
    auto bets = api.get_pending_bets(0, 100);

    BOOST_REQUIRE_EQUAL(bets.size(), 3);

    BOOST_CHECK(bets[0].id == 0u);
    BOOST_CHECK(bets[1].id == 1u);
    BOOST_CHECK(bets[2].id == 2u);
}

BOOST_FIXTURE_TEST_CASE(check_get_matched_bets_from_arg, get_bets_fixture<matched_bet_object>)
{
    matched_bet_id_type from = 0;
    mocks
        .ExpectCallOverload(matched_bet_service,
                            (matched_bet_service_i::view_type(matched_bet_service_i::*)(matched_bet_id_type) const)
                                & matched_bet_service_i::get_bets)
        .With(from)
        .Return({ objects.begin(), objects.end() });

    betting_api_impl api(*factory, game_dba, matched_bet_dba);
    api.get_matched_bets(from, 1);
}

BOOST_FIXTURE_TEST_CASE(get_one_matched_bet, get_bets_fixture<matched_bet_object>)
{
    mocks
        .ExpectCallOverload(matched_bet_service,
                            (matched_bet_service_i::view_type(matched_bet_service_i::*)(matched_bet_id_type) const)
                                & matched_bet_service_i::get_bets)
        .With(_)
        .Return({ objects.begin(), objects.end() });

    betting_api_impl api(*factory, game_dba, matched_bet_dba);
    auto bets = api.get_matched_bets(0, 1);

    BOOST_REQUIRE_EQUAL(bets.size(), 1);

    BOOST_CHECK(bets[0].id == 0u);
}

BOOST_FIXTURE_TEST_CASE(get_all_matched_bets, get_bets_fixture<matched_bet_object>)
{
    mocks
        .ExpectCallOverload(matched_bet_service,
                            (matched_bet_service_i::view_type(matched_bet_service_i::*)(matched_bet_id_type) const)
                                & matched_bet_service_i::get_bets)
        .With(_)
        .Return({ objects.begin(), objects.end() });

    betting_api_impl api(*factory, game_dba, matched_bet_dba);
    auto bets = api.get_matched_bets(0, 100);

    BOOST_REQUIRE_EQUAL(bets.size(), 3);

    BOOST_CHECK(bets[0].id == 0u);
    BOOST_CHECK(bets[1].id == 1u);
    BOOST_CHECK(bets[2].id == 2u);
}

BOOST_AUTO_TEST_SUITE_END()
} // namespace betting_api_tests
