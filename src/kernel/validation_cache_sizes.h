// Copyright (c) 2022 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_KERNEL_VALIDATION_CACHE_SIZES_H
#define BITCOIN_KERNEL_VALIDATION_CACHE_SIZES_H

#include <script/sigcache.h>

#include <cstdint>
#include <limits>

namespace kernel {
struct ValidationCacheSizes {
    int64_t signature_cache_bytes{DEFAULT_MAX_SIG_CACHE_BYTES / 2};
    int64_t script_execution_cache_bytes{DEFAULT_MAX_SIG_CACHE_BYTES / 2};
};
}

#endif // BITCOIN_KERNEL_VALIDATION_CACHE_SIZES_H
