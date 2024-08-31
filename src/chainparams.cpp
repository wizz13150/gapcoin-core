// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2017 The Bitcoin Core developers
// Copyright (c) 2020 The Gapcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <arith_uint256.h>
#include <chainparams.h>
#include <chainparamsbase.h>
#include <chainparamsseeds.h>
#include <consensus/merkle.h>
#include <tinyformat.h>
#include <util.h>
#include <utilstrencodings.h>
#include <pow.h>
#include <arith_uint256.h>

#include <assert.h>
#include <memory>

static const char* pszTimestamp = "The Times 15/Oct/2014 US data sends global stocks into tail-spin";
static const CScript genesisOutputScript = CScript() << ParseHex("044588d54931b7de2f9faaa5a3c1fde654114ae51273754e1f3f9720127f8977af6bfaa1f33e22e80e4b83f5269921501b411d254929faf1b10d2174ded28ac59d") << OP_CHECKSIG;

static CBlock CreateMainnetGenesisBlock(uint32_t nTime, uint32_t nNonce, int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime       = nTime;
    genesis.nNonce      = nNonce;
    genesis.nVersion    = nVersion;
    genesis.nShift      = 20;
    genesis.nDifficulty = PoWUtils::min_difficulty;
    uint8_t nAdd[]      = { 233, 156, 15 };
    genesis.nAdd.assign(nAdd, nAdd + sizeof(nAdd) / sizeof(uint8_t));
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

static CBlock CreateTestnetGenesisBlock(uint32_t nTime, uint32_t nNonce, int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime       = nTime;
    genesis.nNonce      = nNonce;
    genesis.nVersion    = nVersion;
    genesis.nShift      = 20;
    genesis.nDifficulty = PoWUtils::min_test_difficulty;
    uint8_t nAdd[]      = { 25, 1 };
    genesis.nAdd.assign(nAdd, nAdd + sizeof(nAdd) / sizeof(uint8_t));
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database.
 *
*/

void CChainParams::UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
{
    consensus.vDeployments[d].nStartTime = nStartTime;
    consensus.vDeployments[d].nTimeout = nTimeout;
}

/**
 * Main network
 */
/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */

class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = "main";
        consensus.nSubsidyHalvingInterval = 420000;
        // Recognize BIP16 pay to hash after block #
        consensus.BIP16Height = 0;
        // Move to version 2 blocks, do not recognize v1 blocks after this height + hash
        consensus.BIP34Height = 5555555; // (disable BIP34 because legacy chain uses V1 coinbase)
        consensus.BIP34Hash = uint256();
        consensus.BIP65Height = 9999999; // Implements OP_CHECKLOCKTIMEVERIFY opcode which locks tx's until future time.
        consensus.BIP66Height = 9999999; // Enforces DER encoding from 363725 block onwards. https://en.wikipedia.org/wiki/X.690#DER_encoding
        consensus.nPowTargetTimespan = 150; // 
        consensus.nPowTargetSpacing = 150; // 
        consensus.nCoinbaseMaturity = 100;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;

        // rule changes require 96% agreement, measured across 4 retargeting periods.
        // 95% consensus is required to accept protocol rule changes
        consensus.nRuleChangeActivationThreshold = 1916; // 95% of 2016
        // The 95% consensus must be obtained in 2016 blocks time (window = one retargeting period).
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing


        // the minimum and maximum times that three softforks can activate and how they are to be signaled using version bits.

        // Satoshi's playground
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        // From the documentation : "Block height at which CSV (BIP68, BIP112 and BIP113) becomes active."
        // These 3 BIP's collectively knowh as CheckSequenceVerify = https://en.bitcoin.it/wiki/CheckSequenceVerify
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1462060800; // May 1st, 2016
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1493596800; // May 1st, 2017

        // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 0; // November 15th, 2016.
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 0; // November 15th, 2017.

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("000000000000000000000000000000000000000000000000001833f01e293300");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("98ec0eff51b18a0e32fbd79c68d70259aa568aaae1c470aaa2ef377361a6d8f3"); //0

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0xd1;
        pchMessageStart[1] = 0xdf;
        pchMessageStart[2] = 0xe6;
        pchMessageStart[3] = 0xf9;
        
        nDefaultPort = 31469;
        nPruneAfterHeight = 1000000;

        genesis = CreateMainnetGenesisBlock(GENESIS_TIME, MAINNETNONCE, 1, 0 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();

        LogPrint(BCLog::DEVEL, "Mainnet: %s.\n", genesis.ToString());

        assert(consensus.hashGenesisBlock == uint256S(MAINNETGENESISHASH));
        assert(genesis.hashMerkleRoot == uint256S(MERKLETREEROOTHASH));

        // nodes with support for servicebits filtering should be at the top
        // vSeeds.emplace_back("minkiz.co");
        vSeeds.emplace_back("144.76.118.44");

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,38);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,5);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,97);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x03, 0x3C, 0x0D, 0xF8}; // gpxp
        base58Prefixes[EXT_SECRET_KEY] = {0x03, 0x3C, 0x0E, 0x22}; // gpxv
        base58Prefixes[EXT_COIN_TYPE]  = {0x80, 0x00, 0x00, 0x0f}; // BIP44 coin type is '15'

        bech32_hrp = "gp";

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        // Whether to check mempool and block index consistency by default
        fDefaultConsistencyChecks = false;
        // Filter out transactions that don't comply with the Bitcoin Core standard
        fRequireStandard = true;

        fMineBlocksOnDemand = false;
        fMiningRequiresPeers = false;

        // Blockchain is assumed valid before each of these.
        /**
         * What makes a good checkpoint block?
         * + Is surrounded by blocks with reasonable timestamps
         *   (no blocks before with a timestamp after, none after with
         *    timestamp before)
         * + Contains no strange transactions
         */
        checkpointData = {
            {
                { 0,       uint256S(MAINNETGENESISHASH)},
                { 14738,   uint256S("0xce083f91261ea327c2e3b855ba3df7bb696010fec985ab8b5c699b5df3edcd27")},
                { 28592,   uint256S("0xee7852b7dfa68f86dd272f25899bea9cc9ea993c33a621d78b49ec96defc50c7")},
                { 39229,   uint256S("0xb94e3f013746985b122fc121d7cbe707fa3798cbb756337a5f051e2b0dec3a52")},
                { 42132,   uint256S("0x945512bd3775afec9be70503302f230c3d5f678305fcc7225c3a6ca754ce296c")},
                { 50000,   uint256S("0xe3f63c89806b07831b9fb672abc53a9cdfcde4f40ee70286cadefa6af33c51d5")},
                { 75000,   uint256S("0xfcbba53ac9bb9df826a7fa3f607d59e49f340f52f68b9abb3b42d8834438ff83")},
                { 100000,  uint256S("0x9f71c39ec777fa17296962cd61b0889d50a13e1709a6f4b430d6d53cd5efc54e")},
                { 500000,  uint256S("0xc92d2e9e67204d1e7c035869d1e2a109bec242674ed3f62cf37cd87dff80a178")},
                { 1000000, uint256S("0xf8709fd199b8954c108449756a900aa9ecc1872625c64e539350e23f16758d0b")},
                { 1300000, uint256S("0xededc42181abfc795975057c8d2afae6485ceb52a122e4d1198a3c2bb8ce2e97")},
            }
        };

        // Transaction data from a certain block onwards.
        chainTxData = ChainTxData{
            // Data from block height 1300000.
            // getchaintxstats 4096 ededc42181abfc795975057c8d2afae6485ceb52a122e4d1198a3c2bb8ce2e97
            1602237289, // * UNIX timestamp of last checkpoint block
            1470626,    // * total number of transactions between genesis and last checkpoint
                        //   (the tx=... number in the SetBestChain debug.log lines)
            0.00788     // * estimated number of transactions per day after checkpoint
        };
    }
};

/**
 * Testnet (v3)
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = "test";
        consensus.nSubsidyHalvingInterval = 420000;
        consensus.BIP16Height = 0;
        consensus.BIP34Height = 0;
        consensus.BIP34Hash = uint256S("TESTNETGENESISHASH");
        consensus.BIP65Height = 99999999;
        consensus.BIP66Height = 99999999;
        consensus.nPowTargetTimespan = 150; //
        consensus.nPowTargetSpacing = 150; // 
        consensus.nCoinbaseMaturity = 10;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 1512; // 75% for testchains
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1456790400; // March 1st, 2016
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1493596800; // May 1st, 2017

        // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 0;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256();
        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("af4c13931038aa281fcd5a8a9ad0e5811b6a5dc93698bed7ed99c0a8120411f2");

        pchMessageStart[0] = 0x0b;
        pchMessageStart[1] = 0x11;
        pchMessageStart[2] = 0x09;
        pchMessageStart[3] = 0x07;

        nDefaultPort = 19661;
        nPruneAfterHeight = 1000;

        genesis = CreateTestnetGenesisBlock(GENESIS_TIME, TESTNETNONCE, 1, 0 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();

        LogPrint(BCLog::DEVEL, "Testnet: %s.\n", genesis.ToString());

        assert(consensus.hashGenesisBlock == uint256S(TESTNETGENESISHASH));
        assert(genesis.hashMerkleRoot == uint256S(TESTNETMERKLETREEROOTHASH));

        vFixedSeeds.clear();
        vSeeds.clear();
        // nodes with support for servicebits filtering should be at the top
        vSeeds.emplace_back("minkiz.co");

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,111);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,196);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x03, 0x3C, 0x07, 0xA4}; // gptp
        base58Prefixes[EXT_SECRET_KEY] = {0x03, 0x3C, 0x07, 0xCE}; // gptv
        base58Prefixes[EXT_COIN_TYPE]  = {0x80, 0x00, 0x00, 0x01}; // BIP44 coin type is '1' (testnet)

        bech32_hrp = "gpt";

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;


        checkpointData = {
            {
                {0, uint256S(TESTNETGENESISHASH)},
            }
        };

        chainTxData = ChainTxData{
            // Data as of block 0 (height 0).
            0,   // * UNIX timestamp of last checkpoint block
            0,   // * total number of transactions between genesis and last checkpoint
                 //   (the tx=... number in the SetBestChain debug.log lines)
            0    // * estimated number of transactions per day after checkpoint
        };

    }
};

/**
 * Regression test
 */
class CRegTestParams : public CChainParams {
public:
    CRegTestParams() {
        strNetworkID = "regtest";
        consensus.nSubsidyHalvingInterval = 1200;
        consensus.BIP16Height = 0; // always enforce P2SH BIP16 on regtest
        consensus.BIP34Height = 0; // Activated before Gapcoin was cloned
        consensus.BIP34Hash = uint256S("MAINNETGENESISHASH");
        consensus.BIP65Height = 99999999; // BIP65 discusses replacing the ntimelock field with the new CHECKLOCKTIMEVERIFY OP code:
        consensus.BIP66Height = 99999999;
        consensus.nPowTargetTimespan = 150; // 
        consensus.nPowTargetSpacing = 150; // 
        consensus.nCoinbaseMaturity = 10;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.nRuleChangeActivationThreshold = 108; // 75% for testchains
        consensus.nMinerConfirmationWindow = 144; // Faster than normal for regtest (144 instead of 2016)
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;

        // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256();

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S(""); //

        pchMessageStart[0] = 0xfa;
        pchMessageStart[1] = 0xbf;
        pchMessageStart[2] = 0xb5;
        pchMessageStart[3] = 0xda;
        nDefaultPort = 18444;
        nPruneAfterHeight = 1000;

        genesis = CreateMainnetGenesisBlock(GENESIS_TIME, MAINNETNONCE, 1, 0 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();

        LogPrint(BCLog::DEVEL, "Regtest: %s.\n", genesis.ToString());

        assert(consensus.hashGenesisBlock == uint256S(REGTESTGENESISHASH));
        assert(genesis.hashMerkleRoot == uint256S(MERKLETREEROOTHASH));

        vFixedSeeds.clear(); //!< Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();      //!< Regtest mode doesn't have any DNS seeds.

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,38);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,5);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,97);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x03, 0x3C, 0x07, 0xA4}; // as per testnet
        base58Prefixes[EXT_SECRET_KEY] = {0x03, 0x3C, 0x07, 0xCE}; // as per tstnet
        base58Prefixes[EXT_COIN_TYPE]  = {0x80, 0x00, 0x00, 0x01}; // BIP44 coin type is '1' (testnet)


        bech32_hrp = "gprt";

        checkpointData = {
            {
                {0, uint256S(REGTESTGENESISHASH)},
            }
        };

        chainTxData = ChainTxData{
            // Data as of block 0 (height 0).
            0,   // * UNIX timestamp of last checkpoint block
            0,   // * total number of transactions between genesis and last checkpoint
                 //   (the tx=... number in the SetBestChain debug.log lines)
            0    // * estimated number of transactions per day after checkpoint
        };

    }
};

static std::unique_ptr<CChainParams> globalChainParams;

const CChainParams &Params() {
    assert(globalChainParams);
    return *globalChainParams;
}

std::unique_ptr<CChainParams> CreateChainParams(const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN)
        return std::unique_ptr<CChainParams>(new CMainParams());
    else if (chain == CBaseChainParams::TESTNET)
        return std::unique_ptr<CChainParams>(new CTestNetParams());
    else if (chain == CBaseChainParams::REGTEST)
        return std::unique_ptr<CChainParams>(new CRegTestParams());
    throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    globalChainParams = CreateChainParams(network);
}

void UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
{
    globalChainParams->UpdateVersionBitsParameters(d, nStartTime, nTimeout);
}

/*

BIP39 Extended keys
{
    'public': '033C0DF8',
    'private': '033C0E22',
}
Mainnet and Regtest
{
    'p2pkhpub': '033C0DF8',
    'p2pkhprv': '033C0E22',
    'p2shpub': '033C0DF8',
    'p2shprv': '033C0E22',
    'p2sh_p2wpkhpub': '033C0F8D',
    'p2sh_p2wpkhprv': '033C0FB7',
    'p2sh_p2wshpub': '033BE7FF',
    'p2sh_p2wshprv': '033BE829',
    'p2wpkhpub': '033C1123',
    'p2wpkhprv': '033C114C',
    'p2wshpub': '033BE994',
    'p2wshprv': '033BE9BE'
}
Testnet
{
    'p2pkhpub': '033C07A4',
    'p2pkhprv': '033C07CE',
    'p2shpub': '033C07A4',
    'p2shprv': '033C07CE',
    'p2sh_p2wpkhpub': '033C0939',
    'p2sh_p2wpkhprv': '033C0963',
    'p2sh_p2wshpub': '033BE1AB',
    'p2sh_p2wshprv': '033BE1D5',
    'p2wpkhpub': '033C0ACE',
    'p2wpkhprv': '033C0AF8',
    'p2wshpub': '033BE340',
    'p2wshprv': '033BE36A'
}

*/
