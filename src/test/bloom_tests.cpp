// Copyright (c) 2012-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <bloom.h>

#include <base58.h>
#include <clientversion.h>
#include <key.h>
#include <merkleblock.h>
#include <primitives/block.h>
#include <random.h>
#include <serialize.h>
#include <streams.h>
#include <uint256.h>
#include <util.h>
#include <utilstrencodings.h>
#include <test/test_bitcoin.h>

#include <vector>

#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE(bloom_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(bloom_create_insert_serialize)
{
    CBloomFilter filter(3, 0.01, 0, BLOOM_UPDATE_ALL);

    filter.insert(ParseHex("99108ad8ed9bb6274d3980bab5a85c048f0950c8"));
    BOOST_CHECK_MESSAGE( filter.contains(ParseHex("99108ad8ed9bb6274d3980bab5a85c048f0950c8")), "Bloom filter doesn't contain just-inserted object!");
    // One bit different in first byte
    BOOST_CHECK_MESSAGE(!filter.contains(ParseHex("19108ad8ed9bb6274d3980bab5a85c048f0950c8")), "Bloom filter contains something it shouldn't!");

    filter.insert(ParseHex("b5a2c786d9ef4658287ced5914b37a1b4aa32eee"));
    BOOST_CHECK_MESSAGE(filter.contains(ParseHex("b5a2c786d9ef4658287ced5914b37a1b4aa32eee")), "Bloom filter doesn't contain just-inserted object (2)!");

    filter.insert(ParseHex("b9300670b4c5366e95b2699e8b18bc75e5f729c5"));
    BOOST_CHECK_MESSAGE(filter.contains(ParseHex("b9300670b4c5366e95b2699e8b18bc75e5f729c5")), "Bloom filter doesn't contain just-inserted object (3)!");

    CDataStream stream(SER_NETWORK, PROTOCOL_VERSION);
    stream << filter;

    std::vector<unsigned char> vch = ParseHex("03614e9b050000000000000001");
    std::vector<char> expected(vch.size());

    for (unsigned int i = 0; i < vch.size(); i++)
        expected[i] = (char)vch[i];

    BOOST_CHECK_EQUAL_COLLECTIONS(stream.begin(), stream.end(), expected.begin(), expected.end());

    BOOST_CHECK_MESSAGE( filter.contains(ParseHex("99108ad8ed9bb6274d3980bab5a85c048f0950c8")), "Bloom filter doesn't contain just-inserted object!");
    filter.clear();
    BOOST_CHECK_MESSAGE( !filter.contains(ParseHex("99108ad8ed9bb6274d3980bab5a85c048f0950c8")), "Bloom filter should be empty!");
}

BOOST_AUTO_TEST_CASE(bloom_create_insert_serialize_with_tweak)
{
    // Same test as bloom_create_insert_serialize, but we add a nTweak of 100
    CBloomFilter filter(3, 0.01, 2147483649UL, BLOOM_UPDATE_ALL);

    filter.insert(ParseHex("99108ad8ed9bb6274d3980bab5a85c048f0950c8"));
    BOOST_CHECK_MESSAGE( filter.contains(ParseHex("99108ad8ed9bb6274d3980bab5a85c048f0950c8")), "Bloom filter doesn't contain just-inserted object!");
    // One bit different in first byte
    BOOST_CHECK_MESSAGE(!filter.contains(ParseHex("19108ad8ed9bb6274d3980bab5a85c048f0950c8")), "Bloom filter contains something it shouldn't!");

    filter.insert(ParseHex("b5a2c786d9ef4658287ced5914b37a1b4aa32eee"));
    BOOST_CHECK_MESSAGE(filter.contains(ParseHex("b5a2c786d9ef4658287ced5914b37a1b4aa32eee")), "Bloom filter doesn't contain just-inserted object (2)!");

    filter.insert(ParseHex("b9300670b4c5366e95b2699e8b18bc75e5f729c5"));
    BOOST_CHECK_MESSAGE(filter.contains(ParseHex("b9300670b4c5366e95b2699e8b18bc75e5f729c5")), "Bloom filter doesn't contain just-inserted object (3)!");

    CDataStream stream(SER_NETWORK, PROTOCOL_VERSION);
    stream << filter;

    std::vector<unsigned char> vch = ParseHex("03ce4299050000000100008001");
    std::vector<char> expected(vch.size());

    for (unsigned int i = 0; i < vch.size(); i++)
        expected[i] = (char)vch[i];

    BOOST_CHECK_EQUAL_COLLECTIONS(stream.begin(), stream.end(), expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE(bloom_create_insert_key)
{
    std::string strSecret = std::string("4G15gmx24XgoEdAsmJzkqYmBWZZjheDeyXStwQULAmhVUYCTV8J");
    CBitcoinSecret vchSecret;
    BOOST_CHECK(vchSecret.SetString(strSecret));

    CKey key = vchSecret.GetKey();
    CPubKey pubkey = key.GetPubKey();
    std::vector<unsigned char> vchPubKey(pubkey.begin(), pubkey.end());

    CBloomFilter filter(2, 0.001, 0, BLOOM_UPDATE_ALL);
    filter.insert(vchPubKey);
    uint160 hash = pubkey.GetID();
    filter.insert(std::vector<unsigned char>(hash.begin(), hash.end()));

    CDataStream stream(SER_NETWORK, PROTOCOL_VERSION);
    stream << filter;

    std::vector<unsigned char> vch = ParseHex("03e56496080000000000000001");
    std::vector<char> expected(vch.size());

    for (unsigned int i = 0; i < vch.size(); i++)
        expected[i] = (char)vch[i];
    // std::cout << "sed -i 's/038fc16b080000000000000001/" << HexStr(stream.begin(), stream.end()) << "/' src/test/bloom_tests.cpp" << std::endl;
    BOOST_CHECK_EQUAL_COLLECTIONS(stream.begin(), stream.end(), expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE(bloom_match)
{
    // Random real transaction (b4749f017444b051c44dfd2720e88f314ff94f3dd6d56d40ef65854fcd7fff6b)
    CDataStream stream(ParseHex("01000000010b26e9b7735eb6aabdf358bab62f9816a21ba9ebdb719d5299e88607d722c190000000008b4830450220070aca44506c5cef3a16ed519d7c3c39f8aab192c4e1c90d065f37b8a4af6141022100a8e160b856c2d43d27d8fba71e5aef6405b8643ac4cb7cb3c462aced7f14711a0141046d11fee51b0e60666d5049a9101a72741df480b96ee26488a4d3466b95c9a40ac5eeef87e10a5cd336c19a84565f80fa6c547957b7700ff4dfbdefe76036c339ffffffff021bff3d11000000001976a91404943fdd508053c75000106d3bc6e2754dbcff1988ac2f15de00000000001976a914a266436d2965547608b9e15d9032a7b9d64fa43188ac00000000"), SER_DISK, CLIENT_VERSION);
    CTransaction tx(deserialize, stream);

    // and one which spends it (e2769b09e784f32f62ef849763d4f45b98e07ba658647343b915ff832b110436)
    unsigned char ch[] = {0x01, 0x00, 0x00, 0x00, 0x01, 0x6b, 0xff, 0x7f, 0xcd, 0x4f, 0x85, 0x65, 0xef, 0x40, 0x6d, 0xd5, 0xd6, 0x3d, 0x4f, 0xf9, 0x4f, 0x31, 0x8f, 0xe8, 0x20, 0x27, 0xfd, 0x4d, 0xc4, 0x51, 0xb0, 0x44, 0x74, 0x01, 0x9f, 0x74, 0xb4, 0x00, 0x00, 0x00, 0x00, 0x8c, 0x49, 0x30, 0x46, 0x02, 0x21, 0x00, 0xda, 0x0d, 0xc6, 0xae, 0xce, 0xfe, 0x1e, 0x06, 0xef, 0xdf, 0x05, 0x77, 0x37, 0x57, 0xde, 0xb1, 0x68, 0x82, 0x09, 0x30, 0xe3, 0xb0, 0xd0, 0x3f, 0x46, 0xf5, 0xfc, 0xf1, 0x50, 0xbf, 0x99, 0x0c, 0x02, 0x21, 0x00, 0xd2, 0x5b, 0x5c, 0x87, 0x04, 0x00, 0x76, 0xe4, 0xf2, 0x53, 0xf8, 0x26, 0x2e, 0x76, 0x3e, 0x2d, 0xd5, 0x1e, 0x7f, 0xf0, 0xbe, 0x15, 0x77, 0x27, 0xc4, 0xbc, 0x42, 0x80, 0x7f, 0x17, 0xbd, 0x39, 0x01, 0x41, 0x04, 0xe6, 0xc2, 0x6e, 0xf6, 0x7d, 0xc6, 0x10, 0xd2, 0xcd, 0x19, 0x24, 0x84, 0x78, 0x9a, 0x6c, 0xf9, 0xae, 0xa9, 0x93, 0x0b, 0x94, 0x4b, 0x7e, 0x2d, 0xb5, 0x34, 0x2b, 0x9d, 0x9e, 0x5b, 0x9f, 0xf7, 0x9a, 0xff, 0x9a, 0x2e, 0xe1, 0x97, 0x8d, 0xd7, 0xfd, 0x01, 0xdf, 0xc5, 0x22, 0xee, 0x02, 0x28, 0x3d, 0x3b, 0x06, 0xa9, 0xd0, 0x3a, 0xcf, 0x80, 0x96, 0x96, 0x8d, 0x7d, 0xbb, 0x0f, 0x91, 0x78, 0xff, 0xff, 0xff, 0xff, 0x02, 0x8b, 0xa7, 0x94, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x19, 0x76, 0xa9, 0x14, 0xba, 0xde, 0xec, 0xfd, 0xef, 0x05, 0x07, 0x24, 0x7f, 0xc8, 0xf7, 0x42, 0x41, 0xd7, 0x3b, 0xc0, 0x39, 0x97, 0x2d, 0x7b, 0x88, 0xac, 0x40, 0x94, 0xa8, 0x02, 0x00, 0x00, 0x00, 0x00, 0x19, 0x76, 0xa9, 0x14, 0xc1, 0x09, 0x32, 0x48, 0x3f, 0xec, 0x93, 0xed, 0x51, 0xf5, 0xfe, 0x95, 0xe7, 0x25, 0x59, 0xf2, 0xcc, 0x70, 0x43, 0xf9, 0x88, 0xac, 0x00, 0x00, 0x00, 0x00, 0x00};
    std::vector<unsigned char> vch(ch, ch + sizeof(ch) -1);
    CDataStream spendStream(vch, SER_DISK, CLIENT_VERSION);
    CTransaction spendingTx(deserialize, spendStream);

    CBloomFilter filter(10, 0.000001, 0, BLOOM_UPDATE_ALL);
    filter.insert(uint256S("0xb4749f017444b051c44dfd2720e88f314ff94f3dd6d56d40ef65854fcd7fff6b"));
    BOOST_CHECK_MESSAGE(filter.IsRelevantAndUpdate(tx), "Simple Bloom filter didn't match tx hash");

    filter = CBloomFilter(10, 0.000001, 0, BLOOM_UPDATE_ALL);
    // byte-reversed tx hash
    filter.insert(ParseHex("6bff7fcd4f8565ef406dd5d63d4ff94f318fe82027fd4dc451b04474019f74b4"));
    BOOST_CHECK_MESSAGE(filter.IsRelevantAndUpdate(tx), "Simple Bloom filter didn't match manually serialized tx hash");

    filter = CBloomFilter(10, 0.000001, 0, BLOOM_UPDATE_ALL);
    filter.insert(ParseHex("30450220070aca44506c5cef3a16ed519d7c3c39f8aab192c4e1c90d065f37b8a4af6141022100a8e160b856c2d43d27d8fba71e5aef6405b8643ac4cb7cb3c462aced7f14711a01"));
    BOOST_CHECK_MESSAGE(filter.IsRelevantAndUpdate(tx), "Simple Bloom filter didn't match input signature");

    filter = CBloomFilter(10, 0.000001, 0, BLOOM_UPDATE_ALL);
    filter.insert(ParseHex("046d11fee51b0e60666d5049a9101a72741df480b96ee26488a4d3466b95c9a40ac5eeef87e10a5cd336c19a84565f80fa6c547957b7700ff4dfbdefe76036c339"));
    BOOST_CHECK_MESSAGE(filter.IsRelevantAndUpdate(tx), "Simple Bloom filter didn't match input pub key");

    filter = CBloomFilter(10, 0.000001, 0, BLOOM_UPDATE_ALL);
    filter.insert(ParseHex("04943fdd508053c75000106d3bc6e2754dbcff19"));
    BOOST_CHECK_MESSAGE(filter.IsRelevantAndUpdate(tx), "Simple Bloom filter didn't match output address");
    BOOST_CHECK_MESSAGE(filter.IsRelevantAndUpdate(spendingTx), "Simple Bloom filter didn't add output");

    filter = CBloomFilter(10, 0.000001, 0, BLOOM_UPDATE_ALL);
    filter.insert(ParseHex("a266436d2965547608b9e15d9032a7b9d64fa431"));
    BOOST_CHECK_MESSAGE(filter.IsRelevantAndUpdate(tx), "Simple Bloom filter didn't match output address");

    filter = CBloomFilter(10, 0.000001, 0, BLOOM_UPDATE_ALL);
    filter.insert(COutPoint(uint256S("0x90c122d70786e899529d71dbeba91ba216982fb6ba58f3bdaab65e73b7e9260b"), 0));
    BOOST_CHECK_MESSAGE(filter.IsRelevantAndUpdate(tx), "Simple Bloom filter didn't match COutPoint");

    filter = CBloomFilter(10, 0.000001, 0, BLOOM_UPDATE_ALL);
    COutPoint prevOutPoint(uint256S("0x90c122d70786e899529d71dbeba91ba216982fb6ba58f3bdaab65e73b7e9260b"), 0);
    {
        std::vector<unsigned char> data(32 + sizeof(unsigned int));
        memcpy(data.data(), prevOutPoint.hash.begin(), 32);
        memcpy(data.data()+32, &prevOutPoint.n, sizeof(unsigned int));
        filter.insert(data);
    }
    BOOST_CHECK_MESSAGE(filter.IsRelevantAndUpdate(tx), "Simple Bloom filter didn't match manually serialized COutPoint");

    filter = CBloomFilter(10, 0.000001, 0, BLOOM_UPDATE_ALL);
    filter.insert(uint256S("00000009e784f32f62ef849763d4f45b98e07ba658647343b915ff832b110436"));
    BOOST_CHECK_MESSAGE(!filter.IsRelevantAndUpdate(tx), "Simple Bloom filter matched random tx hash");

    filter = CBloomFilter(10, 0.000001, 0, BLOOM_UPDATE_ALL);
    filter.insert(ParseHex("0000006d2965547608b9e15d9032a7b9d64fa431"));
    BOOST_CHECK_MESSAGE(!filter.IsRelevantAndUpdate(tx), "Simple Bloom filter matched random address");

    filter = CBloomFilter(10, 0.000001, 0, BLOOM_UPDATE_ALL);
    filter.insert(COutPoint(uint256S("0x90c122d70786e899529d71dbeba91ba216982fb6ba58f3bdaab65e73b7e9260b"), 1));
    BOOST_CHECK_MESSAGE(!filter.IsRelevantAndUpdate(tx), "Simple Bloom filter matched COutPoint for an output we didn't care about");

    filter = CBloomFilter(10, 0.000001, 0, BLOOM_UPDATE_ALL);
    filter.insert(COutPoint(uint256S("0x000000d70786e899529d71dbeba91ba216982fb6ba58f3bdaab65e73b7e9260b"), 0));
    BOOST_CHECK_MESSAGE(!filter.IsRelevantAndUpdate(tx), "Simple Bloom filter matched COutPoint for an output we didn't care about");
}

BOOST_AUTO_TEST_CASE(merkle_block_1)
{
    CBlock block = getBlockc5de9();
    CBloomFilter filter(10, 0.000001, 0, BLOOM_UPDATE_ALL);
    // Match the last transaction
    filter.insert(uint256S("0xf4f70528d16357691ab71b85d4b35166e3e6f9c23e1e8471debca097ec6dd218"));

    CMerkleBlock merkleBlock(block, filter);
    BOOST_CHECK_EQUAL(merkleBlock.header.GetHash().GetHex(), block.GetHash().GetHex());

    BOOST_CHECK_EQUAL(merkleBlock.vMatchedTxn.size(), 1);
    std::pair<unsigned int, uint256> pair = merkleBlock.vMatchedTxn[0];

    BOOST_CHECK(merkleBlock.vMatchedTxn[0].second == uint256S("0xf4f70528d16357691ab71b85d4b35166e3e6f9c23e1e8471debca097ec6dd218"));
    BOOST_CHECK(merkleBlock.vMatchedTxn[0].first == 8);

    std::vector<uint256> vMatched;
    std::vector<unsigned int> vIndex;
    BOOST_CHECK(merkleBlock.txn.ExtractMatches(vMatched, vIndex) == block.hashMerkleRoot);
    BOOST_CHECK(vMatched.size() == merkleBlock.vMatchedTxn.size());
    for (unsigned int i = 0; i < vMatched.size(); i++)
        BOOST_CHECK(vMatched[i] == merkleBlock.vMatchedTxn[i].second);

    // Also match the 8th transaction
    filter.insert(uint256S("0x0ed69954e883b1596aa0b5754e30813d0a64640d4441f779589be7428754670f"));
    merkleBlock = CMerkleBlock(block, filter);
    BOOST_CHECK(merkleBlock.header.GetHash() == block.GetHash());

    BOOST_CHECK(merkleBlock.vMatchedTxn.size() == 2);

    BOOST_CHECK(merkleBlock.vMatchedTxn[1] == pair);

    BOOST_CHECK(merkleBlock.vMatchedTxn[0].second == uint256S("0x0ed69954e883b1596aa0b5754e30813d0a64640d4441f779589be7428754670f"));
    BOOST_CHECK(merkleBlock.vMatchedTxn[0].first == 7);

    BOOST_CHECK(merkleBlock.txn.ExtractMatches(vMatched, vIndex) == block.hashMerkleRoot);
    BOOST_CHECK(vMatched.size() == merkleBlock.vMatchedTxn.size());
    for (unsigned int i = 0; i < vMatched.size(); i++)
        BOOST_CHECK(vMatched[i] == merkleBlock.vMatchedTxn[i].second);
}

BOOST_AUTO_TEST_CASE(merkle_block_2)
{
    BOOST_TEST_MESSAGE("bloom_tests, merkle_block_2 partly skipped, hard-coded to use the Bitcoin blockchain");
    // Random real block (868728c9844054f64756bdc08d7684b8817426cb165162c974bba25bee5cd155)
    // With 4 txes
    CBlock block;
    CDataStream stream(ParseHex("02000000ef240bd3c4feac7c3d72e15ed28847594ec1ee3d1c9df4835b0407567de42dbd15c2a0ae1c26e09d5f9e342527023c85b5750566a2e21df6c361ee34a2f9221d99961b55d316185fa3af160008000000190004a1fd99010401000000010000000000000000000000000000000000000000000000000000000000000000ffffffff2603aa8601062f503253482f0499961b5508f0000001000001060c2f6e6f6e63652d706f6f6c2f0000000001795f3887000000001976a914e079162838d088a9f6ff2774b6bcbe2d1fc3df0b88ac00000000010000000570cceb845da87625ee94c646f93774ee69c57b286a398cda669a0c8f70ddf50a000000006a473044022069fc5fbbe8117b486dab48615368bcf25f04cc5b7ea1b0d4333680fc9385c033022001c1477f2d002ce588fe35fbde810901d131e53e04190a574986a7e4a54366a9012103f394c9ffda83e8c9efe4328b6267b482d93b3661230b5e701f60054f02a894c1ffffffff235030dc25cc0a0ab606dec50b65610c39e2cce6660be325bc5fe6934dc72188000000006b483045022100c3fa5ad496bd2a15b85ad9548b0596a08439068da397c816548a0288d1bea0e202207da7a9cb8eac2ebfeb09262405303bdf71803f75f2e62cb3e54e572e847e683a012103f394c9ffda83e8c9efe4328b6267b482d93b3661230b5e701f60054f02a894c1ffffffff8ca561caba2439de22aeaf3175d39e56df858a5363cb0278cb55b33243a6269d000000006b483045022100d27c1b0f48ebc8d7cb3dd1acb354522d26fe877ad4e0a525a191a102e3bd8756022066b4e8305bacfa155f2ab4885d71d4ed3bc317f1598f0b40f3e34ce3ba7b2efe012103f394c9ffda83e8c9efe4328b6267b482d93b3661230b5e701f60054f02a894c1ffffffffcef1e9629f46d9035b82397ef54d10c800a3428bf34a1f1821b552de28a78910000000006a47304402204a34b1a028c68afc2c9e5098e559c6574af39b6b454fee4ff41ddf3adf25dcaa02207db0751aac899e33652901541c1217981410e2400077580aadb79eddda84fb62012103f394c9ffda83e8c9efe4328b6267b482d93b3661230b5e701f60054f02a894c1ffffffff08fb3cf58c1971e0cdef6118c5f57906ede59e41f642e4abd82a47d80da107c9000000006a47304402204d07c725a264676f0bae7635076516ad51b8ebc4967a3a9bb094312a124ad3b9022067e6b42184401d4dddd660bd06542d6c94e301867f3b51f0ef3b05094e7073b6012102429ab387a9dc17e4da51e482209d8fb5ab8e0ac5e32d597ee475f3c37bf8f221ffffffff023561380f000000001976a914b55aa139442358d08977b943e8188498084dc19b88acd76179a0020000001976a9141dfd1863451f393a200cf3880b2a607fb94cc99e88ac000000000100000001334fd98d911199c6758cfff14a1a7e7002ede9bdb3b812af8a770fde2e2f42bf000000006a47304402204bbcfb7a5d9b85aae2db2119cc16e99255b31517c4483f593ed7b7fe6793cb67022072726e1b810258d3e9214999dba23285a5d3f0fc429a329cd9a9f170dfd66106012102429ab387a9dc17e4da51e482209d8fb5ab8e0ac5e32d597ee475f3c37bf8f221ffffffff021e74060f000000001976a914d84e29655221bda887f1a7244fd487dd0e4e1e5788ac2b0b2978000000001976a914ddafeae3034f932aff07802e5b298448edf8639088ac0000000001000000019946bc44d3db23f8cfa837d9f65ce0a7508c689bc315280788462d894ddd86df010000006b483045022100f3c78ee9d1556aac85a14c29325ed0eafb7c4f5ebe0e09cd8d47642aa1bb2a5e0220292326a5d189282553e7b34252b6e8e5b532b2241addd28be8bc146514bd8b070121033625e51b8651ed314e162a97ef9f1bfdc39535a20ac6913d4716d427230b5ee6ffffffff02d26a4807000000001976a9141e027befa097ed716f413b98e06fe43dc0ae035788acd1fea02f000000001976a914d6b70c657375f395412ffbc4f15d01bc0db672eb88ac00000000"), SER_NETWORK, PROTOCOL_VERSION);
    stream >> block;

    CBloomFilter filter(10, 0.000001, 0, BLOOM_UPDATE_ALL);
    // Match the first transaction
    filter.insert(uint256S("0xd649a01c919b381523e50485a3dde71dc00baec5a6209a6748b83553856f51b0"));

    CMerkleBlock merkleBlock(block, filter);
    BOOST_CHECK(merkleBlock.header.GetHash() == block.GetHash());

    BOOST_CHECK(merkleBlock.vMatchedTxn.size() == 1);
    // std::pair<unsigned int, uint256> pair = merkleBlock.vMatchedTxn[0];

    BOOST_CHECK(merkleBlock.vMatchedTxn[0].second == uint256S("0xd649a01c919b381523e50485a3dde71dc00baec5a6209a6748b83553856f51b0"));
    BOOST_CHECK(merkleBlock.vMatchedTxn[0].first == 0);

    std::vector<uint256> vMatched;
    std::vector<unsigned int> vIndex;
    BOOST_CHECK(merkleBlock.txn.ExtractMatches(vMatched, vIndex) == block.hashMerkleRoot);
    BOOST_CHECK(vMatched.size() == merkleBlock.vMatchedTxn.size());
    for (unsigned int i = 0; i < vMatched.size(); i++)
        BOOST_CHECK(vMatched[i] == merkleBlock.vMatchedTxn[i].second);

    // // Match an output from the second transaction (the pubkey for address 1DZTzaBHUDM7T3QvUKBz4qXMRpkg8jsfB5)
    // // This should match the third transaction because it spends the output matched
    // // It also matches the fourth transaction, which spends to the pubkey again
    // filter.insert(ParseHex("044a656f065871a353f216ca26cef8dde2f03e8c16202d2e8ad769f02032cb86a5eb5e56842e92e19141d60a01928f8dd2c875a390f67c1f6c94cfc617c0ea45af"));

    // merkleBlock = CMerkleBlock(block, filter);
    // BOOST_CHECK(merkleBlock.header.GetHash() == block.GetHash());

    // BOOST_CHECK(merkleBlock.vMatchedTxn.size() == 4);

    // BOOST_CHECK(pair == merkleBlock.vMatchedTxn[0]);

    // BOOST_CHECK(merkleBlock.vMatchedTxn[1].second == uint256S("0x28204cad1d7fc1d199e8ef4fa22f182de6258a3eaafe1bbe56ebdcacd3069a5f"));
    // BOOST_CHECK(merkleBlock.vMatchedTxn[1].first == 1);

    // BOOST_CHECK(merkleBlock.vMatchedTxn[2].second == uint256S("0x6b0f8a73a56c04b519f1883e8aafda643ba61a30bd1439969df21bea5f4e27e2"));
    // BOOST_CHECK(merkleBlock.vMatchedTxn[2].first == 2);

    // BOOST_CHECK(merkleBlock.vMatchedTxn[3].second == uint256S("0x3c1d7e82342158e4109df2e0b6348b6e84e403d8b4046d7007663ace63cddb23"));
    // BOOST_CHECK(merkleBlock.vMatchedTxn[3].first == 3);

    // BOOST_CHECK(merkleBlock.txn.ExtractMatches(vMatched, vIndex) == block.hashMerkleRoot);
    // BOOST_CHECK(vMatched.size() == merkleBlock.vMatchedTxn.size());
    // for (unsigned int i = 0; i < vMatched.size(); i++)
    //     BOOST_CHECK(vMatched[i] == merkleBlock.vMatchedTxn[i].second);
}

BOOST_AUTO_TEST_CASE(merkle_block_2_with_update_none)
{
    BOOST_TEST_MESSAGE("bloom_tests, merkle_block_2_with_update_none skipped, hard-coded to use the Bitcoin blockchain");
    // // Random real block (000000005a4ded781e667e06ceefafb71410b511fe0d5adc3e5a27ecbec34ae6)
    // // With 4 txes
    // CBlock block;
    // CDataStream stream(ParseHex("0100000075616236cc2126035fadb38deb65b9102cc2c41c09cdf29fc051906800000000fe7d5e12ef0ff901f6050211249919b1c0653771832b3a80c66cea42847f0ae1d4d26e49ffff001d00f0a4410401000000010000000000000000000000000000000000000000000000000000000000000000ffffffff0804ffff001d029105ffffffff0100f2052a010000004341046d8709a041d34357697dfcb30a9d05900a6294078012bf3bb09c6f9b525f1d16d5503d7905db1ada9501446ea00728668fc5719aa80be2fdfc8a858a4dbdd4fbac00000000010000000255605dc6f5c3dc148b6da58442b0b2cd422be385eab2ebea4119ee9c268d28350000000049483045022100aa46504baa86df8a33b1192b1b9367b4d729dc41e389f2c04f3e5c7f0559aae702205e82253a54bf5c4f65b7428551554b2045167d6d206dfe6a2e198127d3f7df1501ffffffff55605dc6f5c3dc148b6da58442b0b2cd422be385eab2ebea4119ee9c268d2835010000004847304402202329484c35fa9d6bb32a55a70c0982f606ce0e3634b69006138683bcd12cbb6602200c28feb1e2555c3210f1dddb299738b4ff8bbe9667b68cb8764b5ac17b7adf0001ffffffff0200e1f505000000004341046a0765b5865641ce08dd39690aade26dfbf5511430ca428a3089261361cef170e3929a68aee3d8d4848b0c5111b0a37b82b86ad559fd2a745b44d8e8d9dfdc0cac00180d8f000000004341044a656f065871a353f216ca26cef8dde2f03e8c16202d2e8ad769f02032cb86a5eb5e56842e92e19141d60a01928f8dd2c875a390f67c1f6c94cfc617c0ea45afac0000000001000000025f9a06d3acdceb56be1bfeaa3e8a25e62d182fa24fefe899d1c17f1dad4c2028000000004847304402205d6058484157235b06028c30736c15613a28bdb768ee628094ca8b0030d4d6eb0220328789c9a2ec27ddaec0ad5ef58efded42e6ea17c2e1ce838f3d6913f5e95db601ffffffff5f9a06d3acdceb56be1bfeaa3e8a25e62d182fa24fefe899d1c17f1dad4c2028010000004a493046022100c45af050d3cea806cedd0ab22520c53ebe63b987b8954146cdca42487b84bdd6022100b9b027716a6b59e640da50a864d6dd8a0ef24c76ce62391fa3eabaf4d2886d2d01ffffffff0200e1f505000000004341046a0765b5865641ce08dd39690aade26dfbf5511430ca428a3089261361cef170e3929a68aee3d8d4848b0c5111b0a37b82b86ad559fd2a745b44d8e8d9dfdc0cac00180d8f000000004341046a0765b5865641ce08dd39690aade26dfbf5511430ca428a3089261361cef170e3929a68aee3d8d4848b0c5111b0a37b82b86ad559fd2a745b44d8e8d9dfdc0cac000000000100000002e2274e5fea1bf29d963914bd301aa63b64daaf8a3e88f119b5046ca5738a0f6b0000000048473044022016e7a727a061ea2254a6c358376aaa617ac537eb836c77d646ebda4c748aac8b0220192ce28bf9f2c06a6467e6531e27648d2b3e2e2bae85159c9242939840295ba501ffffffffe2274e5fea1bf29d963914bd301aa63b64daaf8a3e88f119b5046ca5738a0f6b010000004a493046022100b7a1a755588d4190118936e15cd217d133b0e4a53c3c15924010d5648d8925c9022100aaef031874db2114f2d869ac2de4ae53908fbfea5b2b1862e181626bb9005c9f01ffffffff0200e1f505000000004341044a656f065871a353f216ca26cef8dde2f03e8c16202d2e8ad769f02032cb86a5eb5e56842e92e19141d60a01928f8dd2c875a390f67c1f6c94cfc617c0ea45afac00180d8f000000004341046a0765b5865641ce08dd39690aade26dfbf5511430ca428a3089261361cef170e3929a68aee3d8d4848b0c5111b0a37b82b86ad559fd2a745b44d8e8d9dfdc0cac00000000"), SER_NETWORK, PROTOCOL_VERSION);
    // stream >> block;

    // CBloomFilter filter(10, 0.000001, 0, BLOOM_UPDATE_NONE);
    // // Match the first transaction
    // filter.insert(uint256S("0xe980fe9f792d014e73b95203dc1335c5f9ce19ac537a419e6df5b47aecb93b70"));

    // CMerkleBlock merkleBlock(block, filter);
    // BOOST_CHECK(merkleBlock.header.GetHash() == block.GetHash());

    // BOOST_CHECK(merkleBlock.vMatchedTxn.size() == 1);
    // std::pair<unsigned int, uint256> pair = merkleBlock.vMatchedTxn[0];

    // BOOST_CHECK(merkleBlock.vMatchedTxn[0].second == uint256S("0xe980fe9f792d014e73b95203dc1335c5f9ce19ac537a419e6df5b47aecb93b70"));
    // BOOST_CHECK(merkleBlock.vMatchedTxn[0].first == 0);

    // std::vector<uint256> vMatched;
    // std::vector<unsigned int> vIndex;
    // BOOST_CHECK(merkleBlock.txn.ExtractMatches(vMatched, vIndex) == block.hashMerkleRoot);
    // BOOST_CHECK(vMatched.size() == merkleBlock.vMatchedTxn.size());
    // for (unsigned int i = 0; i < vMatched.size(); i++)
    //     BOOST_CHECK(vMatched[i] == merkleBlock.vMatchedTxn[i].second);

    // // Match an output from the second transaction (the pubkey for address 1DZTzaBHUDM7T3QvUKBz4qXMRpkg8jsfB5)
    // // This should not match the third transaction though it spends the output matched
    // // It will match the fourth transaction, which has another pay-to-pubkey output to the same address
    // filter.insert(ParseHex("044a656f065871a353f216ca26cef8dde2f03e8c16202d2e8ad769f02032cb86a5eb5e56842e92e19141d60a01928f8dd2c875a390f67c1f6c94cfc617c0ea45af"));

    // merkleBlock = CMerkleBlock(block, filter);
    // BOOST_CHECK(merkleBlock.header.GetHash() == block.GetHash());

    // BOOST_CHECK(merkleBlock.vMatchedTxn.size() == 3);

    // BOOST_CHECK(pair == merkleBlock.vMatchedTxn[0]);

    // BOOST_CHECK(merkleBlock.vMatchedTxn[1].second == uint256S("0x28204cad1d7fc1d199e8ef4fa22f182de6258a3eaafe1bbe56ebdcacd3069a5f"));
    // BOOST_CHECK(merkleBlock.vMatchedTxn[1].first == 1);

    // BOOST_CHECK(merkleBlock.vMatchedTxn[2].second == uint256S("0x3c1d7e82342158e4109df2e0b6348b6e84e403d8b4046d7007663ace63cddb23"));
    // BOOST_CHECK(merkleBlock.vMatchedTxn[2].first == 3);

    // BOOST_CHECK(merkleBlock.txn.ExtractMatches(vMatched, vIndex) == block.hashMerkleRoot);
    // BOOST_CHECK(vMatched.size() == merkleBlock.vMatchedTxn.size());
    // for (unsigned int i = 0; i < vMatched.size(); i++)
    //     BOOST_CHECK(vMatched[i] == merkleBlock.vMatchedTxn[i].second);
}

BOOST_AUTO_TEST_CASE(merkle_block_3_and_serialize)
{
    // Random real block (cf6ac463ead3f53fd03d6cce3ee92a0a2145180b8df2bbe72825c7cc004b9084)
    // With one tx
    CBlock block;
    CDataStream stream(ParseHex("02000000056d045d4ddecc18eadc60b0ed20980f90973f47897beec50a5dfceda65c6c830a28e215d2ee3dbd2ab9856ab257d1809ea8ae197b10ddf39d3f1d9774868402ab736b5f3139f34e4ccf15002202000019000377c1b40101000000010000000000000000000000000000000000000000000000000000000000000000ffffffff0d0397b0130109062f503253482fffffffff0193e13f1000000000232103851eff83e1f7fbe6c8f2a1743c613142f3d135ba952412bde9b063df9c7e1ca1ac00000000"), SER_NETWORK, PROTOCOL_VERSION);
    stream >> block;

    CBloomFilter filter(10, 0.000001, 0, BLOOM_UPDATE_ALL);
    // Match the only transaction
    filter.insert(uint256S("0x02848674971d3f9df3dd107b19aea89e80d157b26a85b92abd3deed215e2280a"));

    CMerkleBlock merkleBlock(block, filter);
    BOOST_CHECK(merkleBlock.header.GetHash() == block.GetHash());

    BOOST_CHECK(merkleBlock.vMatchedTxn.size() == 1);

    BOOST_CHECK(merkleBlock.vMatchedTxn[0].second == uint256S("0x02848674971d3f9df3dd107b19aea89e80d157b26a85b92abd3deed215e2280a"));
    BOOST_CHECK(merkleBlock.vMatchedTxn[0].first == 0);

    std::vector<uint256> vMatched;
    std::vector<unsigned int> vIndex;
    BOOST_CHECK(merkleBlock.txn.ExtractMatches(vMatched, vIndex) == block.hashMerkleRoot);
    BOOST_CHECK(vMatched.size() == merkleBlock.vMatchedTxn.size());
    for (unsigned int i = 0; i < vMatched.size(); i++)
        BOOST_CHECK(vMatched[i] == merkleBlock.vMatchedTxn[i].second);

    CDataStream merkleStream(SER_NETWORK, PROTOCOL_VERSION);
    merkleStream << merkleBlock;

    // std::cout << "sed -i 's/" << HexStr(merkleStream.begin(), merkleStream.end()) << "/' src/test/bloom_tests.cpp" << std::endl;

    std::vector<unsigned char> vch = ParseHex("02000000056d045d4ddecc18eadc60b0ed20980f90973f47897beec50a5dfceda65c6c830a28e215d2ee3dbd2ab9856ab257d1809ea8ae197b10ddf39d3f1d9774868402ab736b5f3139f34e4ccf15002202000019000377c1b401000000010a28e215d2ee3dbd2ab9856ab257d1809ea8ae197b10ddf39d3f1d97748684020101");
    std::vector<char> expected(vch.size());

    for (unsigned int i = 0; i < vch.size(); i++)
        expected[i] = (char)vch[i];

    BOOST_CHECK_EQUAL_COLLECTIONS(expected.begin(), expected.end(), merkleStream.begin(), merkleStream.end());
}

BOOST_AUTO_TEST_CASE(merkle_block_4)
{
    // Random real block (8192bda9bc7479e5421335dce24c5be7801980880ef8a27932e6b79f4ea2a0af)
    // With 7 txes
    CBlock block;
    CDataStream stream(ParseHex("020000003aa8232d857917ec56f61d1281d9d15e59b325afa12a90e93cd096f950ff0de6ade5dedc85315ddbef612596020d5b70852a2830e350723a1062d0f6315a5e77a08c1f55fee5eb9847ae160047000000190003b9b7500701000000010000000000000000000000000000000000000000000000000000000000000000ffffffff2603de8d01062f503253482f04a08c1f5508e8005791000000000c2f6e6f6e63652d706f6f6c2f0000000001b9463087000000001976a914eca11686184905fc0973454ab6594180545b5f8188ac0000000001000000012ca078d9054b45072f68c25b29a9afeda2209e8e2c6b23dc577fc48d5159dfae000000006b483045022100d83135ddff47277503d3f73683111698e5c92a3a094dcccaf407f9fdebf5ec140220564271ea761ab3be85a08f0f3f8485e07a526f437fecaaa2e43b61e50190de2b012103f394c9ffda83e8c9efe4328b6267b482d93b3661230b5e701f60054f02a894c1ffffffff02e846fc05000000001976a91473085ed6b5216520d775c1f9382065275c23b9a288acfbd9ee80000000001976a914f1ab119148d12c639f118a919f416bb22fb2145688ac00000000010000000147da75b64539d9384a51b52a7f5069f41e641a36b5c61e5fa1d3d832d53801f1000000006b4830450221008d62bbeccc23ebc5b0c6047b4b073e60b89bc9d7e3801a219bfe754c9b7edea602202ca24926629fd3b168010f02bab3a1f4052cb170184f122295ed7a36f5737495012102429ab387a9dc17e4da51e482209d8fb5ab8e0ac5e32d597ee475f3c37bf8f221ffffffff02098d9b06000000001976a9141e027befa097ed716f413b98e06fe43dc0ae035788ac0a9f4d80000000001976a914b52fdfd34cf0b2ba6a97478ff0783354650257c488ac0000000001000000018fdf6e8b95856dea58ff6284f74fd234c93e763314c680a759b56e1cf578a1e7000000006b4830450221009829fa400fe276289ff4efbe665d68690ef88c8e82c54075d8dc62294a67dfe10220422b60070e1dd1c2ddebd92006f574b7e0ff0b5dc63fd4d8dda5940dd8af8acc012103f394c9ffda83e8c9efe4328b6267b482d93b3661230b5e701f60054f02a894c1ffffffff021cd08071000000001976a914a3096b8dc3c75646ca902a8e4944322ce8eb386788acf2166c15000000001976a914f663e9a46473370b4e8e4c69c61e081f35a21db988ac00000000010000000126d7adb9973c7d2e20c0de6c62f2794ba948f85f3fd1ed957ccfcc20e0300ab8000000006a47304402201d9691aaefa369564445173d2e3c6ab8d0cff67a63fa310bfad7a99299576add022040006c83a6e3080b331dfca35700466ea7fd7933147b5a4cf15d76437c23c3ad012103f394c9ffda83e8c9efe4328b6267b482d93b3661230b5e701f60054f02a894c1ffffffff02107e9706000000001976a9142d7074dd3f3540f2626f85ba4fc0c09897d83c9888acf1295380000000001976a914c82d4f96af4ef2d5ae987c4406f1c282101c738788ac000000000100000001e4f42b1635740268bd925f2452b307bbe511f8b718310736d6bc39ecc4dad0ac000000006a473044022069d94bf47af110172475d1b16d6d39449e41b523329ee281ccce4b440e41560e02204f8c6516bbe4d6b8b507d1b614877680356b63fa1b896d8a1eaf8d6046bf364a012103f394c9ffda83e8c9efe4328b6267b482d93b3661230b5e701f60054f02a894c1ffffffff027673b948000000001976a914e5cda15cef0cd692ea3ab27f10c7ffeb03a1cf1e88acaa26333e000000001976a9146bbe0e6b5b22d504121e4e736c37ffb943f2148688ac000000000100000002995251d49668e099f377fe03b684412e053b5a0d11d94ae488e2f32e0aa2ba53000000006a4730440220212b9cdf6465808d1ea07e7928f6e6d389bd222e78d698b9159ca4da595a042e02200e27b07c61f2fdf81def81bc5ee5df85c12dffca322f7437fb565bcd2245cdce01210301b9768e01e4459c456a8618443a8ee6b8848a1ba13b6d27c2526908cf1375c9ffffffff175a1cda82e1cefddbcaeae8ad40b13ff929a8049b49fad66fda8456e7002a12010000006a47304402203e9fb7e144f1628cb08c93290f8bf3739b9e0a572d9803157453d3c190b1b7010220103f3a3434a60cd7cb2daed8c86d8610e3500dc70a4b1453aac7c0ef09ee0e710121028cf19ee337c9a194f1dae5a23a6a6c69cd3bed5da08fcc746812409d16ee46b7ffffffff0259fa493e000000001976a9149f977f10a7b7d9c6720e7e50e090738b7b7cfe5688ac44a8f700000000001976a914aa387c5a8978a0fb43b747cbfac2b8fb20800f3188ac00000000"), SER_NETWORK, PROTOCOL_VERSION);
    stream >> block;

    CBloomFilter filter(10, 0.000001, 0, BLOOM_UPDATE_ALL);
    // Match the last transaction
    filter.insert(uint256S("0x86d7f427ecafb1a959c15b33f6ff03cd1ace6aeddaaca70caa498d4ef56eaf01"));

    CMerkleBlock merkleBlock(block, filter);
    BOOST_CHECK(merkleBlock.header.GetHash() == block.GetHash());

    BOOST_CHECK(merkleBlock.vMatchedTxn.size() == 1);
    std::pair<unsigned int, uint256> pair = merkleBlock.vMatchedTxn[0];

    BOOST_CHECK(merkleBlock.vMatchedTxn[0].second == uint256S("0x86d7f427ecafb1a959c15b33f6ff03cd1ace6aeddaaca70caa498d4ef56eaf01"));
    BOOST_CHECK(merkleBlock.vMatchedTxn[0].first == 6);

    std::vector<uint256> vMatched;
    std::vector<unsigned int> vIndex;
    BOOST_CHECK(merkleBlock.txn.ExtractMatches(vMatched, vIndex) == block.hashMerkleRoot);
    BOOST_CHECK(vMatched.size() == merkleBlock.vMatchedTxn.size());
    for (unsigned int i = 0; i < vMatched.size(); i++)
        BOOST_CHECK(vMatched[i] == merkleBlock.vMatchedTxn[i].second);

    // Also match the 4th transaction
    filter.insert(uint256S("0xc4d094acbe0d5ddcb08764f572cbbc7d6f9a155452275335361397a3ffdeeb1e"));
    merkleBlock = CMerkleBlock(block, filter);
    BOOST_CHECK(merkleBlock.header.GetHash() == block.GetHash());

    BOOST_CHECK(merkleBlock.vMatchedTxn.size() == 2);

    BOOST_CHECK(merkleBlock.vMatchedTxn[0].second == uint256S("0xc4d094acbe0d5ddcb08764f572cbbc7d6f9a155452275335361397a3ffdeeb1e"));
    BOOST_CHECK(merkleBlock.vMatchedTxn[0].first == 3);

    BOOST_CHECK(merkleBlock.vMatchedTxn[1] == pair);

    BOOST_CHECK(merkleBlock.txn.ExtractMatches(vMatched, vIndex) == block.hashMerkleRoot);
    BOOST_CHECK(vMatched.size() == merkleBlock.vMatchedTxn.size());
    for (unsigned int i = 0; i < vMatched.size(); i++)
        BOOST_CHECK(vMatched[i] == merkleBlock.vMatchedTxn[i].second);
}

BOOST_AUTO_TEST_CASE(merkle_block_4_test_p2pubkey_only)
{
    BOOST_TEST_MESSAGE("bloom_tests, merkle_block_4_test_p2pubkey_only skipped, hard-coded to use the Bitcoin blockchain");
    // // Random real block (000000000000b731f2eef9e8c63173adfb07e41bd53eb0ef0a6b720d6cb6dea4)
    // // With 7 txes
    // CBlock block;
    // CDataStream stream(ParseHex("0100000082bb869cf3a793432a66e826e05a6fc37469f8efb7421dc880670100000000007f16c5962e8bd963659c793ce370d95f093bc7e367117b3c30c1f8fdd0d9728776381b4d4c86041b554b85290701000000010000000000000000000000000000000000000000000000000000000000000000ffffffff07044c86041b0136ffffffff0100f2052a01000000434104eaafc2314def4ca98ac970241bcab022b9c1e1f4ea423a20f134c876f2c01ec0f0dd5b2e86e7168cefe0d81113c3807420ce13ad1357231a2252247d97a46a91ac000000000100000001bcad20a6a29827d1424f08989255120bf7f3e9e3cdaaa6bb31b0737fe048724300000000494830450220356e834b046cadc0f8ebb5a8a017b02de59c86305403dad52cd77b55af062ea10221009253cd6c119d4729b77c978e1e2aa19f5ea6e0e52b3f16e32fa608cd5bab753901ffffffff02008d380c010000001976a9142b4b8072ecbba129b6453c63e129e643207249ca88ac0065cd1d000000001976a9141b8dd13b994bcfc787b32aeadf58ccb3615cbd5488ac000000000100000003fdacf9b3eb077412e7a968d2e4f11b9a9dee312d666187ed77ee7d26af16cb0b000000008c493046022100ea1608e70911ca0de5af51ba57ad23b9a51db8d28f82c53563c56a05c20f5a87022100a8bdc8b4a8acc8634c6b420410150775eb7f2474f5615f7fccd65af30f310fbf01410465fdf49e29b06b9a1582287b6279014f834edc317695d125ef623c1cc3aaece245bd69fcad7508666e9c74a49dc9056d5fc14338ef38118dc4afae5fe2c585caffffffff309e1913634ecb50f3c4f83e96e70b2df071b497b8973a3e75429df397b5af83000000004948304502202bdb79c596a9ffc24e96f4386199aba386e9bc7b6071516e2b51dda942b3a1ed022100c53a857e76b724fc14d45311eac5019650d415c3abb5428f3aae16d8e69bec2301ffffffff2089e33491695080c9edc18a428f7d834db5b6d372df13ce2b1b0e0cbcb1e6c10000000049483045022100d4ce67c5896ee251c810ac1ff9ceccd328b497c8f553ab6e08431e7d40bad6b5022033119c0c2b7d792d31f1187779c7bd95aefd93d90a715586d73801d9b47471c601ffffffff0100714460030000001976a914c7b55141d097ea5df7a0ed330cf794376e53ec8d88ac0000000001000000045bf0e214aa4069a3e792ecee1e1bf0c1d397cde8dd08138f4b72a00681743447000000008b48304502200c45de8c4f3e2c1821f2fc878cba97b1e6f8807d94930713aa1c86a67b9bf1e40221008581abfef2e30f957815fc89978423746b2086375ca8ecf359c85c2a5b7c88ad01410462bb73f76ca0994fcb8b4271e6fb7561f5c0f9ca0cf6485261c4a0dc894f4ab844c6cdfb97cd0b60ffb5018ffd6238f4d87270efb1d3ae37079b794a92d7ec95ffffffffd669f7d7958d40fc59d2253d88e0f248e29b599c80bbcec344a83dda5f9aa72c000000008a473044022078124c8beeaa825f9e0b30bff96e564dd859432f2d0cb3b72d3d5d93d38d7e930220691d233b6c0f995be5acb03d70a7f7a65b6bc9bdd426260f38a1346669507a3601410462bb73f76ca0994fcb8b4271e6fb7561f5c0f9ca0cf6485261c4a0dc894f4ab844c6cdfb97cd0b60ffb5018ffd6238f4d87270efb1d3ae37079b794a92d7ec95fffffffff878af0d93f5229a68166cf051fd372bb7a537232946e0a46f53636b4dafdaa4000000008c493046022100c717d1714551663f69c3c5759bdbb3a0fcd3fab023abc0e522fe6440de35d8290221008d9cbe25bffc44af2b18e81c58eb37293fd7fe1c2e7b46fc37ee8c96c50ab1e201410462bb73f76ca0994fcb8b4271e6fb7561f5c0f9ca0cf6485261c4a0dc894f4ab844c6cdfb97cd0b60ffb5018ffd6238f4d87270efb1d3ae37079b794a92d7ec95ffffffff27f2b668859cd7f2f894aa0fd2d9e60963bcd07c88973f425f999b8cbfd7a1e2000000008c493046022100e00847147cbf517bcc2f502f3ddc6d284358d102ed20d47a8aa788a62f0db780022100d17b2d6fa84dcaf1c95d88d7e7c30385aecf415588d749afd3ec81f6022cecd701410462bb73f76ca0994fcb8b4271e6fb7561f5c0f9ca0cf6485261c4a0dc894f4ab844c6cdfb97cd0b60ffb5018ffd6238f4d87270efb1d3ae37079b794a92d7ec95ffffffff0100c817a8040000001976a914b6efd80d99179f4f4ff6f4dd0a007d018c385d2188ac000000000100000001834537b2f1ce8ef9373a258e10545ce5a50b758df616cd4356e0032554ebd3c4000000008b483045022100e68f422dd7c34fdce11eeb4509ddae38201773dd62f284e8aa9d96f85099d0b002202243bd399ff96b649a0fad05fa759d6a882f0af8c90cf7632c2840c29070aec20141045e58067e815c2f464c6a2a15f987758374203895710c2d452442e28496ff38ba8f5fd901dc20e29e88477167fe4fc299bf818fd0d9e1632d467b2a3d9503b1aaffffffff0280d7e636030000001976a914f34c3e10eb387efe872acb614c89e78bfca7815d88ac404b4c00000000001976a914a84e272933aaf87e1715d7786c51dfaeb5b65a6f88ac00000000010000000143ac81c8e6f6ef307dfe17f3d906d999e23e0189fda838c5510d850927e03ae7000000008c4930460221009c87c344760a64cb8ae6685a3eec2c1ac1bed5b88c87de51acd0e124f266c16602210082d07c037359c3a257b5c63ebd90f5a5edf97b2ac1c434b08ca998839f346dd40141040ba7e521fa7946d12edbb1d1e95a15c34bd4398195e86433c92b431cd315f455fe30032ede69cad9d1e1ed6c3c4ec0dbfced53438c625462afb792dcb098544bffffffff0240420f00000000001976a9144676d1b820d63ec272f1900d59d43bc6463d96f888ac40420f00000000001976a914648d04341d00d7968b3405c034adc38d4d8fb9bd88ac00000000010000000248cc917501ea5c55f4a8d2009c0567c40cfe037c2e71af017d0a452ff705e3f1000000008b483045022100bf5fdc86dc5f08a5d5c8e43a8c9d5b1ed8c65562e280007b52b133021acd9acc02205e325d613e555f772802bf413d36ba807892ed1a690a77811d3033b3de226e0a01410429fa713b124484cb2bd7b5557b2c0b9df7b2b1fee61825eadc5ae6c37a9920d38bfccdc7dc3cb0c47d7b173dbc9db8d37db0a33ae487982c59c6f8606e9d1791ffffffff41ed70551dd7e841883ab8f0b16bf04176b7d1480e4f0af9f3d4c3595768d068000000008b4830450221008513ad65187b903aed1102d1d0c47688127658c51106753fed0151ce9c16b80902201432b9ebcb87bd04ceb2de66035fbbaf4bf8b00d1cfe41f1a1f7338f9ad79d210141049d4cf80125bf50be1709f718c07ad15d0fc612b7da1f5570dddc35f2a352f0f27c978b06820edca9ef982c35fda2d255afba340068c5035552368bc7200c1488ffffffff0100093d00000000001976a9148edb68822f1ad580b043c7b3df2e400f8699eb4888ac00000000"), SER_NETWORK, PROTOCOL_VERSION);
    // stream >> block;

    // CBloomFilter filter(10, 0.000001, 0, BLOOM_UPDATE_P2PUBKEY_ONLY);
    // // Match the generation pubkey
    // filter.insert(ParseHex("04eaafc2314def4ca98ac970241bcab022b9c1e1f4ea423a20f134c876f2c01ec0f0dd5b2e86e7168cefe0d81113c3807420ce13ad1357231a2252247d97a46a91"));
    // // ...and the output address of the 4th transaction
    // filter.insert(ParseHex("b6efd80d99179f4f4ff6f4dd0a007d018c385d21"));

    // CMerkleBlock merkleBlock(block, filter);
    // BOOST_CHECK(merkleBlock.header.GetHash() == block.GetHash());

    // // We should match the generation outpoint
    // BOOST_CHECK(filter.contains(COutPoint(uint256S("0x147caa76786596590baa4e98f5d9f48b86c7765e489f7a6ff3360fe5c674360b"), 0)));
    // // ... but not the 4th transaction's output (its not pay-2-pubkey)
    // BOOST_CHECK(!filter.contains(COutPoint(uint256S("0x02981fa052f0481dbc5868f4fc2166035a10f27a03cfd2de67326471df5bc041"), 0)));
}

BOOST_AUTO_TEST_CASE(merkle_block_4_test_update_none)
{
    BOOST_TEST_MESSAGE("bloom_tests, merkle_block_4_test_update_none skipped, hard-coded to use the Bitcoin blockchain");
    // // Random real block (000000000000b731f2eef9e8c63173adfb07e41bd53eb0ef0a6b720d6cb6dea4)
    // // With 7 txes
    // CBlock block;
    // CDataStream stream(ParseHex("0100000082bb869cf3a793432a66e826e05a6fc37469f8efb7421dc880670100000000007f16c5962e8bd963659c793ce370d95f093bc7e367117b3c30c1f8fdd0d9728776381b4d4c86041b554b85290701000000010000000000000000000000000000000000000000000000000000000000000000ffffffff07044c86041b0136ffffffff0100f2052a01000000434104eaafc2314def4ca98ac970241bcab022b9c1e1f4ea423a20f134c876f2c01ec0f0dd5b2e86e7168cefe0d81113c3807420ce13ad1357231a2252247d97a46a91ac000000000100000001bcad20a6a29827d1424f08989255120bf7f3e9e3cdaaa6bb31b0737fe048724300000000494830450220356e834b046cadc0f8ebb5a8a017b02de59c86305403dad52cd77b55af062ea10221009253cd6c119d4729b77c978e1e2aa19f5ea6e0e52b3f16e32fa608cd5bab753901ffffffff02008d380c010000001976a9142b4b8072ecbba129b6453c63e129e643207249ca88ac0065cd1d000000001976a9141b8dd13b994bcfc787b32aeadf58ccb3615cbd5488ac000000000100000003fdacf9b3eb077412e7a968d2e4f11b9a9dee312d666187ed77ee7d26af16cb0b000000008c493046022100ea1608e70911ca0de5af51ba57ad23b9a51db8d28f82c53563c56a05c20f5a87022100a8bdc8b4a8acc8634c6b420410150775eb7f2474f5615f7fccd65af30f310fbf01410465fdf49e29b06b9a1582287b6279014f834edc317695d125ef623c1cc3aaece245bd69fcad7508666e9c74a49dc9056d5fc14338ef38118dc4afae5fe2c585caffffffff309e1913634ecb50f3c4f83e96e70b2df071b497b8973a3e75429df397b5af83000000004948304502202bdb79c596a9ffc24e96f4386199aba386e9bc7b6071516e2b51dda942b3a1ed022100c53a857e76b724fc14d45311eac5019650d415c3abb5428f3aae16d8e69bec2301ffffffff2089e33491695080c9edc18a428f7d834db5b6d372df13ce2b1b0e0cbcb1e6c10000000049483045022100d4ce67c5896ee251c810ac1ff9ceccd328b497c8f553ab6e08431e7d40bad6b5022033119c0c2b7d792d31f1187779c7bd95aefd93d90a715586d73801d9b47471c601ffffffff0100714460030000001976a914c7b55141d097ea5df7a0ed330cf794376e53ec8d88ac0000000001000000045bf0e214aa4069a3e792ecee1e1bf0c1d397cde8dd08138f4b72a00681743447000000008b48304502200c45de8c4f3e2c1821f2fc878cba97b1e6f8807d94930713aa1c86a67b9bf1e40221008581abfef2e30f957815fc89978423746b2086375ca8ecf359c85c2a5b7c88ad01410462bb73f76ca0994fcb8b4271e6fb7561f5c0f9ca0cf6485261c4a0dc894f4ab844c6cdfb97cd0b60ffb5018ffd6238f4d87270efb1d3ae37079b794a92d7ec95ffffffffd669f7d7958d40fc59d2253d88e0f248e29b599c80bbcec344a83dda5f9aa72c000000008a473044022078124c8beeaa825f9e0b30bff96e564dd859432f2d0cb3b72d3d5d93d38d7e930220691d233b6c0f995be5acb03d70a7f7a65b6bc9bdd426260f38a1346669507a3601410462bb73f76ca0994fcb8b4271e6fb7561f5c0f9ca0cf6485261c4a0dc894f4ab844c6cdfb97cd0b60ffb5018ffd6238f4d87270efb1d3ae37079b794a92d7ec95fffffffff878af0d93f5229a68166cf051fd372bb7a537232946e0a46f53636b4dafdaa4000000008c493046022100c717d1714551663f69c3c5759bdbb3a0fcd3fab023abc0e522fe6440de35d8290221008d9cbe25bffc44af2b18e81c58eb37293fd7fe1c2e7b46fc37ee8c96c50ab1e201410462bb73f76ca0994fcb8b4271e6fb7561f5c0f9ca0cf6485261c4a0dc894f4ab844c6cdfb97cd0b60ffb5018ffd6238f4d87270efb1d3ae37079b794a92d7ec95ffffffff27f2b668859cd7f2f894aa0fd2d9e60963bcd07c88973f425f999b8cbfd7a1e2000000008c493046022100e00847147cbf517bcc2f502f3ddc6d284358d102ed20d47a8aa788a62f0db780022100d17b2d6fa84dcaf1c95d88d7e7c30385aecf415588d749afd3ec81f6022cecd701410462bb73f76ca0994fcb8b4271e6fb7561f5c0f9ca0cf6485261c4a0dc894f4ab844c6cdfb97cd0b60ffb5018ffd6238f4d87270efb1d3ae37079b794a92d7ec95ffffffff0100c817a8040000001976a914b6efd80d99179f4f4ff6f4dd0a007d018c385d2188ac000000000100000001834537b2f1ce8ef9373a258e10545ce5a50b758df616cd4356e0032554ebd3c4000000008b483045022100e68f422dd7c34fdce11eeb4509ddae38201773dd62f284e8aa9d96f85099d0b002202243bd399ff96b649a0fad05fa759d6a882f0af8c90cf7632c2840c29070aec20141045e58067e815c2f464c6a2a15f987758374203895710c2d452442e28496ff38ba8f5fd901dc20e29e88477167fe4fc299bf818fd0d9e1632d467b2a3d9503b1aaffffffff0280d7e636030000001976a914f34c3e10eb387efe872acb614c89e78bfca7815d88ac404b4c00000000001976a914a84e272933aaf87e1715d7786c51dfaeb5b65a6f88ac00000000010000000143ac81c8e6f6ef307dfe17f3d906d999e23e0189fda838c5510d850927e03ae7000000008c4930460221009c87c344760a64cb8ae6685a3eec2c1ac1bed5b88c87de51acd0e124f266c16602210082d07c037359c3a257b5c63ebd90f5a5edf97b2ac1c434b08ca998839f346dd40141040ba7e521fa7946d12edbb1d1e95a15c34bd4398195e86433c92b431cd315f455fe30032ede69cad9d1e1ed6c3c4ec0dbfced53438c625462afb792dcb098544bffffffff0240420f00000000001976a9144676d1b820d63ec272f1900d59d43bc6463d96f888ac40420f00000000001976a914648d04341d00d7968b3405c034adc38d4d8fb9bd88ac00000000010000000248cc917501ea5c55f4a8d2009c0567c40cfe037c2e71af017d0a452ff705e3f1000000008b483045022100bf5fdc86dc5f08a5d5c8e43a8c9d5b1ed8c65562e280007b52b133021acd9acc02205e325d613e555f772802bf413d36ba807892ed1a690a77811d3033b3de226e0a01410429fa713b124484cb2bd7b5557b2c0b9df7b2b1fee61825eadc5ae6c37a9920d38bfccdc7dc3cb0c47d7b173dbc9db8d37db0a33ae487982c59c6f8606e9d1791ffffffff41ed70551dd7e841883ab8f0b16bf04176b7d1480e4f0af9f3d4c3595768d068000000008b4830450221008513ad65187b903aed1102d1d0c47688127658c51106753fed0151ce9c16b80902201432b9ebcb87bd04ceb2de66035fbbaf4bf8b00d1cfe41f1a1f7338f9ad79d210141049d4cf80125bf50be1709f718c07ad15d0fc612b7da1f5570dddc35f2a352f0f27c978b06820edca9ef982c35fda2d255afba340068c5035552368bc7200c1488ffffffff0100093d00000000001976a9148edb68822f1ad580b043c7b3df2e400f8699eb4888ac00000000"), SER_NETWORK, PROTOCOL_VERSION);
    // stream >> block;

    // CBloomFilter filter(10, 0.000001, 0, BLOOM_UPDATE_NONE);
    // // Match the generation pubkey
    // filter.insert(ParseHex("04eaafc2314def4ca98ac970241bcab022b9c1e1f4ea423a20f134c876f2c01ec0f0dd5b2e86e7168cefe0d81113c3807420ce13ad1357231a2252247d97a46a91"));
    // // ...and the output address of the 4th transaction
    // filter.insert(ParseHex("b6efd80d99179f4f4ff6f4dd0a007d018c385d21"));

    // CMerkleBlock merkleBlock(block, filter);
    // BOOST_CHECK(merkleBlock.header.GetHash() == block.GetHash());

    // // We shouldn't match any outpoints (UPDATE_NONE)
    // BOOST_CHECK(!filter.contains(COutPoint(uint256S("0x147caa76786596590baa4e98f5d9f48b86c7765e489f7a6ff3360fe5c674360b"), 0)));
    // BOOST_CHECK(!filter.contains(COutPoint(uint256S("0x02981fa052f0481dbc5868f4fc2166035a10f27a03cfd2de67326471df5bc041"), 0)));
}

static std::vector<unsigned char> RandomData()
{
    uint256 r = InsecureRand256();
    return std::vector<unsigned char>(r.begin(), r.end());
}

BOOST_AUTO_TEST_CASE(rolling_bloom)
{
    // last-100-entry, 1% false positive:
    CRollingBloomFilter rb1(100, 0.01);

    // Overfill:
    static const int DATASIZE=399;
    std::vector<unsigned char> data[DATASIZE];
    for (int i = 0; i < DATASIZE; i++) {
        data[i] = RandomData();
        rb1.insert(data[i]);
    }
    // Last 100 guaranteed to be remembered:
    for (int i = 299; i < DATASIZE; i++) {
        BOOST_CHECK(rb1.contains(data[i]));
    }

    // false positive rate is 1%, so we should get about 100 hits if
    // testing 10,000 random keys. We get worst-case false positive
    // behavior when the filter is as full as possible, which is
    // when we've inserted one minus an integer multiple of nElement*2.
    unsigned int nHits = 0;
    for (int i = 0; i < 10000; i++) {
        if (rb1.contains(RandomData()))
            ++nHits;
    }
    // Run test_bitcoin with --log_level=message to see BOOST_TEST_MESSAGEs:
    BOOST_TEST_MESSAGE("RollingBloomFilter got " << nHits << " false positives (~100 expected)");

    // Insanely unlikely to get a fp count outside this range:
    BOOST_CHECK(nHits > 25);
    BOOST_CHECK(nHits < 175);

    BOOST_CHECK(rb1.contains(data[DATASIZE-1]));
    rb1.reset();
    BOOST_CHECK(!rb1.contains(data[DATASIZE-1]));

    // Now roll through data, make sure last 100 entries
    // are always remembered:
    for (int i = 0; i < DATASIZE; i++) {
        if (i >= 100)
            BOOST_CHECK(rb1.contains(data[i-100]));
        rb1.insert(data[i]);
        BOOST_CHECK(rb1.contains(data[i]));
    }

    // Insert 999 more random entries:
    for (int i = 0; i < 999; i++) {
        std::vector<unsigned char> d = RandomData();
        rb1.insert(d);
        BOOST_CHECK(rb1.contains(d));
    }
    // Sanity check to make sure the filter isn't just filling up:
    nHits = 0;
    for (int i = 0; i < DATASIZE; i++) {
        if (rb1.contains(data[i]))
            ++nHits;
    }
    // Expect about 5 false positives, more than 100 means
    // something is definitely broken.
    BOOST_TEST_MESSAGE("RollingBloomFilter got " << nHits << " false positives (~5 expected)");
    BOOST_CHECK(nHits < 100);

    // last-1000-entry, 0.01% false positive:
    CRollingBloomFilter rb2(1000, 0.001);
    for (int i = 0; i < DATASIZE; i++) {
        rb2.insert(data[i]);
    }
    // ... room for all of them:
    for (int i = 0; i < DATASIZE; i++) {
        BOOST_CHECK(rb2.contains(data[i]));
    }
}

BOOST_AUTO_TEST_SUITE_END()
