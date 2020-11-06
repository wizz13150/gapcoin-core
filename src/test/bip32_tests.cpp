// Copyright (c) 2013-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/test/unit_test.hpp>

#include <base58.h>
#include <key.h>
#include <uint256.h>
#include <util.h>
#include <utilstrencodings.h>
#include <test/test_bitcoin.h>

#include <string>
#include <vector>

struct TestDerivation {
    std::string pub;
    std::string prv;
    unsigned int nChild;
};

struct TestVector {
    std::string strHexMaster;
    std::vector<TestDerivation> vDerive;

    explicit TestVector(std::string strHexMasterIn) : strHexMaster(strHexMasterIn) {}

    TestVector& operator()(std::string pub, std::string prv, unsigned int nChild) {
        vDerive.push_back(TestDerivation());
        TestDerivation &der = vDerive.back();
        der.pub = pub;
        der.prv = prv;
        der.nChild = nChild;
        return *this;
    }
};

TestVector test1 =
  TestVector("000102030405060708090a0b0c0d0e0f")
    ("gpxp1UoyCz5qoNwvRbCpPv826aRZUZzM7CYKmVLCWMw4n39owHLpqUgihxyAuh9Av8a1gaHPx1kR2qEApJUnU2DJ1qSSsAoYVLE6UraHWh7TZw9",
     "gpxv2JGkH1LYqUHceqGRS5cfjxPWCLYzsk8eWshnstVvwLKAGCAxMKvwH9ZLZm7bJu9DtKvy3iaoobwdGwZqPc5ZKimt1gqi2KmGnhLHVtBwvU5",
     0x80000000)
    ("gpxp1X5PChPJte4uaCLXbfzeycXfoPYhVH2LT6m6UTKBkEM6JZe1FiDoq7KagxhsfSUYgnZxpFHqmUJFnhSvR3wg5iuB4oboQe8JznhqpNsERN7",
     "gpxv2LYAGie1vjQboSQ8dqVJczVcXA7MFpcfCV8gqyt3uXWSdUU8mZU2QHukM2TvicXBbNpfR1ugBxnVzhbZ3CMk7nrQ5ZWrGHUAVhy8w4EBRcM",
     1)
    ("gpxp1ZFWQUwCHLwyQeNyKvtXaMRKrmXoVJkdPRRw4j5zgs9JY798yWe7ngDp73AT5PBteWC1bfjwt4njP1BjtP7ZmNj7N1SetH8adXEd7HWwRGr",
     "gpxv2NiHUWBuKSHfdtSaN6PBDjPGaY6TFrLx8ooXSFerrAJes1yGVMtLMroym5ezKYJE3TaMDqW7Gfjv8e7nAY9itxLqbLUaL3F2AWHWTzLznxw",
     0x80000002)
    ("gpxp1brnT1m3zDoPGpB4aXHtwMTU5vQKkrLcixfcK4P9rjWjwxLFnGUYh7mjf4MHXFdD12HkD3iZG1MhqLL2bUf6ppswwuQnyECGMD9mCXukiXX",
     "gpxv2RKZX31m2K95W4EfcgnYajRQogxyXPvwUM3Cgax222g6GsAPJ7imGJMuK7scHmoiYvbjpJT2h5ZpMqrXkZ4v23nVwx38UyUxmn2j6dyfJ8G",
     2)
    ("gpxp1e6BHSt1AhUNMHenGnVyprcnAjh4PSepASYua1udBB5MK3xcr3AuthEfLPLfw3mwULQvM4CWJkFpPkg2gn6JYRMCrgxyoR66Mu4cM8bRvZe",
     "gpxv2TYxMU8iCnp4aXiPJwzdUEaitWFi9zF8upvVwYUVLUEhdxnkMtR8TsppzSH4kSD9cijSSDgbbu1yq2kywycGDSNYMC8E34JfvkVtc5iHWVB",
     1000000000)
    ("gpxp1fowm8VFHprZsdSEPL4i8ZcmxyGNCwTqxgwTpJHmPFTr2BN1NTRa9oPiyDFcnJrex2hTrQ8JqwxQzB53ndb9PgWKjEP4btfjArgXbKjm45t",
     "gpxv2VGiq9jxKvCG6sVqRVZMmwaigjq1yV4Ai5K4BprdYYdCM6C8tJfniyytdJ4GfMLEE88hDafYSwZ6qCmMV3czLQ25ggtT9y4PD64FMDjBuWo",
     0);

TestVector test2 =
  TestVector("fffcf9f6f3f0edeae7e4e1dedbd8d5d2cfccc9c6c3c0bdbab7b4b1aeaba8a5a29f9c999693908d8a8784817e7b7875726f6c696663605d5a5754514e4b484542")
    ("gpxp1UoyCz5qoNwvRCi9W5CJYn9Kim8mnpo9T57voJftkRGgEGN21dTf8s8mFoE9dNvyb8XBrgQaRujRY6D1bD8T9kSPj711KpikoEFPnfLkHyU",
     "gpxv2JGkH1LYqUHceSmkYEgxCA7GSXhRZNPUCTVXAqEkuiS2ZBC9XUhsi3ivupteVQhK8NgPo13G5hypNUHwS2UFw66eU6uxAfg1MwxWrosNDSj",
     0)
    ("gpxp1Y5iUhmoHPmzwUJSdraBcaYr5it2VoHS1EsHy7hLpq8dvEozatojHhi1qGC3RTwQoT1MiuQKpzaPhFdhUMpsVM6g19jgEAs8viN4DqPYKLf",
     "gpxv2MYVYj2WKV7hAiN3g24qFxWnoVSgGLskkdEtLeGCz8HzF9e86k3wrtJBVLAT8UY9Y2vDh1SzWsPwiyvkzJyJsKG3UHL1Z9qkVMs5EACH4Ap",
     0xFFFFFFFF)
    ("gpxp1ZEmjJoKfHx77LWd4JZq5vEQUU6w3HT86PwptWc7UZu2NY3TCnE3VmVcSeSABCYWaxvuNkFw8CzzMvW3FGJzDVZJYEMjau89o6df4evwuhG",
     "gpxv2NhYoL42hPHoLaaE6U4UjJCMCEfaoq3SqnKRG3Ayds4NhSsaidUG4x5n6geRrjGdSGF7U2edZsu4ADfXigaSSchbu1ZMKfa1H9CbzaP1C4S",
     1)
    ("gpxp1c3k9KnFpwFJCxBm7xiQ3sAdozyS9UNPQqSgjaGSnTFU3MucWHkkPfszHSdHy5EoB9PTDPMAmLJpSim4AAjwUq2D8WZaoSZMisJcK7MgyZf",
     "gpxv2RWXDM2xs2azSCFNA8D3hF8aXmY5v1xiADpH76qJwkQpNGjk28zxxrU9wUr3tSUKkDbpDKj1yRaGUvSgma6pT6Wu2n6AAmYPkwdLUDrGkSh",
     0xFFFFFFFE)
    ("gpxp1dDn4HicSLD1VuRMrSXwRNrpijSoCSztSYAaGHxrdDWiViEYbxgkvCF5v6koASmF8b361tQsowdg6MvSr3rTCBifQQt5Mjemb6PBr6mMPHB",
     "gpxv2SgZ8JyKURYhj9Uxtc2b4kpmSW1SxzbDBvYAdpXinWg4pd4g7ovyVNqFaBaSWrrcHntWX9W1Mo9rJ3Y4HX1K8wC4ridUJya26XdrXzESxHE",
     2)
    ("gpxp1eap1iw7xWXFFAcFaWfdQ8Eo7nyuSmuy6hnTg5KMG1NJZua7mbcX6mC5zVq5J27weeFKHovHheefanywg9YMWSaF3aiAJ6wWGPaNWgdNMKQ",
     "gpxv2U3b5kBpzbrwUQfrcgAH3WCjqZYZDKWHr6A43btDRJXetpQFHSrjfwnFebFSfswgL6kEou8potzYLbRa1qNYBuvnfa8yNPhjntyQca5icQ9",
     0);

TestVector test3 =
  TestVector("4b381541583be4423346c643850da4b320e46a87ae3d2a4e6da11eba819cd4acba45d239319ac14f863b8d5ab5a0d0c64d2e8a1e7d1457df2e5a3c51c73235be")
    ("gpxp1UoyCz5qoNwvQGAK2Th6kmQMbQTsYgxRAQtVCbhT27MUCWbHfmhd68NHjnm8SyWcmDH3ggHapaZGyPqvRvAta66XrLuE4PAbawm6K9oJ71g",
     "gpxv2JGkH1LYqUHcdWDv4dBkQ9NJKB2XKEYjuoG5a8GKBQWpXRRRBcwqfJxTPpcpSRveD5M9MvjMz8iXj7uFfTcgiZbtzPUbN71yefcsWjch9b5",
      0x80000000)
    ("gpxp1XBAwwuyUkKGboFiFc2bp2A2Hm5FhYNKZu8S1qPHxRGqTXikFGUVAbA13WbeBL7MxamtTe3KfXkmbrnGNA759LNUXSr7cEfQ587zyaY3mw5",
     "gpxv2Ldx1yAgWqexq3KKHmXFTQ7y1XduU5xeKHW2PMxA7iSBnSYsm7ihjmkAhayAnPFZfDzZNsXEdsLzNfYQEgCSUKoXRXPe7GHnmgaXUC3ghDW",
      0);

void RunTest(const TestVector &test) {
    std::vector<unsigned char> seed = ParseHex(test.strHexMaster);
    CExtKey key;
    CExtPubKey pubkey;
    key.SetMaster(seed.data(), seed.size());
    pubkey = key.Neuter();
    for (const TestDerivation &derive : test.vDerive) {
        unsigned char data[74];
        key.Encode(data);
        pubkey.Encode(data);

        // Test private key
        CBitcoinExtKey b58key; b58key.SetKey(key);
        BOOST_CHECK(b58key.ToString() == derive.prv);

        CBitcoinExtKey b58keyDecodeCheck(derive.prv);
        CExtKey checkKey = b58keyDecodeCheck.GetKey();
        assert(checkKey == key); //ensure a base58 decoded key also matches

        // Test public key
        CBitcoinExtPubKey b58pubkey; b58pubkey.SetKey(pubkey);
        BOOST_CHECK(b58pubkey.ToString() == derive.pub);

        CBitcoinExtPubKey b58PubkeyDecodeCheck(derive.pub);
        CExtPubKey checkPubKey = b58PubkeyDecodeCheck.GetKey();
        assert(checkPubKey == pubkey); //ensure a base58 decoded pubkey also matches

        // Derive new keys
        CExtKey keyNew;
        BOOST_CHECK(key.Derive(keyNew, derive.nChild));
        CExtPubKey pubkeyNew = keyNew.Neuter();
        if (!(derive.nChild & 0x80000000)) {
            // Compare with public derivation
            CExtPubKey pubkeyNew2;
            BOOST_CHECK(pubkey.Derive(pubkeyNew2, derive.nChild));
            BOOST_CHECK(pubkeyNew == pubkeyNew2);
        }
        key = keyNew;
        pubkey = pubkeyNew;

        CDataStream ssPub(SER_DISK, CLIENT_VERSION);
        ssPub << pubkeyNew;
        BOOST_CHECK(ssPub.size() == 75);

        CDataStream ssPriv(SER_DISK, CLIENT_VERSION);
        ssPriv << keyNew;
        BOOST_CHECK(ssPriv.size() == 75);

        CExtPubKey pubCheck;
        CExtKey privCheck;
        ssPub >> pubCheck;
        ssPriv >> privCheck;

        BOOST_CHECK(pubCheck == pubkeyNew);
        BOOST_CHECK(privCheck == keyNew);
    }
}

BOOST_FIXTURE_TEST_SUITE(bip32_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(bip32_test1) {
    RunTest(test1);
}

BOOST_AUTO_TEST_CASE(bip32_test2) {
    RunTest(test2);
}

BOOST_AUTO_TEST_CASE(bip32_test3) {
    RunTest(test3);
}

BOOST_AUTO_TEST_SUITE_END()
