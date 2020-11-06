// Copyright (c) 2011-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <test/test_bitcoin.h>

#include <chainparams.h>
#include <consensus/consensus.h>
#include <consensus/validation.h>
#include <crypto/sha256.h>
#include <validation.h>
#include <miner.h>
#include <net_processing.h>
#include <ui_interface.h>
#include <streams.h>
#include <rpc/server.h>
#include <rpc/register.h>
#include <script/sigcache.h>

#include <memory>

void CConnmanTest::AddNode(CNode& node)
{
    LOCK(g_connman->cs_vNodes);
    g_connman->vNodes.push_back(&node);
}

void CConnmanTest::ClearNodes()
{
    LOCK(g_connman->cs_vNodes);
    g_connman->vNodes.clear();
}

uint256 insecure_rand_seed = GetRandHash();
FastRandomContext insecure_rand_ctx(insecure_rand_seed);

extern bool fPrintToConsole;
extern void noui_connect();

std::ostream& operator<<(std::ostream& os, const uint256& num)
{
    os << num.ToString();
    return os;
}

BasicTestingSetup::BasicTestingSetup(const std::string& chainName)
{
        SHA256AutoDetect();
        RandomInit();
        ECC_Start();
        SetupEnvironment();
        SetupNetworking();
        InitSignatureCache();
        InitScriptExecutionCache();
        fPrintToDebugLog = false; // don't want to write to debug.log file
        fCheckBlockIndex = true;
        SelectParams(chainName);
        noui_connect();
}

BasicTestingSetup::~BasicTestingSetup()
{
        ECC_Stop();
}

TestingSetup::TestingSetup(const std::string& chainName) : BasicTestingSetup(chainName)
{
    const CChainParams& chainparams = Params();
        // Ideally we'd move all the RPC tests to the functional testing framework
        // instead of unit tests, but for now we need these here.

        RegisterAllCoreRPCCommands(tableRPC);
        ClearDatadirCache();
        pathTemp = fs::temp_directory_path() / strprintf("test_gapcoin_%lu_%i", (unsigned long)GetTime(), (int)(InsecureRandRange(100000)));
        fs::create_directories(pathTemp);
        gArgs.ForceSetArg("-datadir", pathTemp.string());

        // We have to run a scheduler thread to prevent ActivateBestChain
        // from blocking due to queue overrun.
        threadGroup.create_thread(boost::bind(&CScheduler::serviceQueue, &scheduler));
        GetMainSignals().RegisterBackgroundSignalScheduler(scheduler);

        mempool.setSanityCheck(1.0);
        pblocktree.reset(new CBlockTreeDB(1 << 20, true));
        pcoinsdbview.reset(new CCoinsViewDB(1 << 23, true));
        pcoinsTip.reset(new CCoinsViewCache(pcoinsdbview.get()));
        if (!LoadGenesisBlock(chainparams)) {
            throw std::runtime_error("LoadGenesisBlock failed.");
        }
        {
            CValidationState state;
            if (!ActivateBestChain(state, chainparams)) {
                throw std::runtime_error("ActivateBestChain failed.");
            }
        }
        nScriptCheckThreads = 3;
        for (int i=0; i < nScriptCheckThreads-1; i++)
            threadGroup.create_thread(&ThreadScriptCheck);
        g_connman = std::unique_ptr<CConnman>(new CConnman(0x1337, 0x1337)); // Deterministic randomness for tests.
        connman = g_connman.get();
        peerLogic.reset(new PeerLogicValidation(connman, scheduler));
}

TestingSetup::~TestingSetup()
{
        threadGroup.interrupt_all();
        threadGroup.join_all();
        GetMainSignals().FlushBackgroundCallbacks();
        GetMainSignals().UnregisterBackgroundSignalScheduler();
        g_connman.reset();
        peerLogic.reset();
        UnloadBlockIndex();
        pcoinsTip.reset();
        pcoinsdbview.reset();
        pblocktree.reset();
        fs::remove_all(pathTemp);
}

TestChain100Setup::TestChain100Setup() : TestingSetup(CBaseChainParams::REGTEST)
{
    // CreateAndProcessBlock() does not support building SegWit blocks, so don't activate in these tests.
    // TODO: fix the code to support SegWit blocks.
    UpdateVersionBitsParameters(Consensus::DEPLOYMENT_SEGWIT, 0, Consensus::BIP9Deployment::NO_TIMEOUT);
    // Generate a 100-block chain:
    coinbaseKey.MakeNewKey(true);
    CScript scriptPubKey = CScript() <<  ToByteVector(coinbaseKey.GetPubKey()) << OP_CHECKSIG;
    for (int i = 0; i < COINBASE_MATURITY; i++)
    {
        std::vector<CMutableTransaction> noTxns;
        CBlock b = CreateAndProcessBlock(noTxns, scriptPubKey);
        coinbaseTxns.push_back(*b.vtx[0]);
    }
}

//
// Create a new block with just given transactions, coinbase paying to
// scriptPubKey, and try to add it to the current chain.
//
CBlock
TestChain100Setup::CreateAndProcessBlock(const std::vector<CMutableTransaction>& txns, const CScript& scriptPubKey)
{
    std::cout << "CreateAndProcessBlock" << std::endl;
    const CChainParams& chainparams = Params();
    std::unique_ptr<CBlockTemplate> pblocktemplate = BlockAssembler(chainparams).CreateNewBlock(scriptPubKey);
    CBlock& block = pblocktemplate->block;

    // Replace mempool-selected txns with just coinbase plus passed-in txns:
    block.nDifficulty = PoWUtils::min_test_difficulty;
    block.nShift = 20;
    uint8_t nAdd[]      = { 25, 1 };
    block.nAdd.assign(nAdd, nAdd + sizeof(nAdd) / sizeof(uint8_t));
    block.vtx.resize(1);
    for (const CMutableTransaction& tx : txns)
        block.vtx.push_back(MakeTransactionRef(tx));
    // IncrementExtraNonce creates a valid coinbase and merkleRoot
    unsigned int extraNonce = 0;
    {
        LOCK(cs_main);
        IncrementExtraNonce(&block, chainActive.Tip(), extraNonce);
    }

    while (!CheckProofOfWork(block.GetHash(), block.nBits, chainparams.GetConsensus())) ++block.nNonce;

    std::shared_ptr<const CBlock> shared_pblock = std::make_shared<const CBlock>(block);
    ProcessNewBlock(chainparams, shared_pblock, true, nullptr);

    CBlock result = block;
    return result;
}

TestChain100Setup::~TestChain100Setup()
{
}


CTxMemPoolEntry TestMemPoolEntryHelper::FromTx(const CMutableTransaction &tx) {
    // std::cout << "TestMemPoolEntryHelper::FromTx CMutableTransaction" << std::endl;
    CTransaction txn(tx);
    return FromTx(txn);
}

CTxMemPoolEntry TestMemPoolEntryHelper::FromTx(const CTransaction &txn) {
    // std::cout << "TestMemPoolEntryHelper::FromTx CTransaction" << std::endl;
    return CTxMemPoolEntry(MakeTransactionRef(txn), nFee, nTime, nHeight,
                           spendsCoinbase, sigOpCost, lp);
}

/**
 * @returns a real block (c5de903b6ac0a97ec859dbf3b84e429b51f5d43c7f80d9fa6fa08af7a7e564a0)
 *      with 9 txs.
 */
CBlock getBlockc5de9()
{
    CBlock block;
    CDataStream stream(ParseHex("02000000ed360a46ce3f14a8fccd7f9636ebf5c90dedcd69053dc4195b805dc0a90dd3e5dbd42f9589e9db7e68a315df330408cbe6263ab64fb86a7581e318302a66072d383a975e29d65bf58a8a1500e85600002000030165f50901000000010000000000000000000000000000000000000000000000000000000000000000ffffffff0d03c538120101062f503253482fffffffff011a4f192000000000232103e52ee5838eb5867bf822632e78107937b6abdfcd0fbee1bd16890ea72aaeb106ac000000000100000001251fb5e3a02404b7d4996454872574b828751402efaa78f27815886c74934c640000000049483045022100c678366042c5e421229f33513de73a056c1350225dd76065db098adacfd28b2202204676e6ebf5398f68eaaee68606e92fafed560e507c445ceda1770325ed2a432601ffffffff026292f101000000001976a9140389077721a8cf4bb00f71a3ba09a57642625beb88ac0065cd1d000000001976a91450d323b47a1e0581c812162cae01dfa66ee9b1f888ac000000000100000001f7222c747d91e7d7e0815a375f6e54ddb533a8d1ba4f1d4a625408acc5c7c24c000000004847304402206185cfa1447f29273115b55e10fc9bcbeb862a05dcb1f20fd615207fc1be436802200e270dc2cd6fedc1ef37612d1120c4bdf50adbab31db568f1e38499e8a6eea5a01ffffffff020065cd1d000000001976a914d7dbbbf4fd63d992325b3d04682d45ce5426800388ac2b98f601000000001976a914f8672edd4a8081d701a069e167025ce8533d71d088ac0000000001000000019b2d17ec09cb77c951e3c22ed586cd72da5c6e4ad02ccb087b753216f23726180000000049483045022100f859512a930b04b3f07f0e3403a7b6c2b83d61ae034465326f4abea8fe2fed49022041f05d4dcf345b7a2515a4f5f65cf201117b30a2f2f77cf399324bd1d0ffb1a601ffffffff020065cd1d000000001976a914ade814b6ed553df75d5ce7102ed535215b48712e88ac5a91fc01000000001976a914e0d2a6b49be4e764b33ad8d04a2d9750ef28f93988ac00000000010000000109ddc734ecd3a61edc094f3661f12efc5f01fa44dffdc88479e3e5c4e6db7a80000000004847304402205fd3f8efc0d984ed687215cb2fb7965c607ca4d3eb21e2288f845cae0321118802204e66fd660b46edcbeb6eea8a0402a8738016b0c8808537818e7f33e10911f6ca01ffffffff02ec65f601000000001976a914793bbd459e47ae92c9cefc4d966b4db51cdbf03488ac0065cd1d000000001976a914861750960247f145d01b3003dab51d4ff5ed034488ac00000000010000000199e526c405758f8f768ea64eb3b58d80737699e7a83cd34d5e3b7f852ebb13ce0000000049483045022100e3bf9f097c93b5415579d05dd9114396d1c692460395409091e2a72785d6e6170220254cf75e3af655d9ef70d9b499ab22343e040607e86a892ddf46fb25a76e4ef801ffffffff02b0edf501000000001976a9143316f4ec37b1401870ddb63a3a54e21762df23d688ac0065cd1d000000001976a914fbe7e3f7caa3650afd5419522af313318c4bca8e88ac0000000001000000015f4a24234f6e350153ded3503d299af3cc5a03f4a177e171d92416efc0830aa80000000048473044022028b2fe5e8b0c1e644537ea6809317069a8df8744917b249b4992c3073e49c04c0220595a8a381a62296ffed0f4da10a34286cbff1dbf3df45f4a6e5ed699f4a9fa0601ffffffff0235c5fb01000000001976a9140d623ef4862cb956750238df5909eae5d76d021e88ac0065cd1d000000001976a914449f4b747aadd2f98ee3901fde38d6804e1de3c888ac000000000100000001b3cd594c8217aa88b719a08d91905622d26b4e8ff695d185f17ce350ac5998e9000000004847304402206c6732b6d0548a9f44d075dea826897ec3a8b8d6c5f51e7c60f8e2e6d1ba40d302205bfc6fbf25de537f0684237a63613071e921fe21a583cb0cb2a6a0b64d6eb87801ffffffff02a5c3f301000000001976a914e0a88a8c4e3c8b425c3fc665e87db4190aa40de688ac0065cd1d000000001976a9142d3791578c7d8f20c6604e3ca338eb88cdd6692588ac000000000100000001357101f8191a3e631c7f87c0b5086db19b3bec18b64a220a8c64777b28a1b2b3000000004847304402207ce55e9d95279b8a8c30a2b04e451f8a311f6c9a88531886d0ca4f0cad337e1c02204cf0caf8ad69064ec0938a961a893cc9d31fd257c6b01897fe0eecb05528505401ffffffff020065cd1d000000001976a914dfda5dab4dd3bcb5dce1110cfc4fc0c27212423e88ace6caf701000000001976a914886bdef01808d94110a5b5a905cc528ca4588a5288ac00000000"), SER_NETWORK, PROTOCOL_VERSION);
    stream >> block;
    return block;
}

CBlock getBlock0()
{
    CBlock block;
    CDataStream stream(ParseHex("01000000000000000000000000000000000000000000000000000000000000000000000094110EB4E3B9C2B70DF4B71DF19D5405CCA21E55C2565E356AE22AADCF101026209F465400000000000010003A340000140003E99C0F0101000000010000000000000000000000000000000000000000000000000000000000000000FFFFFFFF4804FFFF001D0104405468652054696D65732031352F4F63742F3230313420555320646174612073656E647320676C6F62616C2073746F636B7320696E746F207461696C2D7370696EFFFFFFFF0100000000000000004341044588D54931B7DE2F9FAAA5A3C1FDE654114AE51273754E1F3F9720127F8977AF6BFAA1F33E22E80E4B83F5269921501B411D254929FAF1B10D2174DED28AC59DAC00000000"), SER_NETWORK, PROTOCOL_VERSION);
    stream >> block;
    return block;
}
