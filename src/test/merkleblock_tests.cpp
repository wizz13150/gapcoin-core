// Copyright (c) 2012-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <merkleblock.h>
#include <uint256.h>
#include <test/test_bitcoin.h>

#include <boost/test/unit_test.hpp>


BOOST_FIXTURE_TEST_SUITE(merkleblock_tests, BasicTestingSetup)

/**
 * Create a CMerkleBlock using a list of txids which will be found in the
 * given block.
 */
BOOST_AUTO_TEST_CASE(merkleblock_construct_from_txids_found)
{
    CBlock block = getBlockc5de9();
    std::set<uint256> txids;

    // Last txn in block.
    uint256 txhash1 = uint256S("0xf4f70528d16357691ab71b85d4b35166e3e6f9c23e1e8471debca097ec6dd218");

    // Second txn in block.
    uint256 txhash2 = uint256S("0x8c591c7c0c15d3ee64b568b234ae74d3319531c47189949891d3598b385566e1");

    txids.insert(txhash1);
    txids.insert(txhash2);

    CMerkleBlock merkleBlock(block, txids);

    BOOST_CHECK_EQUAL(merkleBlock.header.GetHash().GetHex(), block.GetHash().GetHex());

    // vMatchedTxn is only used when bloom filter is specified.
    BOOST_CHECK_EQUAL(merkleBlock.vMatchedTxn.size(), 0);

    std::vector<uint256> vMatched;
    std::vector<unsigned int> vIndex;

    BOOST_CHECK_EQUAL(merkleBlock.txn.ExtractMatches(vMatched, vIndex).GetHex(), block.hashMerkleRoot.GetHex());
    BOOST_CHECK_EQUAL(vMatched.size(), 2);

    // Ordered by occurrence in depth-first tree traversal.
    BOOST_CHECK_EQUAL(vMatched[0].ToString(), txhash2.ToString());
    BOOST_CHECK_EQUAL(vIndex[0], 1);

    BOOST_CHECK_EQUAL(vMatched[1].ToString(), txhash1.ToString());
    BOOST_CHECK_EQUAL(vIndex[1], 8);
}


/**
 * Create a CMerkleBlock using a list of txids which will not be found in the
 * given block.
 */
BOOST_AUTO_TEST_CASE(merkleblock_construct_from_txids_not_found)
{
    CBlock block = getBlockc5de9();
    std::set<uint256> txids2;
    txids2.insert(uint256S("0xe030c34058fd515d4b9c54645cd18e031cf314cda61c631369c934858cecc6b3"));
    CMerkleBlock merkleBlock(block, txids2);

    BOOST_CHECK_EQUAL(merkleBlock.header.GetHash().GetHex(), block.GetHash().GetHex());
    BOOST_CHECK_EQUAL(merkleBlock.vMatchedTxn.size(), 0);

    std::vector<uint256> vMatched;
    std::vector<unsigned int> vIndex;

    BOOST_CHECK_EQUAL(merkleBlock.txn.ExtractMatches(vMatched, vIndex).GetHex(), block.hashMerkleRoot.GetHex());
    BOOST_CHECK_EQUAL(vMatched.size(), 0);
    BOOST_CHECK_EQUAL(vIndex.size(), 0);
}

BOOST_AUTO_TEST_SUITE_END()
