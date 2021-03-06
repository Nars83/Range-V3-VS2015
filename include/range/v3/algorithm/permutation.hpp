/// \file
// Range v3 library
//
//  Copyright Eric Niebler 2014
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/ericniebler/range-v3
//
//===-------------------------- algorithm ---------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef RANGES_V3_ALGORITHM_PERMUTATION_HPP
#define RANGES_V3_ALGORITHM_PERMUTATION_HPP

#include <meta/meta.hpp>
#include <range/v3/range_fwd.hpp>
#include <range/v3/begin_end.hpp>
#include <range/v3/range_concepts.hpp>
#include <range/v3/range_traits.hpp>
#include <range/v3/distance.hpp>
#include <range/v3/utility/iterator.hpp>
#include <range/v3/utility/iterator_concepts.hpp>
#include <range/v3/utility/iterator_traits.hpp>
#include <range/v3/utility/swap.hpp>
#include <range/v3/utility/functional.hpp>
#include <range/v3/algorithm/reverse.hpp>
#include <range/v3/utility/static_const.hpp>

namespace ranges
{
    inline namespace v3
    {
        /// \ingroup group-concepts
        template<typename I1, typename I2, typename C = equal_to, typename P1 = ident,
            typename P2 = ident>
        using IsPermutationable = meta::fast_and<
            ForwardIterator<I1>,
            ForwardIterator<I2>,
            Comparable<I1, I2, C, P1, P2>>;

        /// \addtogroup group-algorithms
        /// @{
        struct is_permutation_fn
        {
        private:
            template<typename I1, typename S1, typename I2, typename S2, typename C, typename P1,
                typename P2>
            static bool four_iter_impl(I1 begin1, S1 end1, I2 begin2, S2 end2, C pred_, P1 proj1_,
                P2 proj2_)
            {
                auto &&pred = as_function(pred_);
                auto &&proj1 = as_function(proj1_);
                auto &&proj2 = as_function(proj2_);
                // shorten sequences as much as possible by lopping of any equal parts
                for(; begin1 != end1 && begin2 != end2; ++begin1, ++begin2)
                    if(!pred(proj1(*begin1), proj2(*begin2)))
                        goto not_done;
                return begin1 == end1 && begin2 == end2;
            not_done:
                // begin1 != end1 && begin2 != end2 && *begin1 != *begin2
                auto l1 = distance(begin1, end1);
                auto l2 = distance(begin2, end2);
                if(l1 != l2)
                    return false;

                // For each element in [f1, l1) see if there are the same number of
                //    equal elements in [f2, l2)
                for(I1 i = begin1; i != end1; ++i)
                {
                    // Have we already counted the number of *i in [f1, l1)?
                    for(I1 j = begin1; j != i; ++j)
                        if(pred(proj1(*j), proj1(*i)))
                            goto next_iter;
                    {
                        // Count number of *i in [f2, l2)
                        iterator_difference_t<I1> c2 = 0;
                        for(I2 j = begin2; j != end2; ++j)
                            if(pred(proj1(*i), proj2(*j)))
                                ++c2;
                        if(c2 == 0)
                            return false;
                        // Count number of *i in [i, l1) (we can start with 1)
                        iterator_difference_t<I1> c1 = 1;
                        for(I1 j = next(i); j != end1; ++j)
                            if(pred(proj1(*i), proj1(*j)))
                                ++c1;
                        if(c1 != c2)
                            return false;
                    }
            next_iter:;
                }
                return true;
            }

        public:
            template<typename I1, typename S1, typename I2, typename C = equal_to,
                typename P1 = ident, typename P2 = ident,
#ifdef RANGES_WORKAROUND_MSVC_SFINAE_CONSTEXPR
                CONCEPT_REQUIRES_(IteratorRange<I1, S1>::value && IsPermutationable<I1, I2, C, P1, P2>::value)>
#else
                CONCEPT_REQUIRES_(IteratorRange<I1, S1>() && IsPermutationable<I1, I2, C, P1, P2>())>
#endif
            bool operator()(I1 begin1, S1 end1, I2 begin2, C pred_ = C{}, P1 proj1_ = P1{},
                P2 proj2_ = P2{}) const
            {
                auto &&pred = as_function(pred_);
                auto &&proj1 = as_function(proj1_);
                auto &&proj2 = as_function(proj2_);
                // shorten sequences as much as possible by lopping of any equal parts
                for(; begin1 != end1; ++begin1, ++begin2)
                    if(!pred(proj1(*begin1), proj2(*begin2)))
                        goto not_done;
                return true;
            not_done:
                // begin1 != end1 && *begin1 != *begin2
                auto l1 = distance(begin1, end1);
                if(l1 == 1)
                    return false;
                I2 end2 = next(begin2, l1);
                // For each element in [f1, l1) see if there are the same number of
                //    equal elements in [f2, l2)
                for(I1 i = begin1; i != end1; ++i)
                {
                    // Have we already counted the number of *i in [f1, l1)?
                    for(I1 j = begin1; j != i; ++j)
                        if(pred(proj1(*j), proj1(*i)))
                            goto next_iter;
                    {
                        // Count number of *i in [f2, l2)
                        iterator_difference_t<I1> c2 = 0;
                        for(I2 j = begin2; j != end2; ++j)
                            if(pred(proj1(*i), proj2(*j)))
                                ++c2;
                        if(c2 == 0)
                            return false;
                        // Count number of *i in [i, l1) (we can start with 1)
                        iterator_difference_t<I1> c1 = 1;
                        for(I1 j = next(i); j != end1; ++j)
                            if(pred(proj1(*i), proj1(*j)))
                                ++c1;
                        if(c1 != c2)
                            return false;
                    }
            next_iter:;
                }
                return true;
            }

            template<typename I1, typename S1, typename I2, typename S2,
                typename C = equal_to, typename P1 = ident, typename P2 = ident,
#ifdef RANGES_WORKAROUND_MSVC_SFINAE_CONSTEXPR
                CONCEPT_REQUIRES_(IteratorRange<I1, S1>::value && IteratorRange<I2, S2>::value &&
                    IsPermutationable<I1, I2, C, P1, P2>::value)>
#else
                CONCEPT_REQUIRES_(IteratorRange<I1, S1>() && IteratorRange<I2, S2>() &&
                    IsPermutationable<I1, I2, C, P1, P2>())>
#endif
            bool operator()(I1 begin1, S1 end1, I2 begin2, S2 end2, C pred = C{},
                P1 proj1 = P1{}, P2 proj2 = P2{}) const
            {
                if(SizedIteratorRange<I1, S1>() && SizedIteratorRange<I2, S2>())
                    return distance(begin1, end1) == distance(begin2, end2) &&
                        (*this)(std::move(begin1), std::move(end1), std::move(begin2),
                            std::move(pred), std::move(proj1), std::move(proj2));
                return is_permutation_fn::four_iter_impl(std::move(begin1), std::move(end1),
                    std::move(begin2), std::move(end2), std::move(pred), std::move(proj1),
                    std::move(proj2));
            }

            template<typename Rng1, typename I2Ref, typename C = equal_to, typename P1 = ident,
                typename P2 = ident, typename I1 = range_iterator_t<Rng1>,
                typename I2 = uncvref_t<I2Ref>,
#ifdef RANGES_WORKAROUND_MSVC_SFINAE_CONSTEXPR
                CONCEPT_REQUIRES_(ForwardRange<Rng1>::value && Iterator<I2>::value &&
                    IsPermutationable<I1, I2, C, P1, P2>::value)>
#else
                CONCEPT_REQUIRES_(ForwardRange<Rng1>() && Iterator<I2>() &&
                    IsPermutationable<I1, I2, C, P1, P2>())>
#endif
            bool operator()(Rng1 &&rng1, I2Ref &&begin2,
                C pred = C{}, P1 proj1 = P1{}, P2 proj2 = P2{}) const
            {
                return (*this)(begin(rng1), end(rng1), (I2Ref &&) begin2, std::move(pred),
                    std::move(proj1), std::move(proj2));
            }

            template<typename Rng1, typename Rng2, typename C = equal_to, typename P1 = ident,
                typename P2 = ident, typename I1 = range_iterator_t<Rng1>,
                typename I2 = range_iterator_t<Rng2>,
#ifdef RANGES_WORKAROUND_MSVC_SFINAE_CONSTEXPR
                CONCEPT_REQUIRES_(ForwardRange<Rng1>::value && ForwardRange<Rng2>::value &&
                    IsPermutationable<I1, I2, C, P1, P2>::value)>
#else
                CONCEPT_REQUIRES_(ForwardRange<Rng1>() && ForwardRange<Rng2>() &&
                    IsPermutationable<I1, I2, C, P1, P2>())>
#endif
            bool operator()(Rng1 &&rng1, Rng2 &&rng2,
                C pred = C{}, P1 proj1 = P1{}, P2 proj2 = P2{}) const
            {
                if(SizedRange<Rng1>() && SizedRange<Rng2>())
                    return distance(rng1) == distance(rng2) &&
                        (*this)(begin(rng1), end(rng1), begin(rng2), std::move(pred),
                            std::move(proj1), std::move(proj2));
                return is_permutation_fn::four_iter_impl(begin(rng1), end(rng1), begin(rng2),
                    end(rng2), std::move(pred), std::move(proj1), std::move(proj2));
            }
        };

        /// \sa `is_permutation_fn`
        /// \ingroup group-algorithms
        namespace
        {
            constexpr auto&& is_permutation = static_const<with_braced_init_args<is_permutation_fn>>::value;
        }

        struct next_permutation_fn
        {
            template<typename I, typename S, typename C = ordered_less, typename P = ident,
#ifdef RANGES_WORKAROUND_MSVC_SFINAE_CONSTEXPR
                CONCEPT_REQUIRES_(BidirectionalIterator<I>::value && IteratorRange<I, S>::value && Sortable<I, C, P>::value)>
#else
                CONCEPT_REQUIRES_(BidirectionalIterator<I>() && IteratorRange<I, S>() && Sortable<I, C, P>())>
#endif
            bool operator()(I begin, S end_, C pred_ = C{}, P proj_ = P{}) const
            {
                auto &&pred = as_function(pred_);
                auto &&proj = as_function(proj_);
                if(begin == end_)
                    return false;
                I end = ranges::next(begin, end_), i = end;
                if(begin == --i)
                    return false;
                while(true)
                {
                    I ip1 = i;
                    if(pred(proj(*--i), proj(*ip1)))
                    {
                        I j = end;
                        while(!pred(proj(*i), proj(*--j)))
                            ;
                        ranges::iter_swap(i, j);
                        ranges::reverse(ip1, end);
                        return true;
                    }
                    if(i == begin)
                    {
                        ranges::reverse(begin, end);
                        return false;
                    }
                }
            }

            template<typename Rng, typename C = ordered_less, typename P = ident,
                typename I = range_iterator_t<Rng>,
#ifdef RANGES_WORKAROUND_MSVC_SFINAE_CONSTEXPR
                CONCEPT_REQUIRES_(BidirectionalRange<Rng>::value && Sortable<I, C, P>::value)>
#else
                CONCEPT_REQUIRES_(BidirectionalRange<Rng>() && Sortable<I, C, P>())>
#endif
            bool operator()(Rng &&rng, C pred = C{}, P proj = P{}) const
            {
                return (*this)(begin(rng), end(rng), std::move(pred), std::move(proj));
            }
        };

        /// \sa `next_permutation_fn`
        /// \ingroup group-algorithms
        namespace
        {
            constexpr auto&& next_permutation = static_const<with_braced_init_args<next_permutation_fn>>::value;
        }

        struct prev_permutation_fn
        {
            template<typename I, typename S, typename C = ordered_less, typename P = ident,
#ifdef RANGES_WORKAROUND_MSVC_SFINAE_CONSTEXPR
                CONCEPT_REQUIRES_(BidirectionalIterator<I>::value && IteratorRange<I, S>::value && Sortable<I, C, P>::value)>
#else
                CONCEPT_REQUIRES_(BidirectionalIterator<I>() && IteratorRange<I, S>() && Sortable<I, C, P>())>
#endif
            bool operator()(I begin, S end_, C pred_ = C{}, P proj_ = P{}) const
            {
                auto &&pred = as_function(pred_);
                auto &&proj = as_function(proj_);
                if(begin == end_)
                    return false;
                I end = ranges::next(begin, end_), i = end;
                if(begin == --i)
                    return false;
                while(true)
                {
                    I ip1 = i;
                    if(pred(proj(*ip1), proj(*--i)))
                    {
                        I j = end;
                        while(!pred(proj(*--j), proj(*i)))
                            ;
                        ranges::iter_swap(i, j);
                        ranges::reverse(ip1, end);
                        return true;
                    }
                    if(i == begin)
                    {
                        ranges::reverse(begin, end);
                        return false;
                    }
                }
            }

            template<typename Rng, typename C = ordered_less, typename P = ident,
                typename I = range_iterator_t<Rng>,
#ifdef RANGES_WORKAROUND_MSVC_SFINAE_CONSTEXPR
                CONCEPT_REQUIRES_(BidirectionalRange<Rng>::value && Sortable<I, C, P>::value)>
#else
                CONCEPT_REQUIRES_(BidirectionalRange<Rng>() && Sortable<I, C, P>())>
#endif
            bool operator()(Rng &&rng, C pred = C{}, P proj = P{}) const
            {
                return (*this)(begin(rng), end(rng), std::move(pred), std::move(proj));
            }
        };

        /// \sa `prev_permutation_fn`
        /// \ingroup group-algorithms
        namespace
        {
            constexpr auto&& prev_permutation = static_const<with_braced_init_args<prev_permutation_fn>>::value;
        }

        /// @}
    } // namespace v3
} // namespace ranges

#endif // include guard
