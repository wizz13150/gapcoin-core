// Copyright (c) 2015-2017 The Bitcoin Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <cstddef>
#include <chain.h>
#include <chainparams.h>
#include <pow.h>
#include <random.h>
#include <util.h>
#include <test/test_bitcoin.h>

#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE(pow_tests, BasicTestingSetup)

/* Test calculation of next difficulty target with no constraints applying */
BOOST_AUTO_TEST_CASE(get_next_work)
{
    const auto chainParams = CreateChainParams(CBaseChainParams::MAIN);
    // int64_t nLastRetargetTime = 1261130161; // Block #30240
    CBlockIndex pindexLast;
    pindexLast.nHeight = 32255;
    pindexLast.nTime = 1418407764;  // Block #32255
    pindexLast.nDifficulty = 0x1d00ffff; // actually 24.59237286
    CBlock pblock;
    BOOST_CHECK_EQUAL(GetNextWorkRequired(&pindexLast, &pblock, chainParams->GetConsensus()), 0xffff00001d00ffffU /*0x1d00d86aU*/);
}

/* Test the constraint on the upper bound for next work */
BOOST_AUTO_TEST_CASE(get_next_work_pow_limit)
{
    const auto chainParams = CreateChainParams(CBaseChainParams::MAIN);
    // int64_t nLastRetargetTime = 1231006505; // Block #0
    CBlockIndex pindexLast;
    pindexLast.nHeight = 2015;
    pindexLast.nTime = 1414163232;  // Block #2015
    pindexLast.nDifficulty = 0x1d00ffff; // actually 20.26221364
    CBlock pblock;
    BOOST_CHECK_EQUAL(GetNextWorkRequired(&pindexLast, &pblock, chainParams->GetConsensus()), 0xffff00001d00ffffU /*0x1d00ffffU*/);
}

/* Test the constraint on the lower bound for actual time taken */
BOOST_AUTO_TEST_CASE(get_next_work_lower_limit_actual)
{
    const auto chainParams = CreateChainParams(CBaseChainParams::MAIN);
    // int64_t nLastRetargetTime = 1279008237; // Block #66528
    CBlockIndex pindexLast;
    pindexLast.nHeight = 68543;
    pindexLast.nTime = 1423443655;  // Block #68543
    pindexLast.nDifficulty = 0x1c05a3f4; // actually 23.71308238
    CBlock pblock;
    // BOOST_CHECK_EQUAL(GetNextWorkRequired(&pindexLast, nLastRetargetTime, chainParams->GetConsensus()), 0x1c0168fdU);
    BOOST_CHECK_EQUAL(GetNextWorkRequired(&pindexLast, &pblock, chainParams->GetConsensus()), 0xffff00001c05a3f4U /*0x1c0168fdU*/);
}

/* Test the constraint on the upper bound for actual time taken */
BOOST_AUTO_TEST_CASE(get_next_work_upper_limit_actual)
{
    const auto chainParams = CreateChainParams(CBaseChainParams::MAIN);
    // int64_t nLastRetargetTime = 1263163443; // NOTE: Not an actual block time
    CBlockIndex pindexLast;
    pindexLast.nHeight = 46367;
    pindexLast.nTime = 1420363245;  // Block #46367
    pindexLast.nDifficulty = 0x1c387f6f; // actually 24.00984862
    CBlock pblock;
    // BOOST_CHECK_EQUAL(GetNextWorkRequired(&pindexLast, nLastRetargetTime, chainParams->GetConsensus()), 0x1d00e1fdU);
    BOOST_CHECK_EQUAL(GetNextWorkRequired(&pindexLast, &pblock, chainParams->GetConsensus()), 0xffff00001c387f6fU /*0x1d00e1fdU*/);
}

BOOST_AUTO_TEST_CASE(CheckProofOfWork_test_negative_target)
{
    const auto consensus = CreateChainParams(CBaseChainParams::MAIN)->GetConsensus();
    uint256 hash;
    unsigned int nDifficulty;
    // nDifficulty = UintToArith256(consensus.powLimit).GetCompact(true);
    uint256 hashTarget = uint256S("7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");
    nDifficulty = UintToArith256(hashTarget).GetCompact(true);
    hash.SetHex("0x1");
    uint16_t nShift = 20;
    std::vector<unsigned char> nAdd;
    nAdd.assign(1, 0);
    BOOST_CHECK(!CheckProofOfWork(hash, nShift, &nAdd, nDifficulty, consensus));
}

BOOST_AUTO_TEST_CASE(CheckProofOfWork_test_overflow_target)
{
    const auto consensus = CreateChainParams(CBaseChainParams::MAIN)->GetConsensus();
    uint256 hash;
    unsigned int nDifficulty = ~0x00800000;
    hash.SetHex("0x1");
    uint16_t nShift = 20;
    std::vector<unsigned char> nAdd;
    nAdd.assign(1, 0);
    BOOST_CHECK(!CheckProofOfWork(hash, nShift, &nAdd, nDifficulty, consensus));
}

BOOST_AUTO_TEST_CASE(CheckProofOfWork_test_too_easy_target)
{
    const auto consensus = CreateChainParams(CBaseChainParams::MAIN)->GetConsensus();
    uint256 hash;
    unsigned int nDifficulty;
    // arith_uint256 nDifficulty_arith = UintToArith256(consensus.powLimit);
    uint256 hashTarget = uint256S("7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");
    arith_uint256 nDifficulty_arith = UintToArith256(hashTarget);
    nDifficulty_arith *= 2;
    nDifficulty = nDifficulty_arith.GetCompact();
    hash.SetHex("0x1");
    uint16_t nShift = 20;
    std::vector<unsigned char> nAdd;
    nAdd.assign(1, 0);
    BOOST_CHECK(!CheckProofOfWork(hash, nShift, &nAdd, nDifficulty, consensus));
}

BOOST_AUTO_TEST_CASE(CheckProofOfWork_test_bigger_hash_than_target)
{
    const auto consensus = CreateChainParams(CBaseChainParams::MAIN)->GetConsensus();
    uint256 hash;
    unsigned int nDifficulty;
    // arith_uint256 hash_arith = UintToArith256(consensus.powLimit);
    uint256 hashTarget = uint256S("7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");
    arith_uint256 hash_arith = UintToArith256(hashTarget);
    nDifficulty = hash_arith.GetCompact();
    hash_arith *= 2; // hash > nBits
    hash = ArithToUint256(hash_arith);
    uint16_t nShift = 20;
    std::vector<unsigned char> nAdd;
    nAdd.assign(1, 0);
    BOOST_CHECK(!CheckProofOfWork(hash, nShift, &nAdd, nDifficulty, consensus));
}

BOOST_AUTO_TEST_CASE(CheckProofOfWork_test_zero_target) // Fails for Bitcoin CheckPoW, passes for Gapcoin CheckPoW
{
    const auto consensus = CreateChainParams(CBaseChainParams::MAIN)->GetConsensus();
    uint256 hash;
    unsigned int nDifficulty;
    arith_uint256 hash_arith{0};
    nDifficulty = hash_arith.GetCompact();
    hash = ArithToUint256(hash_arith);
    uint16_t nShift = 20;
    std::vector<unsigned char> nAdd;
    nAdd.assign(1, 0);
    BOOST_CHECK(CheckProofOfWork(hash, nShift, &nAdd, nDifficulty, consensus));
}

BOOST_AUTO_TEST_CASE(GetBlockProofEquivalentTime_test)
{
    const auto chainParams = CreateChainParams(CBaseChainParams::MAIN);
    std::vector<CBlockIndex> blocks(10000);
    for (int i = 0; i < 10000; i++) {
        blocks[i].pprev = i ? &blocks[i - 1] : nullptr;
        blocks[i].nHeight = i;
        blocks[i].nTime = 1413914801 + i * chainParams->GetConsensus().nPowTargetSpacing;
        blocks[i].nDifficulty = 0x207fffff; /* target 0x7fffff000... */
        blocks[i].nChainWork = i ? blocks[i - 1].nChainWork + GetBlockProof(blocks[i - 1]) : arith_uint256(0);
    }

    for (int j = 0; j < 1000; j++) {
        CBlockIndex *p1 = &blocks[InsecureRandRange(10000)];
        CBlockIndex *p2 = &blocks[InsecureRandRange(10000)];
        CBlockIndex *p3 = &blocks[InsecureRandRange(10000)];

        int64_t tdiff = GetBlockProofEquivalentTime(*p1, *p2, *p3, chainParams->GetConsensus());
        BOOST_CHECK_EQUAL(tdiff, p1->GetBlockTime() - p2->GetBlockTime());
    }
}

BOOST_AUTO_TEST_SUITE_END()
