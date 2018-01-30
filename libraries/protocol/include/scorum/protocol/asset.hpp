#pragma once
#include <scorum/protocol/types.hpp>
#include <scorum/protocol/config.hpp>

namespace scorum {
namespace protocol {

using asset_symbol_type = uint64_t;
using share_value_type = int64_t;
using share_type = safe<share_value_type>;

struct asset
{
    asset()
        : amount(0)
        , symbol(SCORUM_SYMBOL)
    {
        // used for fc::variant
    }

    asset(share_type a, asset_symbol_type id)
        : amount(a)
        , symbol(id)
    {
    }

    share_type amount;
    asset_symbol_type symbol;

    static asset maximum(asset_symbol_type id)
    {
        return asset(SCORUM_MAX_SHARE_SUPPLY, id);
    }
    static asset min(asset_symbol_type id)
    {
        return asset(0, id);
    }

    double to_real() const
    {
        return double(amount.value) / precision();
    }

    uint8_t decimals() const;
    std::string symbol_name() const;
    int64_t precision() const;
    void set_decimals(uint8_t d);

    static asset from_string(const std::string& from);
    std::string to_string() const;

    template <typename T> asset& operator+=(const T& o_amount)
    {
        amount += o_amount;
        return *this;
    }
    asset& operator+=(const asset& o)
    {
        FC_ASSERT(symbol == o.symbol);
        return *this += o.amount;
    }
    template <typename T> asset& operator-=(const T& o_amount)
    {
        amount -= o_amount;
        return *this;
    }
    asset& operator-=(const asset& o)
    {
        FC_ASSERT(symbol == o.symbol);
        return *this -= o.amount;
    }
    asset operator-() const
    {
        return asset(-amount, symbol);
    }
    template <typename T> asset& operator*=(const T& o_amount)
    {
        amount *= o_amount;
        return *this;
    }
    template <typename T> asset& operator/=(const T& o_amount)
    {
        amount /= o_amount;
        return *this;
    }

    friend bool operator==(const asset& a, const asset& b)
    {
        return std::tie(a.symbol, a.amount) == std::tie(b.symbol, b.amount);
    }
    friend bool operator<(const asset& a, const asset& b)
    {
        FC_ASSERT(a.symbol == b.symbol);
        return std::tie(a.amount, a.symbol) < std::tie(b.amount, b.symbol);
    }
    friend bool operator<=(const asset& a, const asset& b)
    {
        return (a == b) || (a < b);
    }
    friend bool operator!=(const asset& a, const asset& b)
    {
        return !(a == b);
    }
    friend bool operator>(const asset& a, const asset& b)
    {
        return !(a <= b);
    }
    friend bool operator>=(const asset& a, const asset& b)
    {
        return !(a < b);
    }
    template <typename T> friend asset operator+(const asset& a, const T& b_amount)
    {
        asset ret(a);
        ret += b_amount;
        return ret;
    }
    friend asset operator+(const asset& a, const asset& b)
    {
        asset ret(a);
        ret += b;
        return ret;
    }
    template <typename T> friend asset operator-(const asset& a, const T& b_amount)
    {
        asset ret(a);
        ret -= b_amount;
        return ret;
    }
    friend asset operator-(const asset& a, const asset& b)
    {
        asset ret(a);
        ret -= b;
        return ret;
    }
    template <typename T> friend asset operator*(const asset& a, const T& b_amount)
    {
        asset ret(a);
        ret *= b_amount;
        return ret;
    }
    template <typename T> friend asset operator/(const asset& a, const T& b_amount)
    {
        asset ret(a);
        ret /= b_amount;
        return ret;
    }
};

template <typename Stream> Stream& operator<<(Stream& stream, const scorum::protocol::asset& a)
{
    stream << a.to_string();
    return stream;
}

template <typename Stream> Stream& operator>>(Stream& stream, scorum::protocol::asset& a)
{
    std::string str;
    stream >> str;
    a = scorum::protocol::asset::from_string(str);
    return stream;
}

} // namespace protocol
} // namespace scorum

namespace fc {

inline void to_variant(const scorum::protocol::asset& var, fc::variant& vo)
{
    vo = var.to_string();
}

inline void from_variant(const fc::variant& var, scorum::protocol::asset& vo)
{
    vo = scorum::protocol::asset::from_string(var.as_string());
}

} // namespace fc

FC_REFLECT_TYPENAME(scorum::protocol::share_type)

FC_REFLECT(scorum::protocol::asset, (amount)(symbol))
