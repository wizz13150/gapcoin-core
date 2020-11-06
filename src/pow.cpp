// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Copyright (c) 2020 The Gapcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <pow.h>

#include <arith_uint256.h>
#include <chain.h>
#include <chainparams.h>
#include <chainparamsbase.h>
#include <primitives/block.h>
#include <uint256.h>
#include <util.h>

static const int64_t nTargetTimespan = 150; // every block
static const int64_t nTargetSpacing = 150; // 2.5 minutes
static const int64_t nInterval = nTargetTimespan / nTargetSpacing;
static PoWUtils *powUtils = new PoWUtils();

bool TestNet()
{
    if (ChainNameFromCommandLine() == CBaseChainParams::TESTNET)
        return true;
    return false;
}

uint64_t GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params)
{
    // Genesis block
    if (pindexLast == NULL)
        return (TestNet() ? PoWUtils::min_test_difficulty : PoWUtils::min_difficulty);

    if (TestNet())
    {
        // Special difficulty rule for testnet:
        // If the new block's timestamp is more than 100 * 2:30 minutes
        // then allow mining of a min-difficulty block.
        if (pblock->nTime > pindexLast->nTime + nTargetSpacing*100)
            return PoWUtils::min_test_difficulty;
    }

    // don't use genesis block
    const CBlockIndex* pindexFirst = pindexLast;
    if (pindexFirst->pprev && pindexFirst->pprev->pprev)
        pindexFirst = pindexFirst->pprev;

    // Limit adjustment step
    int64_t nActualTimespan = pindexLast->GetBlockTime() - pindexFirst->GetBlockTime();

    // if prev block not avilable asume optimal timespan
    if (pindexFirst == pindexLast)
        nActualTimespan = nTargetSpacing;

    // do not divide by zero (or negative number)
    if (nActualTimespan < 1)
        nActualTimespan = 1;

    // Retarget
    uint64_t nextDifficulty = powUtils->next_difficulty(pindexLast->nDifficulty, nActualTimespan, TestNet());

    /// debug print
    // LogPrint(BCLog::DEVEL, "GetNextWorkRequired Retarget, nTargetTimespan = %li, nActualTimespan = %li.\n", nTargetTimespan, nActualTimespan);
    // LogPrint(BCLog::DEVEL, "Before: %016llx  %F.\n", pindexLast->nDifficulty, powUtils->get_readable_difficulty(pindexLast->nDifficulty));
    // LogPrint(BCLog::DEVEL, "After:  %016llx  %F.\n", nextDifficulty, powUtils->get_readable_difficulty(nextDifficulty));

    return nextDifficulty;
}

bool CheckProofOfWork(const uint256 hash, const uint16_t nShift, const std::vector<uint8_t> *const nAdd, const uint64_t nDifficulty, const Consensus::Params& params)
{
    std::vector<uint8_t> vHash(hash.begin(), hash.end());

    PoW pow(&vHash, nShift, nAdd, nDifficulty);

    // Check proof of work matches claimed amount
    if (!pow.valid())
        return error("CheckProofOfWork() : hash does not match nDifficulty");

    return true;
}
