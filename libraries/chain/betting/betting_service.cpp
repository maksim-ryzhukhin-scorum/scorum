#include <scorum/chain/betting/betting_service.hpp>

#include <boost/range/algorithm/set_algorithm.hpp>
#include <boost/range/algorithm/find_if.hpp>

#include <scorum/chain/schema/bet_objects.hpp>
#include <scorum/chain/schema/game_object.hpp>
#include <scorum/chain/schema/bet_objects.hpp>
#include <scorum/chain/schema/betting_property_object.hpp>

#include <scorum/chain/betting/betting_math.hpp>

#include <scorum/chain/services/account.hpp>

#include <scorum/chain/dba/db_accessor_factory.hpp>
#include <scorum/chain/dba/db_accessor.hpp>

#include <scorum/utils/range/unwrap_ref_wrapper_adaptor.hpp>
#include <scorum/utils/algorithm/foreach_mut.hpp>

#include <scorum/utils/collect_range_adaptor.hpp>

namespace scorum {
namespace chain {

betting_service_i::~betting_service_i() = default;

betting_service::betting_service(data_service_factory_i& db,
                                 database_virtual_operations_emmiter_i& virt_op_emitter,
                                 dba::db_accessor<betting_property_object>& betting_property_dba,
                                 dba::db_accessor<matched_bet_object>& matched_bet_dba,
                                 dba::db_accessor<pending_bet_object>& pending_bet_dba,
                                 dba::db_accessor<game_object>& game_dba,
                                 dba::db_accessor<dynamic_global_property_object>& dprop_dba,
                                 dba::db_accessor<bet_uuid_history_object>& uuid_hist_dba)
    : _account_svc(db.account_service())
    , _virt_op_emitter(virt_op_emitter)
    , _betting_property_dba(betting_property_dba)
    , _matched_bet_dba(matched_bet_dba)
    , _pending_bet_dba(pending_bet_dba)
    , _game_dba(game_dba)
    , _dprop_dba(dprop_dba)
    , _uuid_hist_dba(uuid_hist_dba)
{
}

bool betting_service::is_betting_moderator(const account_name_type& account_name) const
{
    try
    {
        return _betting_property_dba.get().moderator == account_name;
    }
    FC_CAPTURE_LOG_AND_RETHROW((account_name))
}

const pending_bet_object& betting_service::create_pending_bet(const account_name_type& better,
                                                              const asset& stake,
                                                              odds odds,
                                                              const wincase_type& wincase,
                                                              game_id_type game,
                                                              uuid_type bet_uuid,
                                                              pending_bet_kind kind)
{
    const auto& better_acc = _account_svc.get_account(better);
    FC_ASSERT(better_acc.balance >= stake, "Insufficient funds");

    _uuid_hist_dba.create([&](bet_uuid_history_object& o) { o.uuid = bet_uuid; });

    const auto& bet = _pending_bet_dba.create([&](pending_bet_object& o) {
        o.game = game;
        o.market = create_market(wincase);
        o.data.uuid = bet_uuid;
        o.data.stake = stake;
        o.data.bet_odds = odds;
        o.data.created = _dprop_dba.get().time;
        o.data.better = better;
        o.data.kind = kind;
        o.data.wincase = wincase;
    });

    _dprop_dba.update([&](auto& obj) { obj.betting_stats.pending_bets_volume += stake; });

    _account_svc.decrease_balance(better_acc, stake);

    return bet;
}

void betting_service::cancel_game(game_id_type game_id)
{
    auto matched_bets = _matched_bet_dba.get_range_by<by_game_id_market>(game_id);
    FC_ASSERT(matched_bets.empty(), "Cannot cancel game which has associated bets");

    auto pending_bets = _matched_bet_dba.get_range_by<by_game_id_market>(game_id);
    FC_ASSERT(pending_bets.empty(), "Cannot cancel game which has associated bets");

    const auto& game = _game_dba.get_by<by_id>(game_id);
    _game_dba.remove(game);
}

void betting_service::cancel_bets(game_id_type game_id)
{
    cancel_pending_bets(game_id);
    cancel_matched_bets(game_id);
}

void betting_service::cancel_bets(game_id_type game_id, fc::time_point_sec created_after)
{
    using namespace dba;

    auto lower = std::make_tuple(game_id, created_after);
    auto upper = game_id;
    auto matched_bets = _matched_bet_dba.get_range_by<by_game_id_created>(lower <= _x, _x <= upper);
    auto pending_bets = _pending_bet_dba.get_range_by<by_game_id_created>(lower <= _x, _x <= upper);

    const auto& game = _game_dba.get_by<by_id>(game_id);

    cancel_pending_bets(pending_bets, game.uuid);

    for (const matched_bet_object& matched_bet : matched_bets)
    {
        if (matched_bet.bet1_data.created >= created_after)
            return_bet(matched_bet.bet1_data, game.uuid);
        else
            restore_pending_bet(matched_bet.bet1_data, game.uuid);

        if (matched_bet.bet2_data.created >= created_after)
            return_bet(matched_bet.bet2_data, game.uuid);
        else
            restore_pending_bet(matched_bet.bet2_data, game.uuid);
    }

    _matched_bet_dba.remove_all(matched_bets);
}

void betting_service::cancel_bets(game_id_type game_id, const fc::flat_set<market_type>& cancelled_markets)
{
    // clang-format off
    struct less
    {
        bool operator()(const pending_bet_object& b, const market_type& m) const { return b.market < m; }
        bool operator()(const market_type& m, const pending_bet_object& b) const { return m < b.market; }
        bool operator()(const matched_bet_object& b, const market_type& m) const { return b.market < m; }
        bool operator()(const market_type& m, const matched_bet_object& b) const { return m < b.market; }
    };
    // clang-format on

    const auto& game = _game_dba.get_by<by_id>(game_id);

    auto pending_bets = _pending_bet_dba.get_range_by<by_game_id_market>(game_id);

    std::vector<std::reference_wrapper<const pending_bet_object>> filtered_pending_bets;
    boost::set_intersection(pending_bets, cancelled_markets, std::back_inserter(filtered_pending_bets), less{});

    cancel_pending_bets(utils::unwrap_ref_wrapper(filtered_pending_bets), game.uuid);

    auto matched_bets = _matched_bet_dba.get_range_by<by_game_id_market>(game_id);

    std::vector<std::reference_wrapper<const matched_bet_object>> filtered_matched_bets;
    boost::set_intersection(matched_bets, cancelled_markets, std::back_inserter(filtered_matched_bets), less{});

    cancel_matched_bets(utils::unwrap_ref_wrapper(filtered_matched_bets), game.uuid);
}

void betting_service::cancel_pending_bet(pending_bet_id_type id)
{
    const auto& pending_bet = _pending_bet_dba.get_by<by_id>(id);
    const auto& game = _game_dba.get_by<by_id>(pending_bet.game);

    cancel_pending_bet(pending_bet, game.uuid);
}

void betting_service::cancel_pending_bets(game_id_type game_id)
{
    auto pending_bets = _pending_bet_dba.get_range_by<by_game_id_market>(game_id);
    const auto& game = _game_dba.get_by<by_id>(game_id);

    cancel_pending_bets(pending_bets, game.uuid);
}

void betting_service::cancel_pending_bets(game_id_type game_id, pending_bet_kind kind)
{
    auto pending_bets = _pending_bet_dba.get_range_by<by_game_id_kind>(std::make_tuple(game_id, kind));
    const auto& game = _game_dba.get_by<by_id>(game_id);

    cancel_pending_bets(pending_bets, game.uuid);
}

void betting_service::cancel_pending_bets(utils::bidir_range<const pending_bet_object> bets, uuid_type game_uuid)
{
    utils::foreach_mut(bets, [&](const pending_bet_object& bet) { //
        cancel_pending_bet(bet, game_uuid);
    });
}

void betting_service::cancel_matched_bets(game_id_type game_id)
{
    auto matched_bets = _matched_bet_dba.get_range_by<by_game_id_market>(game_id);
    const auto& game = _game_dba.get_by<by_id>(game_id);

    cancel_matched_bets(matched_bets, game.uuid);
}

void betting_service::cancel_matched_bets(utils::bidir_range<const matched_bet_object> bets, uuid_type game_uuid)
{
    utils::foreach_mut(bets, [&](const matched_bet_object& bet) { //
        cancel_matched_bet(bet, game_uuid);
    });
}

void betting_service::cancel_pending_bet(const pending_bet_object& bet, uuid_type game_uuid)
{
    _account_svc.increase_balance(bet.data.better, bet.data.stake);

    push_pending_bet_cancelled_op(bet.data, game_uuid);

    _dprop_dba.update([&](auto& o) { o.betting_stats.pending_bets_volume -= bet.data.stake; });

    _pending_bet_dba.remove(bet);
}

void betting_service::cancel_matched_bet(const matched_bet_object& bet, uuid_type game_uuid)
{
    return_bet(bet.bet1_data, game_uuid);
    return_bet(bet.bet2_data, game_uuid);

    _matched_bet_dba.remove(bet);
}

void betting_service::return_bet(const bet_data& bet, uuid_type game_uuid)
{
    _account_svc.increase_balance(bet.better, bet.stake);

    push_matched_bet_cancelled_op(bet, game_uuid);

    _dprop_dba.update([&](auto& o) { o.betting_stats.matched_bets_volume -= bet.stake; });
}

void betting_service::restore_pending_bet(const bet_data& bet, uuid_type game_uuid)
{
    const auto& game = _game_dba.get_by<by_uuid>(game_uuid);
    auto bets = _pending_bet_dba.get_range_by<by_game_id_better>(std::make_tuple(game.id, bet.better));

    // clang-format off
    auto found_it = boost::find_if(bets, [&](const pending_bet_object& o) {
        return o.data.created == bet.created
            && o.data.bet_odds == bet.bet_odds
            && o.data.kind == bet.kind
            && !(o.data.wincase < bet.wincase)
            && !(bet.wincase < o.data.wincase);
    });
    // clang-format on

    if (found_it != bets.end())
    {
        _pending_bet_dba.update(*found_it, [&](pending_bet_object& o) { o.data.stake += bet.stake; });
    }
    else
    {
        _pending_bet_dba.create([&](pending_bet_object& o) {
            o.game = game.id;
            o.market = create_market(bet.wincase);
            o.data = bet;
        });
    }

    _dprop_dba.update([&](auto& o) {
        o.betting_stats.pending_bets_volume += bet.stake;
        o.betting_stats.matched_bets_volume -= bet.stake;
    });

    _virt_op_emitter.push_virtual_operation(bet_restored_operation{ game_uuid, bet.better, bet.uuid, bet.stake });
}

void betting_service::push_matched_bet_cancelled_op(const bet_data& bet, uuid_type game_uuid)
{
    _virt_op_emitter.push_virtual_operation(
        bet_cancelled_operation{ game_uuid, bet.better, bet.uuid, bet.stake, bet_kind::matched });
}

void betting_service::push_pending_bet_cancelled_op(const bet_data& bet, uuid_type game_uuid)
{
    _virt_op_emitter.push_virtual_operation(
        bet_cancelled_operation{ game_uuid, bet.better, bet.uuid, bet.stake, bet_kind::pending });
}
}
}
