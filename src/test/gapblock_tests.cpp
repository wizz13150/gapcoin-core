// Copyright (c) 2011-2017 The Bitcoin Core developers
// Copyright (c) 2020 The Gapcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <cstddef>
#include <ostream>
#include <sstream>
#include <stdio.h>


#include <amount.h>
#include <base58.h>
#include <blockencodings.h>
#include <chainparams.h>
#include <chainparamsbase.h>
#include <coins.h>
#include <consensus/consensus.h>
#include <consensus/merkle.h>
#include <consensus/tx_verify.h>
#include <consensus/validation.h>
#include <miner.h>
#include <policy/policy.h>
#include <PoWCore/src/PoW.h>
#include <PoWCore/src/PoWProcessor.h>
#include <PoWCore/src/PoWUtils.h>
#include <PoWCore/src/Sieve.h>
#include <primitives/block.h>
#include <primitives/transaction.h>
#include <pubkey.h>
#include <random.h>
#include <script/standard.h>
#include <txmempool.h>
#include <uint256.h>
#include <util.h>
#include <utilstrencodings.h>
#include <validation.h>
#include <wallet/rpcwallet.h>
#include <wallet/wallet.h>

#include <test/test_bitcoin.h>

#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE(gapblock_tests, TestingSetup)

static PoWUtils *powUtils = new PoWUtils();

//class BlockProcessor : public PoWProcessor {
//public:
//    BlockProcessor(CBlock *pblock, CReserveScript coinbasescript) : PoWProcessor() {
//        this->pblock = pblock;
//        this->script = coinbasescript;
//    }

//    bool process(PoW *pow) {
//        pow->get_adder(&pblock->nAdd);
//        bool ret = CheckWork(pblock, script);
//        return !ret;
//    }

//private:
//    CBlock *pblock;
//    CReserveScript script;
//};

class BlockProcessor : public PoWProcessor {

  public:

    BlockProcessor(CBlock *pblock, std::shared_ptr<CReserveScript> coinbase_script) : PoWProcessor() {
      this->pblock = pblock;
      this->script = coinbase_script;
    }

    bool process(PoW *pow) {
      pow->get_adder(&pblock->nAdd);
      bool ret = CheckWork(pblock, script);
      return !ret;
    }

  private:

    CBlock *pblock;
    std::shared_ptr<CReserveScript> script;

};

//CBlockTemplate* CreateNewBlockWithKey(CReserveKey& reservekey)
//{
//    CKey coinbaseKey; // private/public key needed to spend coinbase transactions
//    UpdateVersionBitsParameters(Consensus::DEPLOYMENT_SEGWIT, 0, Consensus::BIP9Deployment::NO_TIMEOUT);
//    coinbaseKey.MakeNewKey(true);
//    CScript scriptPubKey = CScript() <<  ToByteVector(coinbaseKey.GetPubKey()) << OP_CHECKSIG;
//    return CreateNewBlock(scriptPubKey);
//}

//bool CheckWork(CBlock* pblock, CReserveKey& reservekey)
//{
//    uint256 hash = pblock->GetHash();
//    PoW pow(new std::vector<uint8_t>(hash.begin(), hash.end()),
//            pblock->nShift,
//            &pblock->nAdd,
//            pblock->nDifficulty);

//    uint64_t nDifficulty = pow.difficulty();

//    if (nDifficulty < pblock->nDifficulty)
//        return false;

//    //// debug print
//    LogPrintf("GapcoinMiner:\n");
//    LogPrintf("proof-of-work found  \ndifficulty: %" PRIu64 "  \ntarget: %" PRIu64 "\n", nDifficulty, pblock->nDifficulty);
//    pblock->print();
//    LogPrintf("generated %s\n", FormatMoney(pblock->vtx[0].vout[0].nValue));

//    // Found a solution
//    {
//        LOCK(cs_main);
//        if (pblock->hashPrevBlock != chainActive.Tip()->GetBlockHash())
//            return error("GapcoinMiner : generated block is stale");

//        // Remove key from key pool
//        reservekey.KeepKey();

//        // Track how many getdata requests this block gets
//        {
//            LOCK(wallet.cs_wallet);
//            wallet.mapRequestCount[pblock->GetHash()] = 0;
//        }

//        // Process this block the same as if we had received it from another node
//        CValidationState state;
//        if (!ProcessBlock(state, NULL, pblock))
//            return error("GapcoinMiner : ProcessBlock, block not accepted");
//    }

//    return true;
//}

static CFeeRate blockMinFeeRate = CFeeRate(DEFAULT_BLOCK_MIN_TX_FEE);

static BlockAssembler AssemblerForTest(const CChainParams& params) {
    BlockAssembler::Options options;

    options.nBlockMaxWeight = MAX_BLOCK_WEIGHT;
    options.blockMinFeeRate = blockMinFeeRate;
    return BlockAssembler(params, options);
}

void static GapcoinMiner()
{
    const auto chainParams = CreateChainParams(CBaseChainParams::MAIN);
    const CChainParams& chainparams = *chainParams;
    std::unique_ptr<CBlockTemplate> pblocktemplate;

    CTxDestination destination = DecodeDestination("Gg6zcf8yLbPdy1b14T135QzhjhZXQTSVW2");
    BOOST_CHECK(IsValidDestination(destination));

    std::shared_ptr<CReserveScript> coinbaseScript = std::make_shared<CReserveScript>();
    coinbaseScript->reserveScript = GetScriptForDestination(destination);

    unsigned int nExtraNonce = 0;
    uint256 hashTarget = uint256S("0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");

    uint64_t nMiningSieveSize = 1000;
    uint64_t nMiningPrimes = 1000;
    uint16_t nMiningShift = 15;

    Sieve sieve(NULL, nMiningPrimes, nMiningSieveSize);
    std::cout << "Sieve initialised." << std::endl;

    try {
        // Create new block
        CBlockIndex* pindexPrev = chainActive.Tip();

        std::cout << "Creating template." << std::endl;
        // std::unique_ptr<CBlockTemplate> pblocktemplate(BlockAssembler(Params()).CreateNewBlock(coinbaseScript->reserveScript));
        BOOST_CHECK(pblocktemplate = AssemblerForTest(chainparams).CreateNewBlock(coinbaseScript->reserveScript));

        if (!pblocktemplate.get()) {
            std::cout << "Template creation failed." << std::endl;
            return;
        }
        std::cout << "Template created." << std::endl;

        // std::cout << "pblock" << std::endl;
        // const std::vector<unsigned char>::size_type Z(pblocktemplate->block.nAdd.size());
        // std::cout << "{";
        // for (size_t i=0; i<Z; i++)
        // {
        //     std::cout << static_cast<unsigned int>(pblocktemplate->block.nAdd[i]) << ",";
        // }
        // std::cout << "}" << std::endl;
        // std::cout << "shift: " << pblocktemplate->block.nShift << std::endl;
        // std::cout << "difficulty: " << pblocktemplate->block.nDifficulty << std::endl;
        // std::cout << "nonce: " << pblocktemplate->block.nNonce << std::endl;

        CBlock *pblock = &pblocktemplate->block;
        IncrementExtraNonce(pblock, pindexPrev, nExtraNonce);

        pblock->nDifficulty = PoWUtils::min_test_difficulty;
        uint8_t nAdd[]      = { 25, 1 };
        pblock->nAdd.assign(nAdd, nAdd + sizeof(nAdd) / sizeof(uint8_t));

        const std::vector<unsigned char>::size_type T(pblock->nAdd.size());
        std::cout << "pblock" << std::endl;
        std::cout << "{";
        for (size_t i=0; i<T; i++)
        {
            std::cout << static_cast<unsigned int>(pblock->nAdd[i]) << ",";
        }
        std::cout << "}" << std::endl;
        std::cout << "shift: " << pblock->nShift << std::endl;
        std::cout << "difficulty: " << pblock->nDifficulty << std::endl;
        std::cout << "nonce: " << pblock->nNonce << std::endl;

        // Search
        BlockProcessor processor(pblock, coinbaseScript);
        // std::cout << "PoW processor initialised." << std::endl;
        sieve.set_pprocessor(&processor);
        // std::cout << "PoW processor bound to sieve." << std::endl;

        // int64_t nStart = GetTime();

        pblock->nNonce = 0;

        while (true)
        {
            /* header hash has to be greater than 2^255 - 1 */
            // std::cout << "pblock->GetHash()" << pblock->GetHash().ToString() << " hashTarget: " << hashTarget << std::endl;
            while (pblock->GetHash() < hashTarget) {
                pblock->nNonce += 1;
                if (pblock->nNonce % 1000 == 0)
                    std::cout << "pblock->nNonce = " << pblock->nNonce << std::endl;

                uint256 hash = pblock->GetHash();
                std::vector<uint8_t> vHash(hash.begin(), hash.end());

                PoW pow(&vHash, pblock->nShift, &pblock->nAdd, pblock->nDifficulty);
                sieve.run_sieve(&pow, NULL);
                if (pow.valid()) {
                    // std::cout << "Run with pow processor " << pow.to_s() << std::endl;
                    break;
                }
            }
            std::cout << "pblock: " << pblock->ToString() << std::endl;
            const std::vector<unsigned char>::size_type N(pblock->nAdd.size());
            std::cout << "{";
            for (size_t i=0; i<N; i++)
            {
                std::cout << static_cast<unsigned int>(pblock->nAdd[i]) << ",";
            }
            std::cout << "}" << std::endl;
            std::cout << "shift: " << pblock->nShift << std::endl;
            std::cout << "difficulty: " << pblock->nDifficulty << std::endl;
            std::cout << "nonce: " << pblock->nNonce << std::endl;
        }
    }
    catch (boost::thread_interrupted)
    {
        LogPrintf("GapcoinMiner terminated\n");
        throw;
    }
}


BOOST_AUTO_TEST_CASE(one)
{
    BOOST_TEST_MESSAGE(strprintf("nMiningSieveSize=%s, nMiningPrimes=%s, nMiningShift=%s.", nMiningSieveSize, nMiningPrimes, nMiningShift));
    /* FIXME: diabled, pro tem
    GapcoinMiner();
    */
    BOOST_TEST_MESSAGE(strprintf("Done"));
}

//static CBlock BuildBlockTestCase() {
//    CBlock block;
//    CMutableTransaction tx;
//    tx.vin.resize(1);
//    tx.vin[0].scriptSig.resize(10);
//    tx.vout.resize(1);
//    tx.vout[0].nValue = 42;

//    block.vtx.resize(3);
//    block.vtx[0] = MakeTransactionRef(tx);
//    block.nVersion = 42;
//    block.hashPrevBlock = InsecureRand256();
//    block.nDifficulty = PoWUtils::min_difficulty;
//    uint8_t nAdd[]      = { 25, 1 };
//    block.nAdd.assign(nAdd, nAdd + sizeof(nAdd) / sizeof(uint8_t));
//    tx.vin[0].prevout.hash = InsecureRand256();
//    tx.vin[0].prevout.n = 0;
//    block.vtx[1] = MakeTransactionRef(tx);

//    tx.vin.resize(10);
//    for (size_t i = 0; i < tx.vin.size(); i++) {
//        tx.vin[i].prevout.hash = InsecureRand256();
//        tx.vin[i].prevout.n = 0;
//    }

//    block.vtx[2] = MakeTransactionRef(tx);

//    bool mutated;
//    block.hashMerkleRoot = BlockMerkleRoot(block, &mutated);
//    assert(!mutated);
//    while (!CheckProofOfWork(block.GetHash(), block.nShift, &block.nAdd, block.nDifficulty, Params().GetConsensus())) ++block.nNonce;
//    return block;
//}

//static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, uint32_t nNonce, uint64_t nDifficulty, int32_t nVersion, const CAmount& genesisReward)
//{
//    CMutableTransaction txNew;
//    txNew.nVersion = 1;
//    txNew.vin.resize(1);
//    txNew.vout.resize(1);
//    txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
//    txNew.vout[0].nValue = genesisReward;
//    txNew.vout[0].scriptPubKey = genesisOutputScript;

//    CBlock genesis;
//    genesis.nTime    = nTime;
//    genesis.nNonce   = nNonce;
//    genesis.nVersion = nVersion;
//    genesis.nShift = 20;
//    genesis.nDifficulty = nDifficulty;
//    uint8_t nAdd[] = { 233, 156, 15 };
//    genesis.nAdd.assign(nAdd, nAdd + sizeof(nAdd) / sizeof(uint8_t));
//    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
//    genesis.hashPrevBlock.SetNull();
//    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
//    return genesis;
//}

//static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint64_t nDifficulty, int32_t nVersion, const CAmount& genesisReward)
//{
//    const char* pszTimestamp = "The Times 15/Oct/2014 US data sends global stocks into tail-spin";
//    const CScript genesisOutputScript = CScript() << ParseHex("044588d54931b7de2f9faaa5a3c1fde654114ae51273754e1f3f9720127f8977af6bfaa1f33e22e80e4b83f5269921501b411d254929faf1b10d2174ded28ac59d") << OP_CHECKSIG;
//    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nDifficulty, nVersion, genesisReward);
//}

//static bool CheckPoW(const uint256 hash, const uint16_t nShift, const std::vector<uint8_t> *const nAdd, const uint64_t nDifficulty)
//{
//    std::vector<uint8_t> vHash(hash.begin(), hash.end());
  
//    PoW pow(&vHash, nShift, nAdd, nDifficulty);

//    // Check proof of work matches claimed amount
//    if (!pow.valid())
//        return error("CheckProofOfWork() : hash does not match nDifficulty");

//    return true;
//}

//static CBlock ReBuildGensisBlock() {
//    CBlock genesis = CreateGenesisBlock(1413914400, 13370, PoWUtils::min_difficulty, 1, 0 * COIN);
//    return genesis;
//}

//BOOST_AUTO_TEST_CASE(ReadHashTest, *boost::unit_test::disabled() *boost::unit_test::label("readhash") *boost::unit_test::description("Test skipped"))
//{
//    // head -c 298 blocks/blk00000.dat | tail -c +9 | hexdump -v -e '/1 "%02x"'
//    string genesisraw  = "01000000000000000000000000000000000000000000000000000000000000000000000094110eb4e3b9c2b70df4b71df19d5405cca21e55c2565e356ae22aadcf101026209f465400000000000010003a340000140003e99c0f0101000000010000000000000000000000000000000000000000000000000000000000000000ffffffff4804ffff001d0104405468652054696d65732031352f4f63742f3230313420555320646174612073656e647320676c6f62616c2073746f636b7320696e746f207461696c2d7370696effffffff0100000000000000004341044588d54931b7de2f9faaa5a3c1fde654114ae51273754e1f3f9720127f8977af6bfaa1f33e22e80e4b83f5269921501b411d254929faf1b10d2174ded28ac59dac00000000";
//    CBlock block;
//    CDataStream stream(ParseHex(genesisraw), SER_NETWORK, PROTOCOL_VERSION);
//    stream >> block;
//    BOOST_CHECK_EQUAL(block.GetHash().ToString(), "e798f3ae4f57adcf25740fe43100d95ec4fd5d43a1568bc89e2b25df89ff6cb0");
//    BOOST_CHECK_EQUAL(block.hashMerkleRoot.ToString(), "261010cfad2ae26a355e56c2551ea2cc05549df11db7f40db7c2b9e3b40e1194");
//}

//BOOST_AUTO_TEST_CASE(SimpleHashTest, *boost::unit_test::disabled() *boost::unit_test::description("test skipped"))
//{
//    CBlock block = getBlock0();
//    block.GetHash();
//    BOOST_CHECK_EQUAL(block.GetHash().ToString(), "e798f3ae4f57adcf25740fe43100d95ec4fd5d43a1568bc89e2b25df89ff6cb0");
//    BOOST_CHECK_EQUAL(block.hashMerkleRoot.ToString(), "261010cfad2ae26a355e56c2551ea2cc05549df11db7f40db7c2b9e3b40e1194");
//}

//BOOST_AUTO_TEST_CASE(ReadGenesisTest, *boost::unit_test::disabled() *boost::unit_test::description("test skipped"))
//{
//    CBlock block = ReBuildGensisBlock();
//    block.GetHash();
//    BOOST_CHECK_EQUAL(block.GetHash().ToString(), "e798f3ae4f57adcf25740fe43100d95ec4fd5d43a1568bc89e2b25df89ff6cb0");
//    BOOST_CHECK_EQUAL(block.hashMerkleRoot.ToString(), "261010cfad2ae26a355e56c2551ea2cc05549df11db7f40db7c2b9e3b40e1194");
//}

//BOOST_AUTO_TEST_CASE(powcheck, *boost::unit_test::disabled() *boost::unit_test::description("test skipped"))
//{
//    // uint256 hash = pindex->GetBlockHash();
//    // std::vector<uint8_t> vHash(hash.begin(), hash.end());
//    // PoW pow(&vHash, pindex->nShift, &pindex->nAdd, pindex->nDifficulty);

//}

BOOST_AUTO_TEST_SUITE_END()
