/*
@copyright Louis Dionne 2014
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 */

#include <boost/hana/detail/sandbox/map.hpp>

#include <boost/hana/detail/constexpr.hpp>
#include <boost/hana/detail/static_assert.hpp>
#include <boost/hana/integral.hpp>
#include <boost/hana/maybe.hpp>
#include <boost/hana/pair.hpp>
using namespace boost::hana;


template <int i> constexpr auto k = int_<i>;
template <int i> constexpr auto v = int_<-i>;

template <int i, int j>
BOOST_HANA_CONSTEXPR_LAMBDA auto p = pair(k<i>, v<j>);

int main() {
    BOOST_HANA_STATIC_ASSERT(lookup(k<1>, map()) == nothing);
    BOOST_HANA_STATIC_ASSERT(lookup(k<1>, map(p<1, 1>)) == just(v<1>));
    BOOST_HANA_STATIC_ASSERT(lookup(k<2>, map(p<1, 1>)) == nothing);

    BOOST_HANA_STATIC_ASSERT(lookup(k<2>, map(p<1, 1>, p<2, 2>)) == just(v<2>));
    BOOST_HANA_STATIC_ASSERT(lookup(k<3>, map(p<1, 1>, p<2, 2>)) == nothing);
}