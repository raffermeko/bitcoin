// Copyright (c) 2022 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef BITCOIN_KERNEL_MEMPOOL_LIMITS_H
#define BITCOIN_KERNEL_MEMPOOL_LIMITS_H

#include <policy/policy.h>

#include <cstdint>

namespace kernel {
/**
 * Options struct containing limit options for a CTxMemPool. Default constructor
 * populates the struct with sane default values which can be modified.
 *
 * Most of the time, this struct should be referenced as CTxMemPool::Limits.
 */
struct MemPoolLimits {
    int64_t ancestor_count{DEFAULT_ANCESTOR_LIMIT};
    int64_t ancestor_size_vbytes{DEFAULT_ANCESTOR_SIZE_LIMIT_KVB * 1'000};
    int64_t descendant_count{DEFAULT_DESCENDANT_LIMIT};
    int64_t descendant_size_vbytes{DEFAULT_DESCENDANT_SIZE_LIMIT_KVB * 1'000};
};
} // namespace kernel

#endif // BITCOIN_KERNEL_MEMPOOL_LIMITS_H
