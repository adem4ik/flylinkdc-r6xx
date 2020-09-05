/*

Copyright (c) 2014-2015, 2017-2020, Arvid Norberg
Copyright (c) 2019, Steven Siloti
Copyright (c) 2019, Alden Torres
All rights reserved.

You may use, distribute and modify this code under the terms of the BSD license,
see LICENSE file.
*/

#ifndef TORRENT_EXPORT_HPP_INCLUDED
#define TORRENT_EXPORT_HPP_INCLUDED

#include <boost/config.hpp>
#include "libtorrent/config.hpp"

#include "libtorrent/aux_/deprecated.hpp"

#if !defined TORRENT_ABI_VERSION
# ifdef TORRENT_NO_DEPRECATE
#  define TORRENT_ABI_VERSION 4
# else
#  define TORRENT_ABI_VERSION 1
# endif
#endif

#if TORRENT_ABI_VERSION >= 3
# define TORRENT_VERSION_NAMESPACE_3 inline namespace v2 {
# define TORRENT_VERSION_NAMESPACE_3_END  }
#else
# define TORRENT_VERSION_NAMESPACE_3
# define TORRENT_VERSION_NAMESPACE_3_END
#endif

#if TORRENT_ABI_VERSION >= 2
# define TORRENT_VERSION_NAMESPACE_2 inline namespace v1_2 {
# define TORRENT_VERSION_NAMESPACE_2_END  }
#else
# define TORRENT_VERSION_NAMESPACE_2
# define TORRENT_VERSION_NAMESPACE_2_END
#endif

#ifdef TORRENT_USE_LIBGCRYPT
# define TORRENT_CRYPTO_NAMESPACE inline namespace gcry {
# define TORRENT_CRYPTO_NAMESPACE_END }
#elif TORRENT_USE_COMMONCRYPTO
# define TORRENT_CRYPTO_NAMESPACE inline namespace cc {
# define TORRENT_CRYPTO_NAMESPACE_END }
#elif TORRENT_USE_CRYPTOAPI
# define TORRENT_CRYPTO_NAMESPACE inline namespace capi {
# define TORRENT_CRYPTO_NAMESPACE_END }
#elif defined TORRENT_USE_WOLFSSL
# define TORRENT_CRYPTO_NAMESPACE inline namespace wcrypto {
# define TORRENT_CRYPTO_NAMESPACE_END }
#elif defined TORRENT_USE_LIBCRYPTO
# define TORRENT_CRYPTO_NAMESPACE inline namespace lcrypto {
# define TORRENT_CRYPTO_NAMESPACE_END }
#else
# define TORRENT_CRYPTO_NAMESPACE inline namespace builtin {
# define TORRENT_CRYPTO_NAMESPACE_END }
#endif

// backwards compatibility with older versions of boost
#if !defined BOOST_SYMBOL_EXPORT && !defined BOOST_SYMBOL_IMPORT
# if defined _MSC_VER || defined __MINGW32__
#  define BOOST_SYMBOL_EXPORT __declspec(dllexport)
#  define BOOST_SYMBOL_IMPORT __declspec(dllimport)
# elif __GNU__ >= 4
#  define BOOST_SYMBOL_EXPORT __attribute__((visibility("default")))
#  define BOOST_SYMBOL_IMPORT __attribute__((visibility("default")))
# else
#  define BOOST_SYMBOL_EXPORT
#  define BOOST_SYMBOL_IMPORT
# endif
#endif

#if !defined TORRENT_EXPORT_EXTRA \
  && ((defined __GNU__ && __GNU__ >= 4) || defined __clang__)
# define TORRENT_UNEXPORT __attribute__((visibility("hidden")))
#else
# define TORRENT_UNEXPORT
#endif

#if defined TORRENT_BUILDING_SHARED
# define TORRENT_EXPORT BOOST_SYMBOL_EXPORT
#elif defined TORRENT_LINKING_SHARED
# define TORRENT_EXPORT BOOST_SYMBOL_IMPORT
#endif

// when this is specified, export a bunch of extra
// symbols, mostly for the unit tests to reach
#if defined TORRENT_EXPORT_EXTRA
# if defined TORRENT_BUILDING_SHARED
#  define TORRENT_EXTRA_EXPORT BOOST_SYMBOL_EXPORT
# elif defined TORRENT_LINKING_SHARED
#  define TORRENT_EXTRA_EXPORT BOOST_SYMBOL_IMPORT
# endif
#endif

#ifndef TORRENT_EXPORT
# define TORRENT_EXPORT
#endif

#ifndef TORRENT_EXTRA_EXPORT
# define TORRENT_EXTRA_EXPORT
#endif

// only export this type if deprecated functions are enabled
// mingw doesn't like combining C++11 attributes with __attribute__ apparently
#if defined __MINGW64__ || defined __MINGW32__

# if TORRENT_ABI_VERSION >= 2
#  define TORRENT_DEPRECATED_EXPORT TORRENT_EXTRA_EXPORT
# else
#  define TORRENT_DEPRECATED_EXPORT TORRENT_EXPORT
# endif

#else

# if TORRENT_ABI_VERSION >= 2
#  define TORRENT_DEPRECATED_EXPORT TORRENT_DEPRECATED TORRENT_EXTRA_EXPORT
# else
#  define TORRENT_DEPRECATED_EXPORT TORRENT_DEPRECATED TORRENT_EXPORT
# endif

#endif

#endif

