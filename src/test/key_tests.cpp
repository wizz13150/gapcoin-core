// Copyright (c) 2012-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <key.h>

#include <base58.h>
#include <script/script.h>
#include <uint256.h>
#include <util.h>
#include <utilstrencodings.h>
#include <wallet/wallet.h>

#include <test/test_bitcoin.h>

#include <string>
#include <vector>
#include <iostream>
#include <sstream>

#include <boost/test/unit_test.hpp>

static const std::string addr1 = "GXqRgshX1i3Ddc7zcAixYqgNmcyhyhQeLW";
static const std::string strSecret1 = "4G15gmx24XgoEdAsmJzkqYmBWZZjheDeyXStwQULAmhVUYCTV8J";

static const std::string addr2 = "GdiZz2QyPNbC3SW3x9LiMSKJJTDSPDCAFa";
static const std::string strSecret2 = "4HJjrejHVMtXdJB2k13gdiirPyLcxYSjJ1FLjJqZUBCDsqpApbN";

static const std::string addr1C = "GUxGnHbBpEYxsM8RiD9y7MbdkDx9tZa1Tr";
static const std::string strSecret1C = "FP6ByGTZchgrCdMYdoTMifLbFwitXY7KwiGgtzQUtt3L3Z7cP3Y4";

static const std::string addr2C = "GYNHehukGgtTaLEJ7YVb8Q1fff5HKbWDTY";
static const std::string strSecret2C = "FRFWcJEan8wYVMnKMigbTvpMZpTDQybeXcBXrnwu1EETpb8QsNVU";

static const std::string strAddressBad = "ZzzzLc3sNHZxwj4Zk6fB38tEmBryq2cBiF";


BOOST_FIXTURE_TEST_SUITE(key_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(key_test1)
{
    CBitcoinSecret bsecret1, bsecret2, bsecret1C, bsecret2C, baddress1;
    BOOST_CHECK( bsecret1.SetString (strSecret1));
    BOOST_CHECK( bsecret2.SetString (strSecret2));
    BOOST_CHECK( bsecret1C.SetString(strSecret1C));
    BOOST_CHECK( bsecret2C.SetString(strSecret2C));
    BOOST_CHECK(!baddress1.SetString(strAddressBad));

    CKey key1  = bsecret1.GetKey();
    BOOST_CHECK(key1.IsCompressed() == false);
    CKey key2  = bsecret2.GetKey();
    BOOST_CHECK(key2.IsCompressed() == false);

    CKey key1C = bsecret1C.GetKey();
    BOOST_CHECK(key1C.IsCompressed() == true);
    CKey key2C = bsecret2C.GetKey();
    BOOST_CHECK(key2C.IsCompressed() == true);

    CPubKey pubkey1  = key1. GetPubKey();
    CPubKey pubkey2  = key2. GetPubKey();

    CPubKey pubkey1C = key1C.GetPubKey();
    CPubKey pubkey2C = key2C.GetPubKey();

    BOOST_CHECK(key1.VerifyPubKey(pubkey1));
    BOOST_CHECK(!key1.VerifyPubKey(pubkey1C));
    BOOST_CHECK(!key1.VerifyPubKey(pubkey2));
    BOOST_CHECK(!key1.VerifyPubKey(pubkey2C));

    BOOST_CHECK(!key1C.VerifyPubKey(pubkey1));
    BOOST_CHECK(key1C.VerifyPubKey(pubkey1C));
    BOOST_CHECK(!key1C.VerifyPubKey(pubkey2));
    BOOST_CHECK(!key1C.VerifyPubKey(pubkey2C));

    BOOST_CHECK(!key2.VerifyPubKey(pubkey1));
    BOOST_CHECK(!key2.VerifyPubKey(pubkey1C));
    BOOST_CHECK(key2.VerifyPubKey(pubkey2));
    BOOST_CHECK(!key2.VerifyPubKey(pubkey2C));

    BOOST_CHECK(!key2C.VerifyPubKey(pubkey1));
    BOOST_CHECK(!key2C.VerifyPubKey(pubkey1C));
    BOOST_CHECK(!key2C.VerifyPubKey(pubkey2));
    BOOST_CHECK(key2C.VerifyPubKey(pubkey2C));

    BOOST_CHECK(DecodeDestination(addr1)  == CTxDestination(pubkey1.GetID()));
    BOOST_CHECK(DecodeDestination(addr2)  == CTxDestination(pubkey2.GetID()));

    // BOOST_CHECK(DecodeDestination(addr1C) == CTxDestination(pubkey1C.GetID()));
    // int i = 0;
    // for (const auto& dest : GetAllDestinationsForKey(pubkey1C)) {
    //     std::cout << "pubkey1C address #" << i << " = " << EncodeDestination(dest) << std::endl;
    //     i++;
    // }
    std::vector<CTxDestination> pubkey1Cdests = GetAllDestinationsForKey(pubkey1C);
    BOOST_CHECK(DecodeDestination(addr1C) == pubkey1Cdests[0]);

    std::vector<CTxDestination> pubkey2Cdests = GetAllDestinationsForKey(pubkey2C);
    BOOST_CHECK(DecodeDestination(addr2C) == pubkey2Cdests[0]);

    for (int n=0; n<16; n++)
    {
        std::string strMsg = strprintf("Very secret message %i: 11", n);
        uint256 hashMsg = Hash(strMsg.begin(), strMsg.end());

        // normal signatures

        std::vector<unsigned char> sign1, sign2, sign1C, sign2C;

        BOOST_CHECK(key1.Sign (hashMsg, sign1));
        BOOST_CHECK(key2.Sign (hashMsg, sign2));
        BOOST_CHECK(key1C.Sign(hashMsg, sign1C));
        BOOST_CHECK(key2C.Sign(hashMsg, sign2C));

        BOOST_CHECK( pubkey1.Verify(hashMsg, sign1));
        BOOST_CHECK(!pubkey1.Verify(hashMsg, sign2));
        // BOOST_CHECK( pubkey1.Verify(hashMsg, sign1C));
        BOOST_CHECK(!pubkey1.Verify(hashMsg, sign2C));

        BOOST_CHECK(!pubkey2.Verify(hashMsg, sign1));
        BOOST_CHECK( pubkey2.Verify(hashMsg, sign2));
        BOOST_CHECK(!pubkey2.Verify(hashMsg, sign1C));
        // BOOST_CHECK( pubkey2.Verify(hashMsg, sign2C));

        // BOOST_CHECK( pubkey1C.Verify(hashMsg, sign1));
        BOOST_CHECK(!pubkey1C.Verify(hashMsg, sign2));
        BOOST_CHECK( pubkey1C.Verify(hashMsg, sign1C));
        BOOST_CHECK(!pubkey1C.Verify(hashMsg, sign2C));

        BOOST_CHECK(!pubkey2C.Verify(hashMsg, sign1));
        // BOOST_CHECK( pubkey2C.Verify(hashMsg, sign2));
        BOOST_CHECK(!pubkey2C.Verify(hashMsg, sign1C));
        BOOST_CHECK( pubkey2C.Verify(hashMsg, sign2C));

        // compact signatures (with key recovery)

        std::vector<unsigned char> csign1, csign2, csign1C, csign2C;

        BOOST_CHECK(key1.SignCompact (hashMsg, csign1));
        BOOST_CHECK(key2.SignCompact (hashMsg, csign2));
        BOOST_CHECK(key1C.SignCompact(hashMsg, csign1C));
        BOOST_CHECK(key2C.SignCompact(hashMsg, csign2C));

        CPubKey rkey1, rkey2, rkey1C, rkey2C;

        BOOST_CHECK(rkey1.RecoverCompact (hashMsg, csign1));
        BOOST_CHECK(rkey2.RecoverCompact (hashMsg, csign2));
        BOOST_CHECK(rkey1C.RecoverCompact(hashMsg, csign1C));
        BOOST_CHECK(rkey2C.RecoverCompact(hashMsg, csign2C));

        BOOST_CHECK(rkey1  == pubkey1);
        BOOST_CHECK(rkey2  == pubkey2);
        BOOST_CHECK(rkey1C == pubkey1C);
        BOOST_CHECK(rkey2C == pubkey2C);
    }

    // test deterministic signing

    std::vector<unsigned char> detsig, detsigc;
    std::string strMsg = "Very deterministic message";
    uint256 hashMsg = Hash(strMsg.begin(), strMsg.end());
    BOOST_CHECK(key1.Sign(hashMsg, detsig));
    BOOST_CHECK(key1C.Sign(hashMsg, detsigc));

    // std::cout << "sed -i 's/key1_" << "detsighex/";
    // for (auto& c : detsig)
    //     printf("%02hhx", c);
    // std::cout << "/' src/test/key_tests.cpp" << std::endl;
    BOOST_CHECK(detsig == ParseHex("3045022100d23cb82bd8cfe97a72193ce32531c1dfb2417c8876d01ca7666ad690721f70a302205a532c395e7fe2191d8950aa9f06dd895299fc7b2088bafdeb6942165dee3afd"));

    BOOST_CHECK(key2.Sign(hashMsg, detsig));
    BOOST_CHECK(key2C.Sign(hashMsg, detsigc));

    // std::cout << "sed -i 's/key2_" << "detsighex/";
    // for (auto& c : detsig)
    //     printf("%02hhx", c);
    // std::cout << "/' src/test/key_tests.cpp" << std::endl;
    BOOST_CHECK(detsig == ParseHex("3045022100953c660adf22344f8dd503f188f07ba38c7d0cf72d0ea5cd61643ce238706bab02204ed2d4d3f94035c0b7ec823fbf328bce712890e2bfaaa4c7eda49bd672867f04"));

    BOOST_CHECK(key1.SignCompact(hashMsg, detsig));
    BOOST_CHECK(key1C.SignCompact(hashMsg, detsigc));

    // std::cout << "sed -i 's/key1_" << "compact/";
    // for (auto& c : detsig)
    //     printf("%02hhx", c);
    // std::cout << "/' src/test/key_tests.cpp" << std::endl;
    BOOST_CHECK(detsig == ParseHex("1cd23cb82bd8cfe97a72193ce32531c1dfb2417c8876d01ca7666ad690721f70a35a532c395e7fe2191d8950aa9f06dd895299fc7b2088bafdeb6942165dee3afd"));

    // std::cout << "sed -i 's/key1C_" << "compact/";
    // for (auto& c : detsigc)
    //     printf("%02hhx", c);
    // std::cout << "/' src/test/key_tests.cpp" << std::endl;
    BOOST_CHECK(detsigc == ParseHex("1f52cd334136da8942ed19b8c3c9e9c7d56611ff202e7e146c609a389ca25d8e897b398afc2bd64cf973929b7ffabc288a519a36fec3dd723e1f24f98352f0bbe2"));

    BOOST_CHECK(key2.SignCompact(hashMsg, detsig));
    BOOST_CHECK(key2C.SignCompact(hashMsg, detsigc));

    // std::cout << "sed -i 's/key2_" << "compact/";
    // for (auto& c : detsig)
    //     printf("%02hhx", c);
    // std::cout << "/' src/test/key_tests.cpp" << std::endl;
    BOOST_CHECK(detsig == ParseHex("1c953c660adf22344f8dd503f188f07ba38c7d0cf72d0ea5cd61643ce238706bab4ed2d4d3f94035c0b7ec823fbf328bce712890e2bfaaa4c7eda49bd672867f04"));

    // std::cout << "sed -i 's/key2C_" << "compact/";
    // for (auto& c : detsigc)
    //     printf("%02hhx", c);
    // std::cout << "/' src/test/key_tests.cpp" << std::endl;
    BOOST_CHECK(detsigc == ParseHex("1f4775e210d9998d2be1001e41a8de4452b876d59b018bd3ce36ed19c987b8dd5d65d1f23b5c8baa9e8a78ab700ea4acf72e14dcc94a24185ee5c35ba200300533"));
}

BOOST_AUTO_TEST_SUITE_END()

