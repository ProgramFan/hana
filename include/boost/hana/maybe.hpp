/*!
@file
Defines `boost::hana::Maybe`.

@copyright Louis Dionne 2014
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 */

#ifndef BOOST_HANA_MAYBE_HPP
#define BOOST_HANA_MAYBE_HPP

#include <boost/hana/fwd/maybe.hpp>

#include <boost/hana/applicative.hpp>
#include <boost/hana/bool.hpp>
#include <boost/hana/comparable.hpp>
#include <boost/hana/config.hpp>
#include <boost/hana/core/operators.hpp>
#include <boost/hana/detail/std/forward.hpp>
#include <boost/hana/detail/std/integral_constant.hpp>
#include <boost/hana/detail/std/move.hpp>
#include <boost/hana/detail/std/remove_reference.hpp>
#include <boost/hana/foldable.hpp>
#include <boost/hana/functional/always.hpp>
#include <boost/hana/functional/compose.hpp>
#include <boost/hana/functional/id.hpp>
#include <boost/hana/functor.hpp>
#include <boost/hana/logical.hpp>
#include <boost/hana/monad.hpp>
#include <boost/hana/monad_plus.hpp>
#include <boost/hana/orderable.hpp>
#include <boost/hana/searchable.hpp>
#include <boost/hana/traversable.hpp>


namespace boost { namespace hana {
    //////////////////////////////////////////////////////////////////////////
    // Operators
    //////////////////////////////////////////////////////////////////////////
    namespace operators {
        template <>
        struct of<Maybe>
            : operators::of<Comparable, Orderable, Monad>
        { };
    }

    //////////////////////////////////////////////////////////////////////////
    // is_just and is_nothing
    //////////////////////////////////////////////////////////////////////////
    // Remove warnings generated by poor confused Doxygen
    //! @cond

    template <typename M>
    constexpr auto _is_just::operator()(M const&) const
    { return _bool<M::is_just>{}; }

    template <typename M>
    constexpr auto _is_nothing::operator()(M const&) const
    { return _bool<!M::is_just>{}; }

    //////////////////////////////////////////////////////////////////////////
    // from_maybe and from_just
    //////////////////////////////////////////////////////////////////////////
    template <typename Default, typename M>
    constexpr decltype(auto) _from_maybe::operator()(Default&& default_, M&& m) const {
        return hana::maybe(detail::std::forward<Default>(default_), id,
                                            detail::std::forward<M>(m));
    }

    template <typename M>
    constexpr decltype(auto) _from_just::operator()(M&& m) const {
        static_assert(detail::std::remove_reference<M>::type::is_just,
        "trying to extract the value inside a boost::hana::nothing "
        "with boost::hana::from_just");
        return hana::id(detail::std::forward<M>(m).val);
    }

    //////////////////////////////////////////////////////////////////////////
    // only_when
    //////////////////////////////////////////////////////////////////////////
    namespace maybe_detail {
        template <typename F, typename X>
        struct just_f_x {
            F f; X x;
            template <typename Id>
            constexpr decltype(auto) operator()(Id _) && {
                return hana::just(_(detail::std::forward<F>(f))(
                    detail::std::forward<X>(x)
                ));
            }

#ifndef BOOST_HANA_CONFIG_CONSTEXPR_MEMBER_FUNCTION_IS_CONST
            template <typename Id>
            constexpr decltype(auto) operator()(Id _) &
            { return hana::just(_(f)(x)); }
#endif

            template <typename Id>
            constexpr decltype(auto) operator()(Id _) const&
            { return hana::just(_(f)(x)); }
        };
    }

    template <typename Pred, typename F, typename X>
    constexpr decltype(auto) _only_when::operator()(Pred&& pred, F&& f, X&& x) const {
        return hana::eval_if(detail::std::forward<Pred>(pred)(x),
            maybe_detail::just_f_x<F, X>{detail::std::forward<F>(f),
                                         detail::std::forward<X>(x)},
            hana::always(nothing)
        );
    }

    //! @endcond

    //////////////////////////////////////////////////////////////////////////
    // Comparable
    //////////////////////////////////////////////////////////////////////////
    template <>
    struct equal_impl<Maybe, Maybe> {
        template <typename T, typename U>
        static constexpr decltype(auto) apply(_just<T> const& t, _just<U> const& u)
        { return hana::equal(t.val, u.val); }

        static constexpr _true apply(_nothing const&, _nothing const&)
        { return {}; }

        template <typename T, typename U>
        static constexpr _false apply(T const&, U const&)
        { return {}; }
    };

    //////////////////////////////////////////////////////////////////////////
    // Orderable
    //////////////////////////////////////////////////////////////////////////
    template <>
    struct less_impl<Maybe, Maybe> {
        template <typename T>
        static constexpr _true apply(_nothing const&, _just<T> const&)
        { return {}; }

        static constexpr _false apply(_nothing const&, _nothing const&)
        { return {}; }

        template <typename T>
        static constexpr _false apply(_just<T> const&, _nothing const&)
        { return {}; }

        template <typename T, typename U>
        static constexpr auto apply(_just<T> const& x, _just<U> const& y)
        { return hana::less(x.val, y.val); }
    };

    //////////////////////////////////////////////////////////////////////////
    // Functor
    //////////////////////////////////////////////////////////////////////////
    template <>
    struct transform_impl<Maybe> {
        template <typename M, typename F>
        static constexpr decltype(auto) apply(M&& m, F&& f) {
            return hana::maybe(
                nothing,
                hana::compose(just, detail::std::forward<F>(f)),
                detail::std::forward<M>(m)
            );
        }
    };

    //////////////////////////////////////////////////////////////////////////
    // Applicative
    //////////////////////////////////////////////////////////////////////////
    template <>
    struct lift_impl<Maybe> {
        template <typename X>
        static constexpr decltype(auto) apply(X&& x)
        { return hana::just(detail::std::forward<X>(x)); }
    };

    template <>
    struct ap_impl<Maybe> {
        template <typename F, typename X>
        static constexpr decltype(auto) apply_impl(F&& f, X&& x, detail::std::true_type) {
            return hana::just(detail::std::forward<F>(f).val(detail::std::forward<X>(x).val));
        }

        template <typename F, typename X>
        static constexpr auto apply_impl(F&&, X&&, detail::std::false_type)
        { return nothing; }

        template <typename F, typename X>
        static constexpr auto apply(F&& f, X&& x) {
            auto f_is_just = hana::is_just(f);
            auto x_is_just = hana::is_just(x);
            return apply_impl(
                detail::std::forward<F>(f), detail::std::forward<X>(x),
                detail::std::integral_constant<bool,
                    hana::value2<decltype(f_is_just)>() && hana::value2<decltype(x_is_just)>()
                >{}
            );
        }
    };

    //////////////////////////////////////////////////////////////////////////
    // Monad
    //////////////////////////////////////////////////////////////////////////
    template <>
    struct flatten_impl<Maybe> {
        template <typename MMX>
        static constexpr decltype(auto) apply(MMX&& mmx) {
            return hana::maybe(nothing, id, detail::std::forward<MMX>(mmx));
        }
    };

    //////////////////////////////////////////////////////////////////////////
    // MonadPlus
    //////////////////////////////////////////////////////////////////////////
    template <>
    struct concat_impl<Maybe> {
        template <typename Y>
        static constexpr auto apply(_nothing, Y&& y)
        { return detail::std::forward<Y>(y); }

        template <typename X, typename Y>
        static constexpr auto apply(X&& x, Y const&)
        { return detail::std::forward<X>(x); }
    };

    template <>
    struct empty_impl<Maybe> {
        static constexpr auto apply()
        { return nothing; }
    };

    //////////////////////////////////////////////////////////////////////////
    // Traversable
    //////////////////////////////////////////////////////////////////////////
    template <>
    struct traverse_impl<Maybe> {
        template <typename A, typename F>
        static constexpr decltype(auto) apply(_nothing const&, F&& f)
        { return lift<A>(nothing); }

        template <typename A, typename T, typename F>
        static constexpr decltype(auto) apply(_just<T> const& x, F&& f)
        { return hana::transform(detail::std::forward<F>(f)(x.val), just); }

        template <typename A, typename T, typename F>
        static constexpr decltype(auto) apply(_just<T>& x, F&& f)
        { return hana::transform(detail::std::forward<F>(f)(x.val), just); }

        template <typename A, typename T, typename F>
        static constexpr decltype(auto) apply(_just<T>&& x, F&& f) {
            return hana::transform(detail::std::forward<F>(f)(
                                        detail::std::move(x.val)), just);
        }
    };

    //////////////////////////////////////////////////////////////////////////
    // Foldable
    //////////////////////////////////////////////////////////////////////////
    template <>
    struct unpack_impl<Maybe> {
        template <typename M, typename F>
        static constexpr decltype(auto) apply(M&& m, F&& f)
        { return detail::std::forward<F>(f)(detail::std::forward<M>(m).val); }

        template <typename F>
        static constexpr decltype(auto) apply(_nothing const&, F&& f)
        { return detail::std::forward<F>(f)(); }

        template <typename F>
        static constexpr decltype(auto) apply(_nothing&&, F&& f)
        { return detail::std::forward<F>(f)(); }

        template <typename F>
        static constexpr decltype(auto) apply(_nothing&, F&& f)
        { return detail::std::forward<F>(f)(); }
    };

    //////////////////////////////////////////////////////////////////////////
    // Searchable
    //////////////////////////////////////////////////////////////////////////
    template <>
    struct find_impl<Maybe> {
        template <typename M, typename Pred>
        static constexpr decltype(auto) apply(M&& m, Pred&& pred) {
            return hana::only_when(detail::std::forward<Pred>(pred), id,
                                        detail::std::forward<M>(m).val);
        }

        template <typename Pred>
        static constexpr auto apply(_nothing const&, Pred&&)
        { return nothing; }

        template <typename Pred>
        static constexpr auto apply(_nothing&&, Pred&&)
        { return nothing; }

        template <typename Pred>
        static constexpr auto apply(_nothing&, Pred&&)
        { return nothing; }
    };

    template <>
    struct any_impl<Maybe> {
        template <typename M, typename Pred>
        static constexpr decltype(auto) apply(M&& m, Pred&& p) {
            return hana::maybe(_false{},
                detail::std::forward<Pred>(p),
                detail::std::forward<M>(m)
            );
        }
    };
}} // end namespace boost::hana

#endif // !BOOST_HANA_MAYBE_HPP
