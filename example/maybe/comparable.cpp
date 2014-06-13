/*
@copyright Louis Dionne 2014
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 */

#include <boost/hana/detail/static_assert.hpp>
#include <boost/hana/maybe.hpp>
using namespace boost::hana;


int main() {
    //! [main]
    BOOST_HANA_STATIC_ASSERT(nothing == nothing);
    BOOST_HANA_STATIC_ASSERT(just(1) == just(1));
    BOOST_HANA_STATIC_ASSERT(just(1) != just(2));
    BOOST_HANA_STATIC_ASSERT(just(1) != nothing);
    //! [main]
}