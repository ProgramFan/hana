/*
@copyright Louis Dionne 2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 */

#ifndef BOOST_HANA_TEST_LAWS_FOLDABLE_HPP
#define BOOST_HANA_TEST_LAWS_FOLDABLE_HPP

#include <boost/hana/assert.hpp>
#include <boost/hana/comparable.hpp>
#include <boost/hana/config.hpp>
#include <boost/hana/core/models.hpp>
#include <boost/hana/core/operators.hpp>
#include <boost/hana/core/when.hpp>
#include <boost/hana/foldable.hpp>
#include <boost/hana/functional/capture.hpp>
#include <boost/hana/functional/curry.hpp>
#include <boost/hana/functional/demux.hpp>
#include <boost/hana/functional/flip.hpp>
#include <boost/hana/integral_constant.hpp>
#include <boost/hana/lazy.hpp>

#include <laws/base.hpp>
#include <test/identity.hpp>

#include <vector>


namespace boost { namespace hana { namespace test {
    template <typename F, typename = when<true>>
    struct TestFoldable : TestFoldable<F, laws> {
        using TestFoldable<F, laws>::TestFoldable;
    };

    template <typename F>
    struct TestFoldable<F, laws> {
        static_assert(_models<Foldable, F>{}, "");

        template <typename Foldables>
        TestFoldable(Foldables foldables) {
            hana::for_each(foldables, [](auto xs) {
                _injection<0> f{};
                ct_eq<999> s{};

                // equivalence of size(xs) and length(xs)
                BOOST_HANA_CHECK(hana::equal(
                    hana::length(xs),
                    hana::size(xs)
                ));

                // equivalence of fold with fold.left and
                // of reverse_fold with fold.right
                BOOST_HANA_CHECK(hana::equal(
                    hana::fold(xs, s, f),
                    hana::fold.left(xs, s, f)
                ));

                BOOST_HANA_CHECK(hana::equal(
                    hana::reverse_fold(xs, s, f),
                    hana::fold.right(xs, s, hana::flip(f))
                ));

                only_when_(hana::not_equal(hana::length(xs), size_t<0>),
                hana::lazy([](auto f, auto xs) {
                    BOOST_HANA_CHECK(hana::equal(
                        hana::fold(xs, f),
                        hana::fold.left(xs, f)
                    ));

                    BOOST_HANA_CHECK(hana::equal(
                        hana::reverse_fold(xs, f),
                        hana::fold.right(xs, hana::flip(f))
                    ));
                })(f, xs));

                // equivalence of count(xs, val) and count_if(xs, equal.to(val))
                struct not_there { };
                BOOST_HANA_CHECK(hana::equal(
                    hana::count(xs, not_there{}),
                    hana::count_if(xs, equal.to(not_there{}))
                ));

                hana::for_each(xs, hana::capture(xs)([](auto xs, auto value) {
                    BOOST_HANA_CHECK(hana::equal(
                        hana::count(xs, value),
                        hana::count_if(xs, equal.to(value))
                    ));
                }));
            });
        }
    };

    template <typename P>
    struct TestFoldable<P, when<_models<Product, P>{}>> : TestFoldable<P, laws> {
        template <typename Products>
        TestFoldable(Products products) : TestFoldable<P, laws>{products} {
            hana::for_each(products, [](auto p) {
                _injection<0> f{};

                BOOST_HANA_CHECK(hana::equal(
                    hana::unpack(p, f),
                    f(hana::first(p), hana::second(p))
                ));
            });
        }
    };

    template <typename S>
    struct TestFoldable<S, when<_models<Sequence, S>{}>> : TestFoldable<S, laws> {
        template <int i>
        using x = _constant<i>;

        template <int i>
        using ord = _constant<i>;

        struct undefined { };

        template <typename Xs>
        TestFoldable(Xs xs) : TestFoldable<S, laws>{xs} {
            _injection<0> f{};
            auto z = x<999>{};
            constexpr auto list = make<S>;

            //////////////////////////////////////////////////////////////////
            // fold.left (with initial state)
            //////////////////////////////////////////////////////////////////
            BOOST_HANA_CONSTANT_CHECK(equal(
                fold.left(list(), z, undefined{}),
                z
            ));

            BOOST_HANA_CONSTANT_CHECK(equal(
                fold.left(list(x<1>{}), z, f),
                f(z, x<1>{})
            ));
            BOOST_HANA_CONSTANT_CHECK(equal(
                fold.left(list(x<1>{}, x<2>{}), z, f),
                f(f(z, x<1>{}), x<2>{})
            ));
            BOOST_HANA_CONSTANT_CHECK(equal(
                fold.left(list(x<1>{}, x<2>{}, x<3>{}), z, f),
                f(f(f(z, x<1>{}), x<2>{}), x<3>{})
            ));
            BOOST_HANA_CONSTANT_CHECK(equal(
                fold.left(list(x<1>{}, x<2>{}, x<3>{}, x<4>{}), z, f),
                f(f(f(f(z, x<1>{}), x<2>{}), x<3>{}), x<4>{})
            ));

            BOOST_HANA_CONSTEXPR_CHECK(equal(
                fold.left(list(1), z, f),
                f(z, 1)
            ));
            BOOST_HANA_CONSTEXPR_CHECK(equal(
                fold.left(list(1, '2'), z, f),
                f(f(z, 1), '2')
            ));
            BOOST_HANA_CONSTEXPR_CHECK(equal(
                fold.left(list(1, '2', 3.3), z, f),
                f(f(f(z, 1), '2'), 3.3)
            ));
            BOOST_HANA_CONSTEXPR_CHECK(equal(
                fold.left(list(1, '2', 3.3, 4.4f), z, f),
                f(f(f(f(z, 1), '2'), 3.3), 4.4f)
            ));

            //////////////////////////////////////////////////////////////////
            // fold.left (without initial state)
            //////////////////////////////////////////////////////////////////
            BOOST_HANA_CONSTANT_CHECK(equal(
                fold.left(list(z), undefined{}),
                z
            ));
            BOOST_HANA_CONSTANT_CHECK(equal(
                fold.left(list(z, x<2>{}), f),
                f(z, x<2>{})
            ));
            BOOST_HANA_CONSTANT_CHECK(equal(
                fold.left(list(z, x<2>{}, x<3>{}), f),
                f(f(z, x<2>{}), x<3>{})
            ));
            BOOST_HANA_CONSTANT_CHECK(equal(
                fold.left(list(z, x<2>{}, x<3>{}, x<4>{}), f),
                f(f(f(z, x<2>{}), x<3>{}), x<4>{})
            ));

            //////////////////////////////////////////////////////////////////
            // fold.right (with initial state)
            //////////////////////////////////////////////////////////////////
            BOOST_HANA_CONSTANT_CHECK(equal(
                fold.right(list(), z, undefined{}),
                z
            ));

            BOOST_HANA_CONSTANT_CHECK(equal(
                fold.right(list(x<0>{}), z, f),
                f(x<0>{}, z)
            ));
            BOOST_HANA_CONSTANT_CHECK(equal(
                fold.right(list(x<0>{}, x<1>{}), z, f),
                f(x<0>{}, f(x<1>{}, z))
            ));
            BOOST_HANA_CONSTANT_CHECK(equal(
                fold.right(list(x<0>{}, x<1>{}, x<2>{}), z, f),
                f(x<0>{}, f(x<1>{}, f(x<2>{}, z)))
            ));
            BOOST_HANA_CONSTANT_CHECK(equal(
                fold.right(list(x<0>{}, x<1>{}, x<2>{}, x<3>{}), z, f),
                f(x<0>{}, f(x<1>{}, f(x<2>{}, f(x<3>{}, z))))
            ));


            BOOST_HANA_CONSTEXPR_CHECK(equal(
                fold.right(list(1), z, f),
                f(1, z)
            ));
            BOOST_HANA_CONSTEXPR_CHECK(equal(
                fold.right(list(1, '2'), z, f),
                f(1, f('2', z))
            ));
            BOOST_HANA_CONSTEXPR_CHECK(equal(
                fold.right(list(1, '2', 3.3), z, f),
                f(1, f('2', f(3.3, z)))
            ));
            BOOST_HANA_CONSTEXPR_CHECK(equal(
                fold.right(list(1, '2', 3.3, 4.4f), z, f),
                f(1, f('2', f(3.3, f(4.4f, z))))
            ));

            //////////////////////////////////////////////////////////////////
            // fold.right (without initial state)
            //////////////////////////////////////////////////////////////////
            BOOST_HANA_CONSTANT_CHECK(equal(
                fold.right(list(z), undefined{}),
                z
            ));
            BOOST_HANA_CONSTANT_CHECK(equal(
                fold.right(list(x<1>{}, z), f),
                f(x<1>{}, z)
            ));
            BOOST_HANA_CONSTANT_CHECK(equal(
                fold.right(list(x<1>{}, x<2>{}, z), f),
                f(x<1>{}, f(x<2>{}, z))
            ));
            BOOST_HANA_CONSTANT_CHECK(equal(
                fold.right(list(x<1>{}, x<2>{}, x<3>{}, z), f),
                f(x<1>{}, f(x<2>{}, f(x<3>{}, z)))
            ));

            //////////////////////////////////////////////////////////////////
            // monadic_fold.left (with initial state)
            //////////////////////////////////////////////////////////////////
            {
                using M = test::Identity;
                auto f = hana::demux(test::identity)(test::_injection<0>{});
                auto s = x<999>{};
                auto fp = hana::curry<2>(hana::flip(f));

                BOOST_HANA_CONSTANT_CHECK(equal(
                    monadic_fold<M>.left(list(), s, undefined{}),
                    lift<M>(s)
                ));

                BOOST_HANA_CONSTANT_CHECK(equal(
                    monadic_fold<M>.left(list(x<1>{}), s, f),
                    f(s, x<1>{})
                ));

                BOOST_HANA_CONSTANT_CHECK(equal(
                    monadic_fold<M>.left(list(x<1>{}, x<2>{}), s, f),
                    chain(f(s, x<1>{}), fp(x<2>{}))
                ));

                BOOST_HANA_CONSTANT_CHECK(equal(
                    monadic_fold<M>.left(list(x<1>{}, x<2>{}, x<3>{}), s, f),
                    chain(chain(f(s, x<1>{}), fp(x<2>{})), fp(x<3>{}))
                ));

                BOOST_HANA_CONSTANT_CHECK(equal(
                    monadic_fold<M>.left(list(x<1>{}, x<2>{}, x<3>{}, x<4>{}), s, f),
                    chain(chain(chain(f(s, x<1>{}), fp(x<2>{})), fp(x<3>{})), fp(x<4>{}))
                ));

                BOOST_HANA_CONSTANT_CHECK(equal(
                    monadic_fold<M>.left(list(x<1>{}, x<2>{}, x<3>{}, x<4>{}, x<5>{}), s, f),
                    chain(chain(chain(chain(f(s, x<1>{}), fp(x<2>{})), fp(x<3>{})), fp(x<4>{})), fp(x<5>{}))
                ));
            }

            //////////////////////////////////////////////////////////////////
            // monadic_fold.left (without initial state)
            //////////////////////////////////////////////////////////////////
            {
                using M = test::Identity;
                auto f = hana::demux(test::identity)(test::_injection<0>{});
                auto fp = hana::curry<2>(hana::flip(f));

                BOOST_HANA_CONSTANT_CHECK(equal(
                    monadic_fold<M>.left(list(x<1>{}), undefined{}),
                    lift<M>(x<1>{})
                ));

                BOOST_HANA_CONSTANT_CHECK(equal(
                    monadic_fold<M>.left(list(x<1>{}, x<2>{}), f),
                    f(x<1>{}, x<2>{})
                ));

                BOOST_HANA_CONSTANT_CHECK(equal(
                    monadic_fold<M>.left(list(x<1>{}, x<2>{}, x<3>{}), f),
                    chain(f(x<1>{}, x<2>{}), fp(x<3>{}))
                ));

                BOOST_HANA_CONSTANT_CHECK(equal(
                    monadic_fold<M>.left(list(x<1>{}, x<2>{}, x<3>{}, x<4>{}), f),
                    chain(chain(f(x<1>{}, x<2>{}), fp(x<3>{})), fp(x<4>{}))
                ));

                BOOST_HANA_CONSTANT_CHECK(equal(
                    monadic_fold<M>.left(list(x<1>{}, x<2>{}, x<3>{}, x<4>{}, x<5>{}), f),
                    chain(chain(chain(f(x<1>{}, x<2>{}), fp(x<3>{})), fp(x<4>{})), fp(x<5>{}))
                ));
            }

            //////////////////////////////////////////////////////////////////
            // monadic_fold.right (with initial state)
            //////////////////////////////////////////////////////////////////
            {
                using M = test::Identity;
                auto f = hana::demux(test::identity)(test::_injection<0>{});
                auto s = x<999>{};
                auto fp = hana::curry<2>(f);
                // flipping `chain` makes the right associativity easier to see
                auto chain = hana::flip(hana::chain);

                BOOST_HANA_CONSTANT_CHECK(equal(
                    monadic_fold<M>.right(list(), s, undefined{}),
                    lift<M>(s)
                ));

                BOOST_HANA_CONSTANT_CHECK(equal(
                    monadic_fold<M>.right(list(x<1>{}), s, f),
                    f(x<1>{}, s)
                ));

                BOOST_HANA_CONSTANT_CHECK(equal(
                    monadic_fold<M>.right(list(x<1>{}, x<2>{}), s, f),
                    chain(fp(x<1>{}), f(x<2>{}, s))
                ));

                BOOST_HANA_CONSTANT_CHECK(equal(
                    monadic_fold<M>.right(list(x<1>{}, x<2>{}, x<3>{}), s, f),
                    chain(fp(x<1>{}), chain(fp(x<2>{}), f(x<3>{}, s)))
                ));

                BOOST_HANA_CONSTANT_CHECK(equal(
                    monadic_fold<M>.right(list(x<1>{}, x<2>{}, x<3>{}, x<4>{}), s, f),
                    chain(fp(x<1>{}), chain(fp(x<2>{}), chain(fp(x<3>{}), f(x<4>{}, s))))
                ));

                BOOST_HANA_CONSTANT_CHECK(equal(
                    monadic_fold<M>.right(list(x<1>{}, x<2>{}, x<3>{}, x<4>{}, x<5>{}), s, f),
                    chain(fp(x<1>{}), chain(fp(x<2>{}), chain(fp(x<3>{}), chain(fp(x<4>{}), f(x<5>{}, s)))))
                ));
            }

            //////////////////////////////////////////////////////////////////
            // monadic_fold.right (without initial state)
            //////////////////////////////////////////////////////////////////
            {
                using M = test::Identity;
                auto f = hana::demux(test::identity)(test::_injection<0>{});
                auto fp = hana::curry<2>(f);
                // flipping `chain` makes the right associativity easier to see
                auto chain = hana::flip(hana::chain);

                BOOST_HANA_CONSTANT_CHECK(equal(
                    monadic_fold<M>.right(list(x<1>{}), undefined{}),
                    lift<M>(x<1>{})
                ));

                BOOST_HANA_CONSTANT_CHECK(equal(
                    monadic_fold<M>.right(list(x<1>{}, x<2>{}), f),
                    f(x<1>{}, x<2>{})
                ));

                BOOST_HANA_CONSTANT_CHECK(equal(
                    monadic_fold<M>.right(list(x<1>{}, x<2>{}, x<3>{}), f),
                    chain(fp(x<1>{}), f(x<2>{}, x<3>{}))
                ));

                BOOST_HANA_CONSTANT_CHECK(equal(
                    monadic_fold<M>.right(list(x<1>{}, x<2>{}, x<3>{}, x<4>{}), f),
                    chain(fp(x<1>{}), chain(fp(x<2>{}), f(x<3>{}, x<4>{})))
                ));

                BOOST_HANA_CONSTANT_CHECK(equal(
                    monadic_fold<M>.right(list(x<1>{}, x<2>{}, x<3>{}, x<4>{}, x<5>{}), f),
                    chain(fp(x<1>{}), chain(fp(x<2>{}), chain(fp(x<3>{}), f(x<4>{}, x<5>{}))))
                ));
            }

            //////////////////////////////////////////////////////////////////
            // for_each
            //////////////////////////////////////////////////////////////////
            auto check = [list](auto ...xs) {
                std::vector<int> seen{};
                hana::for_each(list(xs...), [&seen](int x) {
                    seen.push_back(x);
                });
                BOOST_HANA_RUNTIME_CHECK(seen == std::vector<int>{xs...});
            };
            check();
            check(0);
            check(0, 1);
            check(0, 1, 2);
            check(0, 1, 2, 3);
            check(0, 1, 2, 3, 4);

            //////////////////////////////////////////////////////////////////
            // length
            //////////////////////////////////////////////////////////////////
            BOOST_HANA_CONSTANT_CHECK(equal(
                length(list()), size_t<0>
            ));
            BOOST_HANA_CONSTANT_CHECK(equal(
                length(list(undefined{})), size_t<1>
            ));
            BOOST_HANA_CONSTANT_CHECK(equal(
                length(list(undefined{}, undefined{})), size_t<2>
            ));
            BOOST_HANA_CONSTANT_CHECK(equal(
                length(list(undefined{}, undefined{}, undefined{})), size_t<3>
            ));

            int i = 0; // non-constexpr
            BOOST_HANA_CONSTANT_CHECK(equal(
                length(list(i, i)),
                size_t<2>
            ));

            //////////////////////////////////////////////////////////////////
            // maximum (without a custom predicate)
            //////////////////////////////////////////////////////////////////
            {
                BOOST_HANA_CONSTANT_CHECK(equal(
                    maximum(list(ord<0>{})),
                    ord<0>{}
                ));

                BOOST_HANA_CONSTANT_CHECK(equal(
                    maximum(list(ord<0>{}, ord<1>{})),
                    ord<1>{}
                ));
                BOOST_HANA_CONSTANT_CHECK(equal(
                    maximum(list(ord<1>{}, ord<0>{})),
                    ord<1>{}
                ));

                BOOST_HANA_CONSTANT_CHECK(equal(
                    maximum(list(ord<0>{}, ord<1>{}, ord<2>{})),
                    ord<2>{}
                ));
                BOOST_HANA_CONSTANT_CHECK(equal(
                    maximum(list(ord<1>{}, ord<0>{}, ord<2>{})),
                    ord<2>{}
                ));
                BOOST_HANA_CONSTANT_CHECK(equal(
                    maximum(list(ord<1>{}, ord<2>{}, ord<0>{})),
                    ord<2>{}
                ));
                BOOST_HANA_CONSTANT_CHECK(equal(
                    maximum(list(ord<2>{}, ord<1>{}, ord<0>{})),
                    ord<2>{}
                ));
                BOOST_HANA_CONSTANT_CHECK(equal(
                    maximum(list(ord<2>{}, ord<0>{}, ord<1>{})),
                    ord<2>{}
                ));

                BOOST_HANA_CONSTANT_CHECK(equal(
                    maximum(list(ord<0>{}, ord<1>{}, ord<2>{}, ord<3>{})),
                    ord<3>{}
                ));
                BOOST_HANA_CONSTANT_CHECK(equal(
                    maximum(list(ord<1>{}, ord<0>{}, ord<2>{}, ord<3>{})),
                    ord<3>{}
                ));
                BOOST_HANA_CONSTANT_CHECK(equal(
                    maximum(list(ord<1>{}, ord<2>{}, ord<0>{}, ord<3>{})),
                    ord<3>{}
                ));
                BOOST_HANA_CONSTANT_CHECK(equal(
                    maximum(list(ord<1>{}, ord<2>{}, ord<3>{}, ord<0>{})),
                    ord<3>{}
                ));
                BOOST_HANA_CONSTANT_CHECK(equal(
                    maximum(list(ord<2>{}, ord<1>{}, ord<3>{}, ord<0>{})),
                    ord<3>{}
                ));
                BOOST_HANA_CONSTANT_CHECK(equal(
                    maximum(list(ord<2>{}, ord<3>{}, ord<1>{}, ord<0>{})),
                    ord<3>{}
                ));
                BOOST_HANA_CONSTANT_CHECK(equal(
                    maximum(list(ord<2>{}, ord<3>{}, ord<0>{}, ord<1>{})),
                    ord<3>{}
                ));
                BOOST_HANA_CONSTANT_CHECK(equal(
                    maximum(list(ord<3>{}, ord<2>{}, ord<0>{}, ord<1>{})),
                    ord<3>{}
                ));
                BOOST_HANA_CONSTANT_CHECK(equal(
                    maximum(list(ord<3>{}, ord<0>{}, ord<2>{}, ord<1>{})),
                    ord<3>{}
                ));
                BOOST_HANA_CONSTANT_CHECK(equal(
                    maximum(list(ord<3>{}, ord<0>{}, ord<1>{}, ord<2>{})),
                    ord<3>{}
                ));
                BOOST_HANA_CONSTANT_CHECK(equal(
                    maximum(list(ord<0>{}, ord<2>{}, ord<3>{}, ord<1>{})),
                    ord<3>{}
                ));
                BOOST_HANA_CONSTANT_CHECK(equal(
                    maximum(list(ord<0>{}, ord<3>{}, ord<1>{}, ord<2>{})),
                    ord<3>{}
                ));


                BOOST_HANA_CONSTEXPR_CHECK(equal(
                    maximum(list(int{0})),
                    int{0}
                ));

                BOOST_HANA_CONSTEXPR_CHECK(equal(
                    maximum(list(int{0}, long{1})),
                    long{1}
                ));
                BOOST_HANA_CONSTEXPR_CHECK(equal(
                    maximum(list(int{1}, long{0})),
                    int{1}
                ));

                BOOST_HANA_CONSTEXPR_CHECK(equal(
                    maximum(list(int{0}, 1ll, long{2})),
                    long{2}
                ));
                BOOST_HANA_CONSTEXPR_CHECK(equal(
                    maximum(list(int{1}, 0ll, long{2})),
                    long{2}
                ));
                BOOST_HANA_CONSTEXPR_CHECK(equal(
                    maximum(list(int{1}, 2ll, long{0})),
                    2ll
                ));
                BOOST_HANA_CONSTEXPR_CHECK(equal(
                    maximum(list(int{2}, 1ll, long{0})),
                    int{2}
                ));
                BOOST_HANA_CONSTEXPR_CHECK(equal(
                    maximum(list(int{2}, 0ll, long{1})),
                    int{2}
                ));

                BOOST_HANA_CONSTEXPR_CHECK(equal(
                    maximum(list(int{0}, 1ll, long{2}, short{3})),
                    short{3}
                ));
                BOOST_HANA_CONSTEXPR_CHECK(equal(
                    maximum(list(int{1}, 0ll, long{2}, short{3})),
                    short{3}
                ));
                BOOST_HANA_CONSTEXPR_CHECK(equal(
                    maximum(list(int{1}, 2ll, long{0}, short{3})),
                    short{3}
                ));
                BOOST_HANA_CONSTEXPR_CHECK(equal(
                    maximum(list(int{1}, 2ll, long{3}, short{0})),
                    long{3}
                ));
                BOOST_HANA_CONSTEXPR_CHECK(equal(
                    maximum(list(int{2}, 1ll, long{3}, short{0})),
                    long{3}
                ));
                BOOST_HANA_CONSTEXPR_CHECK(equal(
                    maximum(list(int{2}, 3ll, long{1}, short{0})),
                    3ll
                ));
                BOOST_HANA_CONSTEXPR_CHECK(equal(
                    maximum(list(int{2}, 3ll, long{0}, short{1})),
                    3ll
                ));
                BOOST_HANA_CONSTEXPR_CHECK(equal(
                    maximum(list(int{3}, 2ll, long{0}, short{1})),
                    int{3}
                ));
                BOOST_HANA_CONSTEXPR_CHECK(equal(
                    maximum(list(int{3}, 0ll, long{2}, short{1})),
                    int{3}
                ));
                BOOST_HANA_CONSTEXPR_CHECK(equal(
                    maximum(list(int{3}, 0ll, long{1}, short{2})),
                    int{3}
                ));
                BOOST_HANA_CONSTEXPR_CHECK(equal(
                    maximum(list(int{0}, 2ll, long{3}, short{1})),
                    long{3}
                ));
                BOOST_HANA_CONSTEXPR_CHECK(equal(
                    maximum(list(int{0}, 3ll, long{1}, short{2})),
                    3ll
                ));


                BOOST_HANA_CONSTEXPR_CHECK(equal(
                    maximum(list(int{0}, 1ll, long_<2>)),
                    long{2}
                ));
                BOOST_HANA_CONSTEXPR_CHECK(equal(
                    maximum(list(int{0}, long_<1>, 2ll)),
                    2ll
                ));
            }

            //////////////////////////////////////////////////////////////////
            // minimum (without a custom predicate)
            //////////////////////////////////////////////////////////////////
            {
                BOOST_HANA_CONSTANT_CHECK(equal(
                    minimum(list(ord<6>{})),
                    ord<6>{}
                ));

                BOOST_HANA_CONSTANT_CHECK(equal(
                    minimum(list(ord<6>{}, ord<7>{})),
                    ord<6>{}
                ));
                BOOST_HANA_CONSTANT_CHECK(equal(
                    minimum(list(ord<7>{}, ord<6>{})),
                    ord<6>{}
                ));

                BOOST_HANA_CONSTANT_CHECK(equal(
                    minimum(list(ord<6>{}, ord<7>{}, ord<8>{})),
                    ord<6>{}
                ));
                BOOST_HANA_CONSTANT_CHECK(equal(
                    minimum(list(ord<7>{}, ord<6>{}, ord<8>{})),
                    ord<6>{}
                ));
                BOOST_HANA_CONSTANT_CHECK(equal(
                    minimum(list(ord<7>{}, ord<8>{}, ord<6>{})),
                    ord<6>{}
                ));
                BOOST_HANA_CONSTANT_CHECK(equal(
                    minimum(list(ord<8>{}, ord<7>{}, ord<6>{})),
                    ord<6>{}
                ));
                BOOST_HANA_CONSTANT_CHECK(equal(
                    minimum(list(ord<8>{}, ord<6>{}, ord<7>{})),
                    ord<6>{}
                ));

                BOOST_HANA_CONSTANT_CHECK(equal(
                    minimum(list(ord<6>{}, ord<7>{}, ord<8>{}, ord<9>{})),
                    ord<6>{}
                ));
                BOOST_HANA_CONSTANT_CHECK(equal(
                    minimum(list(ord<7>{}, ord<6>{}, ord<8>{}, ord<9>{})),
                    ord<6>{}
                ));
                BOOST_HANA_CONSTANT_CHECK(equal(
                    minimum(list(ord<7>{}, ord<8>{}, ord<6>{}, ord<9>{})),
                    ord<6>{}
                ));
                BOOST_HANA_CONSTANT_CHECK(equal(
                    minimum(list(ord<7>{}, ord<8>{}, ord<9>{}, ord<6>{})),
                    ord<6>{}
                ));
                BOOST_HANA_CONSTANT_CHECK(equal(
                    minimum(list(ord<8>{}, ord<7>{}, ord<9>{}, ord<6>{})),
                    ord<6>{}
                ));
                BOOST_HANA_CONSTANT_CHECK(equal(
                    minimum(list(ord<8>{}, ord<9>{}, ord<7>{}, ord<6>{})),
                    ord<6>{}
                ));
                BOOST_HANA_CONSTANT_CHECK(equal(
                    minimum(list(ord<8>{}, ord<9>{}, ord<6>{}, ord<7>{})),
                    ord<6>{}
                ));
                BOOST_HANA_CONSTANT_CHECK(equal(
                    minimum(list(ord<9>{}, ord<8>{}, ord<6>{}, ord<7>{})),
                    ord<6>{}
                ));
                BOOST_HANA_CONSTANT_CHECK(equal(
                    minimum(list(ord<9>{}, ord<6>{}, ord<8>{}, ord<7>{})),
                    ord<6>{}
                ));
                BOOST_HANA_CONSTANT_CHECK(equal(
                    minimum(list(ord<9>{}, ord<6>{}, ord<7>{}, ord<8>{})),
                    ord<6>{}
                ));
                BOOST_HANA_CONSTANT_CHECK(equal(
                    minimum(list(ord<6>{}, ord<8>{}, ord<9>{}, ord<7>{})),
                    ord<6>{}
                ));
                BOOST_HANA_CONSTANT_CHECK(equal(
                    minimum(list(ord<6>{}, ord<9>{}, ord<7>{}, ord<8>{})),
                    ord<6>{}
                ));


                BOOST_HANA_CONSTEXPR_CHECK(equal(
                    minimum(list(int{4})),
                    int{4}
                ));

                BOOST_HANA_CONSTEXPR_CHECK(equal(
                    minimum(list(int{4}, short{5})),
                    int{4}
                ));
                BOOST_HANA_CONSTEXPR_CHECK(equal(
                    minimum(list(int{5}, short{4})),
                    short{4}
                ));

                BOOST_HANA_CONSTEXPR_CHECK(equal(
                    minimum(list(int{4}, short{5}, long{6})),
                    int{4}
                ));
                BOOST_HANA_CONSTEXPR_CHECK(equal(
                    minimum(list(int{7}, short{6}, long{8})),
                    short{6}
                ));
                BOOST_HANA_CONSTEXPR_CHECK(equal(
                    minimum(list(int{7}, short{8}, long{6})),
                    long{6}
                ));
                BOOST_HANA_CONSTEXPR_CHECK(equal(
                    minimum(list(int{8}, short{7}, long{6})),
                    long{6}
                ));
                BOOST_HANA_CONSTEXPR_CHECK(equal(
                    minimum(list(int{8}, short{6}, long{7})),
                    short{6}
                ));

                BOOST_HANA_CONSTEXPR_CHECK(equal(
                    minimum(list(int{6}, short{7}, long{8}, 9ll)),
                    int{6}
                ));
                BOOST_HANA_CONSTEXPR_CHECK(equal(
                    minimum(list(int{7}, short{6}, long{8}, 9ll)),
                    short{6}
                ));
                BOOST_HANA_CONSTEXPR_CHECK(equal(
                    minimum(list(int{7}, short{8}, long{6}, 9ll)),
                    long{6}
                ));
                BOOST_HANA_CONSTEXPR_CHECK(equal(
                    minimum(list(int{7}, short{8}, long{9}, 6ll)),
                    6ll
                ));
                BOOST_HANA_CONSTEXPR_CHECK(equal(
                    minimum(list(int{8}, short{7}, long{9}, 6ll)),
                    6ll
                ));
                BOOST_HANA_CONSTEXPR_CHECK(equal(
                    minimum(list(int{8}, short{9}, long{7}, 6ll)),
                    6ll
                ));
                BOOST_HANA_CONSTEXPR_CHECK(equal(
                    minimum(list(int{8}, short{9}, long{6}, 7ll)),
                    long{6}
                ));
                BOOST_HANA_CONSTEXPR_CHECK(equal(
                    minimum(list(int{9}, short{8}, long{6}, 7ll)),
                    long{6}
                ));
                BOOST_HANA_CONSTEXPR_CHECK(equal(
                    minimum(list(int{9}, short{6}, long{8}, 7ll)),
                    short{6}
                ));
                BOOST_HANA_CONSTEXPR_CHECK(equal(
                    minimum(list(int{9}, short{6}, long{7}, 8ll)),
                    short{6}
                ));
                BOOST_HANA_CONSTEXPR_CHECK(equal(
                    minimum(list(int{6}, short{8}, long{9}, 7ll)),
                    int{6}
                ));
                BOOST_HANA_CONSTEXPR_CHECK(equal(
                    minimum(list(int{6}, short{9}, long{7}, 8ll)),
                    int{6}
                ));


                BOOST_HANA_CONSTEXPR_CHECK(equal(
                    minimum(list(int{3}, short{4}, long_<5>)),
                    int{3}
                ));
                BOOST_HANA_CONSTEXPR_CHECK(equal(
                    minimum(list(ord<33>{}, short{45}, long{46})),
                    ord<33>{}
                ));
            }


            //////////////////////////////////////////////////////////////////
            // count_if
            //////////////////////////////////////////////////////////////////
            BOOST_HANA_CONSTANT_CHECK(equal(
                count_if(list(), id), size_t<0>
            ));
            BOOST_HANA_CONSTANT_CHECK(equal(
                count_if(list(int_<1>), id), size_t<1>
            ));
            BOOST_HANA_CONSTANT_CHECK(equal(
                count_if(list(int_<0>), id), size_t<0>
            ));

            BOOST_HANA_CONSTANT_CHECK(equal(
                count_if(list(int_<1>, char_<1>), id), size_t<2>
            ));
            BOOST_HANA_CONSTANT_CHECK(equal(
                count_if(list(int_<1>, char_<0>), id), size_t<1>
            ));
            BOOST_HANA_CONSTANT_CHECK(equal(
                count_if(list(int_<0>, char_<1>), id), size_t<1>
            ));
            BOOST_HANA_CONSTANT_CHECK(equal(
                count_if(list(int_<0>, char_<0>), id), size_t<0>
            ));

            BOOST_HANA_CONSTANT_CHECK(equal(
                count_if(list(int_<1>, char_<1>, long_<1>), id), size_t<3>
            ));
            BOOST_HANA_CONSTANT_CHECK(equal(
                count_if(list(int_<1>, char_<1>, long_<0>), id), size_t<2>
            ));
            BOOST_HANA_CONSTANT_CHECK(equal(
                count_if(list(int_<1>, char_<0>, long_<1>), id), size_t<2>
            ));
            BOOST_HANA_CONSTANT_CHECK(equal(
                count_if(list(int_<1>, char_<0>, long_<0>), id), size_t<1>
            ));
            BOOST_HANA_CONSTANT_CHECK(equal(
                count_if(list(int_<0>, char_<1>, long_<1>), id), size_t<2>
            ));
            BOOST_HANA_CONSTANT_CHECK(equal(
                count_if(list(int_<0>, char_<1>, long_<0>), id), size_t<1>
            ));
            BOOST_HANA_CONSTANT_CHECK(equal(
                count_if(list(int_<0>, char_<0>, long_<1>), id), size_t<1>
            ));
            BOOST_HANA_CONSTANT_CHECK(equal(
                count_if(list(int_<0>, char_<0>, long_<0>), id), size_t<0>
            ));


            BOOST_HANA_CONSTEXPR_CHECK(equal(
                count_if(list(int{1}), id), 1u
            ));
            BOOST_HANA_CONSTEXPR_CHECK(equal(
                count_if(list(int{0}), id), 0u
            ));

            BOOST_HANA_CONSTEXPR_CHECK(equal(
                count_if(list(int{1}, char{1}), id), 2u
            ));
            BOOST_HANA_CONSTEXPR_CHECK(equal(
                count_if(list(int{1}, char{0}), id), 1u
            ));
            BOOST_HANA_CONSTEXPR_CHECK(equal(
                count_if(list(int{0}, char{1}), id), 1u
            ));
            BOOST_HANA_CONSTEXPR_CHECK(equal(
                count_if(list(int{0}, char{0}), id), 0u
            ));

            BOOST_HANA_CONSTEXPR_CHECK(equal(
                count_if(list(int{1}, char{1}, double{1}), id), 3u
            ));
            BOOST_HANA_CONSTEXPR_CHECK(equal(
                count_if(list(int{1}, char{1}, double{0}), id), 2u
            ));
            BOOST_HANA_CONSTEXPR_CHECK(equal(
                count_if(list(int{1}, char{0}, double{1}), id), 2u
            ));
            BOOST_HANA_CONSTEXPR_CHECK(equal(
                count_if(list(int{1}, char{0}, double{0}), id), 1u
            ));
            BOOST_HANA_CONSTEXPR_CHECK(equal(
                count_if(list(int{0}, char{1}, double{1}), id), 2u
            ));
            BOOST_HANA_CONSTEXPR_CHECK(equal(
                count_if(list(int{0}, char{1}, double{0}), id), 1u
            ));
            BOOST_HANA_CONSTEXPR_CHECK(equal(
                count_if(list(int{0}, char{0}, double{1}), id), 1u
            ));
            BOOST_HANA_CONSTEXPR_CHECK(equal(
                count_if(list(int{0}, char{0}, double{0}), id), 0u
            ));


            BOOST_HANA_CONSTEXPR_LAMBDA auto is_even = [](auto x) {
                return x % 2 == 0;
            };
            BOOST_HANA_CONSTEXPR_CHECK(equal(
                count_if(list(), is_even), 0u
            ));
            BOOST_HANA_CONSTEXPR_CHECK(equal(
                count_if(list(1), is_even), 0u
            ));
            BOOST_HANA_CONSTEXPR_CHECK(equal(
                count_if(list(2), is_even), 1u
            ));
            BOOST_HANA_CONSTEXPR_CHECK(equal(
                count_if(list(1, 2), is_even), 1u
            ));
            BOOST_HANA_CONSTEXPR_CHECK(equal(
                count_if(list(1, 2, 3), is_even), 1u
            ));
            BOOST_HANA_CONSTEXPR_CHECK(equal(
                count_if(list(1, 2, 3, 4), is_even), 2u
            ));

            //////////////////////////////////////////////////////////////////
            // count
            //////////////////////////////////////////////////////////////////
            BOOST_HANA_CONSTANT_CHECK(hana::equal(
                hana::count(list(), undefined{}),
                size_t<0>
            ));

            BOOST_HANA_CONSTANT_CHECK(hana::equal(
                hana::count(list(ct_eq<0>{}), undefined{}),
                size_t<0>
            ));
            BOOST_HANA_CONSTANT_CHECK(hana::equal(
                hana::count(list(ct_eq<0>{}), ct_eq<0>{}),
                size_t<1>
            ));

            BOOST_HANA_CONSTANT_CHECK(hana::equal(
                hana::count(list(ct_eq<0>{}, ct_eq<1>{}), undefined{}),
                size_t<0>
            ));
            BOOST_HANA_CONSTANT_CHECK(hana::equal(
                hana::count(list(ct_eq<0>{}, ct_eq<1>{}), ct_eq<0>{}),
                size_t<1>
            ));
            BOOST_HANA_CONSTANT_CHECK(hana::equal(
                hana::count(list(ct_eq<0>{}, ct_eq<1>{}), ct_eq<1>{}),
                size_t<1>
            ));
            BOOST_HANA_CONSTANT_CHECK(hana::equal(
                hana::count(list(ct_eq<0>{}, ct_eq<0>{}), ct_eq<0>{}),
                size_t<2>
            ));

            //////////////////////////////////////////////////////////////////
            // product
            //////////////////////////////////////////////////////////////////
            BOOST_HANA_CONSTANT_CHECK(equal(
                product<IntegralConstant<int>>(list()),
                int_<1>
            ));
            BOOST_HANA_CONSTANT_CHECK(equal(
                product<IntegralConstant<int>>(list(int_<2>)),
                int_<2>
            ));
            BOOST_HANA_CONSTANT_CHECK(equal(
                product<IntegralConstant<int>>(list(int_<2>, int_<3>)),
                int_<2 * 3>
            ));
            BOOST_HANA_CONSTANT_CHECK(equal(
                product<IntegralConstant<int>>(list(int_<2>, int_<3>, int_<4>)),
                int_<2 * 3 * 4>
            ));
            BOOST_HANA_CONSTANT_CHECK(equal(
                product<IntegralConstant<int>>(list(int_<2>, int_<3>, int_<4>, int_<5>)),
                int_<2 * 3 * 4 * 5>
            ));

            BOOST_HANA_CONSTANT_CHECK(equal(
                product<IntegralConstant<unsigned long>>(list()),
                ulong<1>
            ));
            BOOST_HANA_CONSTANT_CHECK(equal(
                product<IntegralConstant<unsigned long>>(list(ulong<2>, ulong<3>, ulong<4>)),
                ulong<2 * 3 * 4>
            ));

            BOOST_HANA_CONSTEXPR_CHECK(equal(
                product<int>(list(2)),
                2
            ));
            BOOST_HANA_CONSTEXPR_CHECK(equal(
                product<int>(list(2, 3)),
                2 * 3
            ));
            BOOST_HANA_CONSTEXPR_CHECK(equal(
                product<int>(list(2, 3, 4)),
                2 * 3 * 4
            ));
            BOOST_HANA_CONSTEXPR_CHECK(equal(
                product<int>(list(2, 3, 4, 5)),
                2 * 3 * 4 * 5
            ));


            //////////////////////////////////////////////////////////////////
            // sum
            //////////////////////////////////////////////////////////////////
            BOOST_HANA_CONSTANT_CHECK(equal(
                sum<IntegralConstant<int>>(list()),
                int_<0>
            ));
            BOOST_HANA_CONSTANT_CHECK(equal(
                sum<IntegralConstant<int>>(list(int_<1>)),
                int_<1>
            ));
            BOOST_HANA_CONSTANT_CHECK(equal(
                sum<IntegralConstant<int>>(list(int_<1>, int_<2>)),
                int_<1 + 2>
            ));
            BOOST_HANA_CONSTANT_CHECK(equal(
                sum<IntegralConstant<int>>(list(int_<1>, int_<2>, int_<3>)),
                int_<1 + 2 + 3>
            ));
            BOOST_HANA_CONSTANT_CHECK(equal(
                sum<IntegralConstant<int>>(list(int_<1>, int_<2>, int_<3>, int_<4>)),
                int_<1 + 2 + 3 + 4>
            ));


            BOOST_HANA_CONSTANT_CHECK(equal(
                sum<IntegralConstant<unsigned long>>(list()),
                ulong<0>
            ));
            BOOST_HANA_CONSTANT_CHECK(equal(
                sum<IntegralConstant<unsigned long>>(list(ulong<1>, ulong<2>, ulong<3>)),
                ulong<1 + 2 + 3>
            ));


            BOOST_HANA_CONSTEXPR_CHECK(equal(
                sum<int>(list(1)), 1
            ));
            BOOST_HANA_CONSTEXPR_CHECK(equal(
                sum<int>(list(1, 2)), 1 + 2
            ));
            BOOST_HANA_CONSTEXPR_CHECK(equal(
                sum<int>(list(1, 2, 3)), 1 + 2 + 3
            ));
            BOOST_HANA_CONSTEXPR_CHECK(equal(
                sum<int>(list(1, 2, 3, 4)), 1 + 2 + 3 + 4
            ));


            //////////////////////////////////////////////////////////////////
            // unpack
            //////////////////////////////////////////////////////////////////
            BOOST_HANA_CONSTANT_CHECK(equal(
                unpack(list(), f),
                f()
            ));
            BOOST_HANA_CONSTANT_CHECK(equal(
                unpack(list(x<0>{}), f),
                f(x<0>{})
            ));
            BOOST_HANA_CONSTANT_CHECK(equal(
                unpack(list(x<0>{}, x<1>{}), f),
                f(x<0>{}, x<1>{})
            ));
            BOOST_HANA_CONSTANT_CHECK(equal(
                unpack(list(x<0>{}, x<1>{}, x<2>{}), f),
                f(x<0>{}, x<1>{}, x<2>{})
            ));
            BOOST_HANA_CONSTANT_CHECK(equal(
                unpack(list(x<0>{}, x<1>{}, x<2>{}, x<3>{}), f),
                f(x<0>{}, x<1>{}, x<2>{}, x<3>{})
            ));
        }
    };
}}} // end namespace boost::hana::test

#endif // !BOOST_HANA_TEST_LAWS_FOLDABLE_HPP
