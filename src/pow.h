// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Copyright (c) 2020 The Gapcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_POW_H
#define BITCOIN_POW_H

#include <consensus/params.h>
#include <PoWCore/src/PoW.h>

#include <stdint.h>

class CBlockHeader;
class CBlockIndex;
class uint256;

bool TestNet();

uint64_t GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params&);

/** Check whether a block hash satisfies the proof-of-work requirement specified by nDifficulty */
bool CheckProofOfWork(const uint256 hash, const uint16_t nShift, const std::vector<uint8_t> *const nAdd, const uint64_t nDifficulty, const Consensus::Params&);

#endif // BITCOIN_POW_H
