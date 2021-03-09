// Copyright (c) 2011-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <base58.h>
#include <chainparams.h>
#include <coins.h>
#include <consensus/consensus.h>
#include <consensus/merkle.h>
#include <consensus/tx_verify.h>
#include <consensus/validation.h>
#include <core_io.h>
#include <validation.h>
#include <miner.h>
#include <policy/policy.h>
#include <pow.h>
#include <pubkey.h>
#include <script/standard.h>
#include <txmempool.h>
#include <uint256.h>
#include <util.h>
#include <utilstrencodings.h>

#include <PoWCore/src/PoW.h>
#include <PoWCore/src/PoWProcessor.h>
#include <PoWCore/src/PoWUtils.h>
#include <PoWCore/src/Sieve.h>

#include <test/test_bitcoin.h>

#include <memory>

#include <boost/test/unit_test.hpp>

class BlockProcessor : public PoWProcessor {

  public:

    BlockProcessor(CBlock *pblock, std::shared_ptr<CReserveScript> coinbase_script) : PoWProcessor() {
      this->pblock = pblock;
      this->coinbasescript = coinbase_script;
    }

    bool process(PoW *pow) {
      SetThreadPriority(THREAD_PRIORITY_NORMAL);

      pow->get_adder(&pblock->nAdd);
      bool ret = CheckWork(pblock, coinbasescript);

      SetThreadPriority(THREAD_PRIORITY_LOWEST);
      return !ret;
    }

  private:

    CBlock *pblock;
    std::shared_ptr<CReserveScript> coinbasescript;

};


BOOST_FIXTURE_TEST_SUITE(miner_tests, TestingSetup)

// BOOST_CHECK_EXCEPTION predicates to check the specific validation error
class HasReason {
public:
    HasReason(const std::string& reason) : m_reason(reason) {}
    bool operator() (const std::runtime_error& e) const {
        return std::string(e.what()).find(m_reason) != std::string::npos;
    };
private:
    const std::string m_reason;
};

static CFeeRate blockMinFeeRate = CFeeRate(DEFAULT_BLOCK_MIN_TX_FEE);

static BlockAssembler AssemblerForTest(const CChainParams& params) {
    BlockAssembler::Options options;

    options.nBlockMaxWeight = MAX_BLOCK_WEIGHT;
    options.blockMinFeeRate = blockMinFeeRate;
    return BlockAssembler(params, options);
}

/* Bitcoin-specific, unusable
static
struct {
    unsigned char extranonce;
    unsigned int nonce;
} blockinfo[] = {
    {4, 0xa4a3e223}, {2, 0x15c32f9e}, {1, 0x0375b547}, {1, 0x7004a8a5},
    {2, 0xce440296}, {2, 0x52cfe198}, {1, 0x77a72cd0}, {2, 0xbb5d6f84},
    {2, 0x83f30c2c}, {1, 0x48a73d5b}, {1, 0xef7dcd01}, {2, 0x6809c6c4},
    {2, 0x0883ab3c}, {1, 0x087bbbe2}, {2, 0x2104a814}, {2, 0xdffb6daa},
    {1, 0xee8a0a08}, {2, 0xba4237c1}, {1, 0xa70349dc}, {1, 0x344722bb},
    {3, 0xd6294733}, {2, 0xec9f5c94}, {2, 0xca2fbc28}, {1, 0x6ba4f406},
    {2, 0x015d4532}, {1, 0x6e119b7c}, {2, 0x43e8f314}, {2, 0x27962f38},
    {2, 0xb571b51b}, {2, 0xb36bee23}, {2, 0xd17924a8}, {2, 0x6bc212d9},
    {1, 0x630d4948}, {2, 0x9a4c4ebb}, {2, 0x554be537}, {1, 0xd63ddfc7},
    {2, 0xa10acc11}, {1, 0x759a8363}, {2, 0xfb73090d}, {1, 0xe82c6a34},
    {1, 0xe33e92d7}, {3, 0x658ef5cb}, {2, 0xba32ff22}, {5, 0x0227a10c},
    {1, 0xa9a70155}, {5, 0xd096d809}, {1, 0x37176174}, {1, 0x830b8d0f},
    {1, 0xc6e3910e}, {2, 0x823f3ca8}, {1, 0x99850849}, {1, 0x7521fb81},
    {1, 0xaacaabab}, {1, 0xd645a2eb}, {5, 0x7aea1781}, {5, 0x9d6e4b78},
    {1, 0x4ce90fd8}, {1, 0xabdc832d}, {6, 0x4a34f32a}, {2, 0xf2524c1c},
    {2, 0x1bbeb08a}, {1, 0xad47f480}, {1, 0x9f026aeb}, {1, 0x15a95049},
    {2, 0xd1cb95b2}, {2, 0xf84bbda5}, {1, 0x0fa62cd1}, {1, 0xe05f9169},
    {1, 0x78d194a9}, {5, 0x3e38147b}, {5, 0x737ba0d4}, {1, 0x63378e10},
    {1, 0x6d5f91cf}, {2, 0x88612eb8}, {2, 0xe9639484}, {1, 0xb7fabc9d},
    {2, 0x19b01592}, {1, 0x5a90dd31}, {2, 0x5bd7e028}, {2, 0x94d00323},
    {1, 0xa9b9c01a}, {1, 0x3a40de61}, {1, 0x56e7eec7}, {5, 0x859f7ef6},
    {1, 0xfd8e5630}, {1, 0x2b0c9f7f}, {1, 0xba700e26}, {1, 0x7170a408},
    {1, 0x70de86a8}, {1, 0x74d64cd5}, {1, 0x49e738a1}, {2, 0x6910b602},
    {0, 0x643c565f}, {1, 0x54264b3f}, {2, 0x97ea6396}, {2, 0x55174459},
    {2, 0x03e8779a}, {1, 0x98f34d8f}, {1, 0xc07b2b07}, {1, 0xdfe29668},
    {1, 0x3141c7c1}, {1, 0xb3b595f4}, {1, 0x735abf08}, {5, 0x623bfbce},
    {2, 0xd351e722}, {1, 0xf4ca48c9}, {1, 0x5b19c670}, {1, 0xa164bf0e},
    {2, 0xbbbeb305}, {2, 0xfe1c810a},
};
*/

static
struct {
    unsigned char extranonce;
    uint16_t shift;
    uint32_t nonce;
    uint64_t difficulty;
    std::vector<unsigned char> nadd;
} blockinfo[] = {
    {4, 25, 4, 4503599627370496, {193,135,69,1}}, {2, 25, 114, 4503599627370496, {169,33,179,1}},
    {1, 25, 33, 4509108879138899, {163,175,214}}, {1, 25, 22, 4514618130907302, {123,224,193}},
    {2, 25, 7, 4520127382675705, {111,32,45,1}}, {2, 25, 63, 4525636634444108, {217,162,72,1}},
    {1, 25, 230, 4531145886212511, {89,239,250,1}}, {2, 25, 194, 4536655137980914, {39,113,44,1}},
    {2, 25, 30, 4542164389749317, {25,220,55}}, {1, 25, 24, 4547673641517720, {229,184,129,1}},
    {1, 25, 232, 4553182893286123, {29,213,99,1}}, {2, 25, 236, 4558692145054526, {17,207,85}},
    {2, 25, 152, 4564201396822929, {175,155,7}}, {1, 25, 261, 4569710648591332, {51,237,83}},
    {2, 25, 3, 4575219900359735, {127,100,187,1}}, {2, 25, 134, 4580729152128138, {151,155,165}},
    {1, 25, 160, 4586238403896541, {219,255,202}}, {2, 25, 278, 4591747655664944, {181,136,100}},
    {1, 25, 319, 4597256907433347, {85,50,188}}, {1, 25, 178, 4602766159201750, {201,109,232,1}},
    {3, 25, 162, 4608275410970153, {79,231,49}}, {2, 25, 774, 4613784662738556, {215,213,134}},
    {2, 25, 509, 4619293914506959, {77,128,9,1}}, {1, 25, 310, 4624803166275362, {147,178,86,1}},
    {2, 25, 65, 4630312418043765, {49,51,102,1}}, {1, 25, 365, 4635821669812168, {211,46,100}},
    {2, 25, 276, 4641330921580571, {149,162,145}}, {2, 25, 44, 4646840173348974, {197,46,213}},
    {2, 25, 123, 4652349425117377, {223,128,93,1}}, {2, 25, 23, 4657858676885780, {1,162,41,1}},
    {2, 25, 3, 4663367928654183, {13,50,80,1}}, {2, 25, 55, 4668877180422586, {123,63,56,1}},
    {1, 25, 133, 4674386432190989, {33,45,232,1}}, {2, 25, 65, 4679895683959392, {55,136,195}},
    {2, 25, 102, 4685404935727795, {105,14,55,1}}, {1, 25, 172, 4690914187496198, {177,83,201,1}},
    {2, 25, 11, 4696423439264601, {57,54,227}}, {1, 25, 7, 4701932691033004, {87,26,188}},
    {2, 25, 1164, 4707441942801407, {43,137,28,1}}, {1, 25, 678, 4712951194569810, {245,213,5,1}},
    {1, 25, 147, 4718460446338213, {65,3,177}}, {3, 25, 776, 4723969698106616, {29,248,205,1}},
    {2, 25, 432, 4729478949875019, {115,151,208}}, {5, 25, 136, 4734988201643422, {133,205,33,1}},
    {1, 25, 143, 4740497453411825, {69,182,44,1}}, {5, 25, 463, 4746006705180228, {47,5,173}},
    {1, 25, 264, 4751515956948631, {197,252,152,1}}, {1, 25, 998, 4757025208717034, {195,41,235}},
    {1, 25, 269, 4762534460485437, {143,82,72,1}}, {2, 25, 32, 4768043712253840, {99,77,114,1}},
    {1, 25, 479, 4773552964022243, {79,140,104,1}}, {1, 25, 270, 4779062215790646, {137,45,71}},
    {1, 25, 586, 4784571467559049, {123,204,137}}, {1, 25, 236, 4790080719327452, {67,126,134,1}},
    {5, 25, 639, 4795589971095855, {75,97,43,1}}, {5, 25, 743, 4801099222864258, {33,101,151,1}},
    {1, 25, 1096, 4806608474632661, {43,7,214,1}}, {1, 25, 83, 4812117726401064, {207,0,71,1}},
    {6, 25, 373, 4817626978169467, {149,74,108,1}}, {2, 25, 709, 4823136229937870, {173,112,18}},
    {2, 25, 86, 4828645481706273, {201,144,198}}, {1, 25, 4, 4834154733474676, {177,92,159,1}},
    {1, 25, 104, 4839663985243079, {199,159,101}}, {1, 25, 213, 4845173237011482, {109,48,156}},
    {2, 25, 373, 4850682488779885, {251,66,190}}, {2, 25, 425, 4856191740548288, {97,100,181,1}},
    {1, 25, 258, 4861700992316691, {133,109,66}}, {1, 25, 499, 4867210244085094, {195,177,30}},
    {1, 25, 889, 4872719495853497, {191,178,93}}, {5, 25, 156, 4878228747621900, {65,17,24}},
    {5, 25, 257, 4883737999390303, {141,203,148,1}}, {1, 25, 138, 4889247251158706, {195,92,129}},
    {1, 25, 733, 4894756502927109, {155,131,252}}, {2, 25, 239, 4900265754695512, {27,58,217}},
    {2, 25, 1281, 4905775006463915, {65,39,62}}, {1, 25, 2037, 4911284258232318, {63,233,10}},
    {2, 25, 1507, 4916793510000721, {13,45,152,1}}, {1, 25, 318, 4922302761769124, {137,136,8}},
    {2, 25, 453, 4927812013537527, {117,180,43,1}}, {2, 25, 395, 4933321265305930, {33,18,29,1}},
    {1, 25, 318, 4938830517074333, {171,112,40}}, {1, 25, 464, 4944339768842736, {183,15,172}},
    {1, 25, 155, 4949849020611139, {19,179,106}}, {5, 25, 942, 4955358272379542, {73,46,228}},
    {1, 25, 97, 4960867524147945, {165,127,194,1}}, {1, 25, 787, 4966376775916348, {125,104,225}},
    {1, 25, 914, 4971886027684751, {9,6,11,1}}, {1, 25, 355, 4977395279453154, {43,82,230,1}},
    {1, 25, 340, 4982904531221557, {87,154,122}}, {1, 25, 587, 4988413782989960, {159,27,87,1}},
    {1, 25, 1280, 4993923034758363, {47,161,100,1}}, {2, 25, 765, 4999432286526766, {247,132,155,1}},
    {0, 25, 2102, 5004941538295169, {13,213,244}}, {1, 25, 502, 5010450790063572, {179,155,52}},
    {2, 25, 1188, 5015960041831975, {23,43,183,1}}, {2, 25, 1474, 5021469293600378, {137,189,196}},
    {2, 25, 21, 5026978545368781, {173,198,147,1}}, {1, 25, 623, 5032487797137184, {93,207,209,1}},
    {1, 25, 1297, 5037997048905587, {45,151,222}}, {1, 25, 3579, 5043506300673990, {227,174,163,1}},
    {1, 25, 739, 5049015552442393, {199,125,20}}, {1, 25, 1473, 5054524804210796, {49,174,23,1}},
    {1, 25, 1534, 5060034055979199, {217,14,86,1}}, {5, 25, 2015, 5065543307747602, {65,63,149}},
    {2, 25, 1288, 5071052559516005, {169,115,196,1}}, {1, 25, 2672, 5076561811284408, {159,0,39}},
    {1, 25, 31, 5082071063052811, {159,116,161,1}}, {1, 25, 2459, 5087580314821214, {37,1,252,1}},
    {2, 25, 1017, 5093089566589617, {49,189,58}}, {2, 25, 682, 5098598818358020, {49,91,174,1}},
};

CBlockIndex CreateBlockIndex(int nHeight)
{
    CBlockIndex index;
    index.nHeight = nHeight;
    index.pprev = chainActive.Tip();
    return index;
}

bool TestSequenceLocks(const CTransaction &tx, int flags)
{
    LOCK(mempool.cs);
    return CheckSequenceLocks(tx, flags);
}

// Test suite for ancestor feerate transaction selection.
// Implemented as an additional function, rather than a separate test case,
// to allow reusing the blockchain created in CreateNewBlock_validity.
void TestPackageSelection(const CChainParams& chainparams, CScript scriptPubKey, std::vector<CTransactionRef>& txFirst)
{
    BOOST_TEST_MESSAGE(strprintf("TestPackageSelection"));
    // Test the ancestor feerate transaction selection.
    TestMemPoolEntryHelper entry;

    // Test that a medium fee transaction will be selected after a higher fee
    // rate package with a low fee rate parent.
    CMutableTransaction tx;
    tx.vin.resize(1);
    tx.vin[0].scriptSig = CScript() << OP_1;
    tx.vin[0].prevout.hash = txFirst[0]->GetHash();
    tx.vin[0].prevout.n = 0;
    tx.vout.resize(1);
    // tx.vout[0].nValue = 5000000000LL - 1000;
    tx.vout[0].nValue = 0.00001205 - 100;
    // This tx has a low fee: 1000 satoshis
    uint256 hashParentTx = tx.GetHash(); // save this txid for later use
    mempool.addUnchecked(hashParentTx, entry.Fee(100).Time(GetTime()).SpendsCoinbase(true).FromTx(tx));

    // This tx has a medium fee: 10000 satoshis
    tx.vin[0].prevout.hash = txFirst[1]->GetHash();
    tx.vout[0].nValue = 0.00001205 - 500;
    uint256 hashMediumFeeTx = tx.GetHash();
    mempool.addUnchecked(hashMediumFeeTx, entry.Fee(500).Time(GetTime()).SpendsCoinbase(true).FromTx(tx));

    // This tx has a high fee, but depends on the first transaction
    tx.vin[0].prevout.hash = hashParentTx;
    tx.vout[0].nValue = 0.00001205 - 100 - 500; // 50k satoshi fee
    uint256 hashHighFeeTx = tx.GetHash();
    mempool.addUnchecked(hashHighFeeTx, entry.Fee(600).Time(GetTime()).SpendsCoinbase(false).FromTx(tx));

    std::unique_ptr<CBlockTemplate> pblocktemplate = AssemblerForTest(chainparams).CreateNewBlock(scriptPubKey);
    BOOST_CHECK(pblocktemplate->block.vtx[1]->GetHash() == hashParentTx);
    BOOST_CHECK(pblocktemplate->block.vtx[2]->GetHash() == hashHighFeeTx);
    BOOST_CHECK(pblocktemplate->block.vtx[3]->GetHash() == hashMediumFeeTx);

    // Test that a package below the block min tx fee doesn't get included
    tx.vin[0].prevout.hash = hashHighFeeTx;
    // tx.vout[0].nValue = 5000000000LL - 1000 - 50000; // 0 fee
    tx.vout[0].nValue = 0.00001205 - 100 - 1000; // 0 fee
    uint256 hashFreeTx = tx.GetHash();
    mempool.addUnchecked(hashFreeTx, entry.Fee(0).FromTx(tx));
    size_t freeTxSize = ::GetSerializeSize(tx, SER_NETWORK, PROTOCOL_VERSION);

    // Calculate a fee on child transaction that will put the package just
    // below the block min tx fee (assuming 1 child tx of the same size).
    CAmount feeToUse = blockMinFeeRate.GetFee(2*freeTxSize) - 1;

    tx.vin[0].prevout.hash = hashFreeTx;
    // tx.vout[0].nValue = 5000000000LL - 1000 - 50000 - feeToUse;
    tx.vout[0].nValue = 0.00001205 - 100 - 1000  - feeToUse;
    uint256 hashLowFeeTx = tx.GetHash();
    mempool.addUnchecked(hashLowFeeTx, entry.Fee(feeToUse).FromTx(tx));
    pblocktemplate = AssemblerForTest(chainparams).CreateNewBlock(scriptPubKey);
    // Verify that the free tx and the low fee tx didn't get selected
    for (size_t i=0; i<pblocktemplate->block.vtx.size(); ++i) {
        BOOST_CHECK(pblocktemplate->block.vtx[i]->GetHash() != hashFreeTx);
        BOOST_CHECK(pblocktemplate->block.vtx[i]->GetHash() != hashLowFeeTx);
    }

    // Test that packages above the min relay fee do get included, even if one
    // of the transactions is below the min relay fee
    // Remove the low fee transaction and replace with a higher fee transaction
    mempool.removeRecursive(tx);
    tx.vout[0].nValue -= 2; // Now we should be just over the min relay fee
    hashLowFeeTx = tx.GetHash();
    mempool.addUnchecked(hashLowFeeTx, entry.Fee(feeToUse+2).FromTx(tx));
    pblocktemplate = AssemblerForTest(chainparams).CreateNewBlock(scriptPubKey);
    BOOST_CHECK(pblocktemplate->block.vtx[4]->GetHash() == hashFreeTx);
    BOOST_CHECK(pblocktemplate->block.vtx[5]->GetHash() == hashLowFeeTx);

    // Test that transaction selection properly updates ancestor fee
    // calculations as ancestor transactions get included in a block.
    // Add a 0-fee transaction that has 2 outputs.
    tx.vin[0].prevout.hash = txFirst[2]->GetHash();
    tx.vout.resize(2);
    tx.vout[0].nValue = 5000000000LL - 100000000;
    tx.vout[1].nValue = 100000000; // 1GAP output
    uint256 hashFreeTx2 = tx.GetHash();
    mempool.addUnchecked(hashFreeTx2, entry.Fee(0).SpendsCoinbase(true).FromTx(tx));

    // This tx can't be mined by itself
    tx.vin[0].prevout.hash = hashFreeTx2;
    tx.vout.resize(1);
    feeToUse = blockMinFeeRate.GetFee(freeTxSize);
    tx.vout[0].nValue = 5000000000LL - 100000000 - feeToUse;
    uint256 hashLowFeeTx2 = tx.GetHash();
    mempool.addUnchecked(hashLowFeeTx2, entry.Fee(feeToUse).SpendsCoinbase(false).FromTx(tx));
    pblocktemplate = AssemblerForTest(chainparams).CreateNewBlock(scriptPubKey);

    // Verify that this tx isn't selected.
    for (size_t i=0; i<pblocktemplate->block.vtx.size(); ++i) {
        BOOST_CHECK(pblocktemplate->block.vtx[i]->GetHash() != hashFreeTx2);
        BOOST_CHECK(pblocktemplate->block.vtx[i]->GetHash() != hashLowFeeTx2);
    }

    // This tx will be mineable, and should cause hashLowFeeTx2 to be selected
    // as well.
    tx.vin[0].prevout.n = 1;
    tx.vout[0].nValue = 100000000 - 10000; // 10k satoshi fee
    mempool.addUnchecked(tx.GetHash(), entry.Fee(10000).FromTx(tx));
    pblocktemplate = AssemblerForTest(chainparams).CreateNewBlock(scriptPubKey);
    BOOST_CHECK(pblocktemplate->block.vtx[8]->GetHash() == hashLowFeeTx2);
}

// NOTE: These tests rely on CreateNewBlock doing its own self-validation!
BOOST_AUTO_TEST_CASE(CreateNewBlock_validity)
{
    // Note that by default, these tests run with size accounting enabled.
    BOOST_TEST_MESSAGE("Creating chain params");
    const auto chainParams = CreateChainParams(CBaseChainParams::MAIN);
    const CChainParams& chainparams = *chainParams;
    CScript scriptPubKey = CScript() << ParseHex("044588d54931b7de2f9faaa5a3c1fde654114ae51273754e1f3f9720127f8977af6bfaa1f33e22e80e4b83f5269921501b411d254929faf1b10d2174ded28ac59d") << OP_CHECKSIG;
    std::unique_ptr<CBlockTemplate> pblocktemplate;
    CMutableTransaction tx,tx2;
    CScript script;
    uint256 hash;
    TestMemPoolEntryHelper entry;
    entry.nFee = 11;
    entry.nHeight = 11;

    fCheckpointsEnabled = false;

    BOOST_TEST_MESSAGE(strprintf("Checking Simple block creation with pblocktemplate"));
    // Simple block creation, nothing special yet:
    BOOST_CHECK(pblocktemplate = AssemblerForTest(chainparams).CreateNewBlock(scriptPubKey));
    BOOST_TEST_MESSAGE(strprintf("Succeeded with simple block creation with pblocktemplate"));

    // We can't make transactions until we have inputs
    // Therefore, load 100 blocks :)
    BOOST_TEST_MESSAGE("Creating 110 blocks.");
    uint64_t nMiningSieveSize = 33554432;
    uint64_t nMiningPrimes = 900000;
    uint256 hashTarget = uint256S("7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF");

    int baseheight = 0;
    std::vector<CTransactionRef> txFirst;
    for (unsigned int i = 0; i < sizeof(blockinfo)/sizeof(*blockinfo); ++i)
    {
        // std::cout << "Block #" << i << std::endl;
        CBlock *pblock = &pblocktemplate->block; // pointer for convenience
        {
            LOCK(cs_main);
            pblock->nVersion = 1;
            pblock->nShift = 25;
            pblock->nTime = chainActive.Tip()->GetMedianTimePast()+1;
            if (i == 0)
                pblock->nDifficulty = PoWUtils::min_difficulty;
            else {
                CBlockIndex* pindexLast = chainActive.Tip();
                pblock->nDifficulty = GetNextWorkRequired(pindexLast, pblock, chainparams.GetConsensus());
            }
            CMutableTransaction txCoinbase(*pblock->vtx[0]);
            txCoinbase.nVersion = 1;
            txCoinbase.vin[0].scriptSig = CScript();
            txCoinbase.vin[0].scriptSig.push_back(blockinfo[i].extranonce);
            txCoinbase.vin[0].scriptSig.push_back(chainActive.Height());
            txCoinbase.vout.resize(1); // Ignore the (optional) segwit commitment added by CreateNewBlock (as the hardcoded nonces don't account for this)
            txCoinbase.vout[0].scriptPubKey = CScript();
            pblock->vtx[0] = MakeTransactionRef(std::move(txCoinbase));
            if (txFirst.size() == 0)
                baseheight = chainActive.Height();
            if (txFirst.size() < 4)
                txFirst.push_back(pblock->vtx[0]);
            pblock->hashMerkleRoot = BlockMerkleRoot(*pblock);
            pblock->nNonce = 0;

            /* FIXME: gjh run once to populate blockinfo
            CTxDestination destination = DecodeDestination("Gg6zcf8yLbPdy1b14T135QzhjhZXQTSVW2");
            std::shared_ptr<CReserveScript> coinbaseScript = std::make_shared<CReserveScript>();
            coinbaseScript->reserveScript = GetScriptForDestination(destination);

            BlockProcessor processor(pblock, coinbaseScript);
            Sieve sieve(NULL, nMiningPrimes, nMiningSieveSize);
            sieve.set_pprocessor(&processor);

            while (pblock->GetHash() < hashTarget) {
                pblock->nNonce += 1;
                uint256 hash = pblock->GetHash();
                std::vector<uint8_t> vHash(hash.begin(), hash.end());
                PoW pow(&vHash, pblock->nShift, &pblock->nAdd, pblock->nDifficulty);
                sieve.run_sieve(&pow, NULL);
                if (pow.valid()) {
                    break;
                }
            }
            std::cout << "{" << strprintf("%u", blockinfo[i].extranonce) << ", " << pblock->nShift << ", " << pblock->nNonce << ", " << pblock->nDifficulty << ", {" ;
            const std::vector<unsigned char>::size_type N(pblock->nAdd.size());
            std::cout << "{";
            for (size_t i=0; i<N; i++)
            {
                std::cout << static_cast<unsigned int>(pblock->nAdd[i]);
                if (i < (N-1))
                    std::cout << ",";
            }
            std::cout << "}}," << std::endl;
            */
            pblock->nShift = blockinfo[i].shift;
            pblock->nNonce = blockinfo[i].nonce;
            pblock->nDifficulty = blockinfo[i].difficulty;
            pblock->nAdd = blockinfo[i].nadd;
        }
        std::shared_ptr<const CBlock> shared_pblock = std::make_shared<const CBlock>(*pblock);
        BOOST_CHECK(ProcessNewBlock(chainparams, shared_pblock, true, nullptr));
        pblock->hashPrevBlock = pblock->GetHash();
    }
    BOOST_TEST_MESSAGE("110 blocks created.");

    LOCK(cs_main);

    BOOST_TEST_MESSAGE(strprintf("Ensure we can still make simple blocks."));
    // Just to make sure we can still make simple blocks
    BOOST_CHECK(pblocktemplate = AssemblerForTest(chainparams).CreateNewBlock(scriptPubKey));
    // BOOST_TEST_MESSAGE(strprintf("We can still make simple blocks.\n\n"));

    const CAmount BLOCKSUBSIDY = 50*COIN;
    const CAmount LOWFEE = CENT;
    const CAmount HIGHFEE = COIN;
    const CAmount HIGHERFEE = 4*COIN;

    const CAmount GAPBLOCKSUBSIDY = 0.00001205;
    const CAmount GAPLOWFEE = 0.00000100;
    const CAmount GAPHIGHFEE = 0.00000500;
    const CAmount GAPHIGHERFEE = 0.00001000;

    BOOST_TEST_MESSAGE("Setting up test for checking block sigops > limit: 1000 CHECKMULTISIG + 1");
    tx.vin.resize(1);
    // NOTE: OP_NOP is used to force 20 SigOps for the CHECKMULTISIG
    tx.vin[0].scriptSig = CScript() << OP_0 << OP_0 << OP_0 << OP_NOP << OP_CHECKMULTISIG << OP_1;
    tx.vin[0].prevout.hash = txFirst[0]->GetHash();
    tx.vin[0].prevout.n = 0;
    tx.vout.resize(1);
    // Okay to use Bitcoin BLOCKSUBSIDY of 50 COIN here.
    tx.vout[0].nValue = BLOCKSUBSIDY;

    for (unsigned int i = 0; i < 1001; ++i)
    {
        tx.vout[0].nValue -= GAPLOWFEE;
        hash = tx.GetHash();
        bool spendsCoinbase = i == 0; // only first tx spends coinbase
        // If we don't set the # of sig ops in the CTxMemPoolEntry, template creation fails
        mempool.addUnchecked(hash, entry.Fee(GAPLOWFEE).Time(GetTime()).SpendsCoinbase(spendsCoinbase).FromTx(tx));
        tx.vin[0].prevout.hash = hash;
    }

    BOOST_TEST_MESSAGE("Check block sigops > limit: 1000 CHECKMULTISIG + 1 creates error.");
    BOOST_CHECK_EXCEPTION(AssemblerForTest(chainparams).CreateNewBlock(scriptPubKey), std::runtime_error, HasReason("bad-blk-sigops"));
    BOOST_TEST_MESSAGE("Block sigops test passed, clearing mempool.");

    mempool.clear();

    BOOST_TEST_MESSAGE("Sigops cost check.");

    tx.vin[0].prevout.hash = txFirst[0]->GetHash();
    // Not okay to use Bitcoin BLOCKSUBSIDY of 50 COIN here.
    // tx.vout[0].nValue = BLOCKSUBSIDY;
    tx.vout[0].nValue = GetBlockSubsidy(1, blockinfo[1].difficulty, chainparams.GetConsensus());
    BOOST_TEST_MESSAGE(strprintf("GapCoin block #1 tx.vout[0].nValue=%llu", tx.vout[0].nValue));

    BOOST_TEST_MESSAGE("Create 1001 extra-lowfee vouts with sigopscost 80 in mempool.");
    for (unsigned int i = 0; i < 1001; ++i)
    {
        tx.vout[0].nValue -= GAPLOWFEE;
        hash = tx.GetHash();
        bool spendsCoinbase = i == 0; // only first tx spends coinbase
        // If we do set the # of sig ops in the CTxMemPoolEntry, template creation passes
        mempool.addUnchecked(hash, entry.Fee(GAPLOWFEE).Time(GetTime()).SpendsCoinbase(spendsCoinbase).SigOpsCost(80).FromTx(tx));
        tx.vin[0].prevout.hash = hash;
    }
    BOOST_TEST_MESSAGE("Check sigops cost.");
    BOOST_CHECK(pblocktemplate = AssemblerForTest(chainparams).CreateNewBlock(scriptPubKey));
    BOOST_TEST_MESSAGE("Sigops cost checked okay, clearing mempool");
    mempool.clear();

    // BOOST_TEST_MESSAGE("Skipping block size limit test.");

    BOOST_TEST_MESSAGE("Test for checking block size limit.");
    // block size > limit
    tx.vin[0].scriptSig = CScript();
    // 18 * (520char + DROP) + OP_1 = 9433 bytes
    std::vector<unsigned char> vchData(520);
    for (unsigned int i = 0; i < 18; ++i)
        tx.vin[0].scriptSig << vchData << OP_DROP;
    tx.vin[0].scriptSig << OP_1;
    tx.vin[0].prevout.hash = txFirst[0]->GetHash();
    // Not okay to use Bitcoin BLOCKSUBSIDY of 50 COIN here.
    // tx.vout[0].nValue = BLOCKSUBSIDY;
    tx.vout[0].nValue = GetBlockSubsidy(1, blockinfo[1].difficulty, chainparams.GetConsensus());
    for (unsigned int i = 0; i < 128; ++i)
    {
        tx.vout[0].nValue -= GAPLOWFEE;
        hash = tx.GetHash();
        bool spendsCoinbase = i == 0; // only first tx spends coinbase
        mempool.addUnchecked(hash, entry.Fee(GAPLOWFEE).Time(GetTime()).SpendsCoinbase(spendsCoinbase).FromTx(tx));
        tx.vin[0].prevout.hash = hash;
    }
    BOOST_TEST_MESSAGE("Checking block size limit.");
    BOOST_CHECK(pblocktemplate = AssemblerForTest(chainparams).CreateNewBlock(scriptPubKey));
    mempool.clear();

    BOOST_TEST_MESSAGE("Skipping check of orphan in mempool, template creation fails.");

    // orphan in mempool, template creation fails
    hash = tx.GetHash();
    mempool.addUnchecked(hash, entry.Fee(GAPLOWFEE).Time(GetTime()).FromTx(tx));
    BOOST_CHECK_EXCEPTION(AssemblerForTest(chainparams).CreateNewBlock(scriptPubKey), std::runtime_error, HasReason("bad-txns-inputs-missingorspent"));
    mempool.clear();

    // BOOST_TEST_MESSAGE("Skipping check of child with higher feerate than parent.");

    BOOST_TEST_MESSAGE("Check of child with higher feerate than parent.");
    // child with higher feerate than parent
    tx.vin[0].scriptSig = CScript() << OP_1;
    tx.vin[0].prevout.hash = txFirst[1]->GetHash();
    tx.vout[0].nValue = GAPBLOCKSUBSIDY-GAPHIGHFEE;
    hash = tx.GetHash();
    mempool.addUnchecked(hash, entry.Fee(GAPHIGHFEE).Time(GetTime()).SpendsCoinbase(true).FromTx(tx));
    tx.vin[0].prevout.hash = hash;
    tx.vin.resize(2);
    tx.vin[1].scriptSig = CScript() << OP_1;
    tx.vin[1].prevout.hash = txFirst[0]->GetHash();
    tx.vin[1].prevout.n = 0;
    tx.vout[0].nValue = tx.vout[0].nValue+GAPBLOCKSUBSIDY-GAPHIGHERFEE; //First txn output + fresh coinbase - new txn fee
    hash = tx.GetHash();
    mempool.addUnchecked(hash, entry.Fee(GAPHIGHERFEE).Time(GetTime()).SpendsCoinbase(true).FromTx(tx));
    BOOST_CHECK(pblocktemplate = AssemblerForTest(chainparams).CreateNewBlock(scriptPubKey));
    mempool.clear();

    // BOOST_TEST_MESSAGE("Skipping check of coinbase in mempool, template creation fails.");
    BOOST_TEST_MESSAGE("Check of coinbase in mempool, template creation fails.");
    // coinbase in mempool, template creation fails
    tx.vin.resize(1);
    tx.vin[0].prevout.SetNull();
    tx.vin[0].scriptSig = CScript() << OP_0 << OP_1;
    tx.vout[0].nValue = 0;
    hash = tx.GetHash();
    // give it a fee so it'll get mined
    mempool.addUnchecked(hash, entry.Fee(GAPLOWFEE).Time(GetTime()).SpendsCoinbase(false).FromTx(tx));
    // Should throw bad-cb-multiple
    BOOST_CHECK_EXCEPTION(AssemblerForTest(chainparams).CreateNewBlock(scriptPubKey), std::runtime_error, HasReason("bad-cb-multiple"));
    mempool.clear();

    // BOOST_TEST_MESSAGE("Skipping check of double spend txn pair in mempool, template creation fails.");
    BOOST_TEST_MESSAGE("Check of double spend txn pair in mempool, template creation fails.");
    // double spend txn pair in mempool, template creation fails
    tx.vin[0].prevout.hash = txFirst[0]->GetHash();
    tx.vin[0].scriptSig = CScript() << OP_1;
    tx.vout[0].nValue = GAPBLOCKSUBSIDY-GAPHIGHFEE;
    tx.vout[0].scriptPubKey = CScript() << OP_1;
    hash = tx.GetHash();
    mempool.addUnchecked(hash, entry.Fee(GAPHIGHFEE).Time(GetTime()).SpendsCoinbase(true).FromTx(tx));
    tx.vout[0].scriptPubKey = CScript() << OP_2;
    hash = tx.GetHash();
    mempool.addUnchecked(hash, entry.Fee(GAPHIGHFEE).Time(GetTime()).SpendsCoinbase(true).FromTx(tx));
    BOOST_CHECK_EXCEPTION(AssemblerForTest(chainparams).CreateNewBlock(scriptPubKey), std::runtime_error, HasReason("bad-txns-inputs-missingorspent"));
    mempool.clear();

    // BOOST_TEST_MESSAGE("Skipping subsidy changing check.");
    BOOST_TEST_MESSAGE("Subsidy changing check.");
    // subsidy changing
    int nHeight = chainActive.Height();
    // Create an actual 209999-long block chain (without valid blocks).
    while (chainActive.Tip()->nHeight < 209999) {
        CBlockIndex* prev = chainActive.Tip();
        CBlockIndex* next = new CBlockIndex();
        next->phashBlock = new uint256(InsecureRand256());
        pcoinsTip->SetBestBlock(next->GetBlockHash());
        next->pprev = prev;
        next->nHeight = prev->nHeight + 1;
        next->BuildSkip();
        chainActive.SetTip(next);
    }
    BOOST_CHECK(pblocktemplate = AssemblerForTest(chainparams).CreateNewBlock(scriptPubKey));
    // Extend to a 210000-long block chain.
    while (chainActive.Tip()->nHeight < 210000) {
        CBlockIndex* prev = chainActive.Tip();
        CBlockIndex* next = new CBlockIndex();
        next->phashBlock = new uint256(InsecureRand256());
        pcoinsTip->SetBestBlock(next->GetBlockHash());
        next->pprev = prev;
        next->nHeight = prev->nHeight + 1;
        next->BuildSkip();
        chainActive.SetTip(next);
    }

    BOOST_CHECK(pblocktemplate = AssemblerForTest(chainparams).CreateNewBlock(scriptPubKey));

    // BOOST_TEST_MESSAGE("Skipping check of invalid p2sh txn in mempool, template creation fails.");
    BOOST_TEST_MESSAGE("Check of invalid p2sh txn in mempool, template creation fails.");
    // invalid p2sh txn in mempool, template creation fails
    tx.vin[0].prevout.hash = txFirst[0]->GetHash();
    tx.vin[0].prevout.n = 0;
    tx.vin[0].scriptSig = CScript() << OP_1;
    tx.vout[0].nValue = GAPBLOCKSUBSIDY-GAPLOWFEE;
    script = CScript() << OP_0;
    tx.vout[0].scriptPubKey = GetScriptForDestination(CScriptID(script));
    hash = tx.GetHash();
    mempool.addUnchecked(hash, entry.Fee(GAPLOWFEE).Time(GetTime()).SpendsCoinbase(true).FromTx(tx));
    tx.vin[0].prevout.hash = hash;
    tx.vin[0].scriptSig = CScript() << std::vector<unsigned char>(script.begin(), script.end());
    tx.vout[0].nValue -= GAPLOWFEE;
    hash = tx.GetHash();
    mempool.addUnchecked(hash, entry.Fee(GAPLOWFEE).Time(GetTime()).SpendsCoinbase(false).FromTx(tx));
    // Should throw block-validation-failed
    BOOST_CHECK_EXCEPTION(AssemblerForTest(chainparams).CreateNewBlock(scriptPubKey), std::runtime_error, HasReason("block-validation-failed"));
    mempool.clear();

    // Delete the dummy blocks again.
    while (chainActive.Tip()->nHeight > nHeight) {
        CBlockIndex* del = chainActive.Tip();
        chainActive.SetTip(del->pprev);
        pcoinsTip->SetBestBlock(del->pprev->GetBlockHash());
        delete del->phashBlock;
        delete del;
    }

    // non-final txs in mempool
    SetMockTime(chainActive.Tip()->GetMedianTimePast()+1);
    int flags = LOCKTIME_VERIFY_SEQUENCE|LOCKTIME_MEDIAN_TIME_PAST;
    // height map
    std::vector<int> prevheights;

    BOOST_TEST_MESSAGE("Check of relative height locked.");
    // relative height locked
    tx.nVersion = 2;
    tx.vin.resize(1);
    prevheights.resize(1);
    tx.vin[0].prevout.hash = txFirst[0]->GetHash(); // only 1 transaction
    tx.vin[0].prevout.n = 0;
    tx.vin[0].scriptSig = CScript() << OP_1;
    tx.vin[0].nSequence = chainActive.Tip()->nHeight + 1; // txFirst[0] is the 2nd block
    prevheights[0] = baseheight + 1;
    tx.vout.resize(1);
    tx.vout[0].nValue = GAPBLOCKSUBSIDY-GAPHIGHFEE;
    tx.vout[0].scriptPubKey = CScript() << OP_1;
    tx.nLockTime = 0;
    hash = tx.GetHash();
    mempool.addUnchecked(hash, entry.Fee(GAPHIGHFEE).Time(GetTime()).SpendsCoinbase(true).FromTx(tx));
    BOOST_CHECK(CheckFinalTx(tx, flags)); // Locktime passes
    BOOST_CHECK(!TestSequenceLocks(tx, flags)); // Sequence locks fail
    BOOST_CHECK(SequenceLocks(tx, flags, &prevheights, CreateBlockIndex(chainActive.Tip()->nHeight + 2))); // Sequence locks pass on 2nd block

    // BOOST_TEST_MESSAGE("Skipping check of relative time locked.");
    BOOST_TEST_MESSAGE("Check of relative time locked.");
    // relative time locked
    tx.vin[0].prevout.hash = txFirst[1]->GetHash();
    tx.vin[0].nSequence = CTxIn::SEQUENCE_LOCKTIME_TYPE_FLAG | (((chainActive.Tip()->GetMedianTimePast()+1-chainActive[1]->GetMedianTimePast()) >> CTxIn::SEQUENCE_LOCKTIME_GRANULARITY) + 1); // txFirst[1] is the 3rd block
    prevheights[0] = baseheight + 2;
    hash = tx.GetHash();
    mempool.addUnchecked(hash, entry.Time(GetTime()).FromTx(tx));
    BOOST_CHECK(CheckFinalTx(tx, flags)); // Locktime passes
    BOOST_CHECK(!TestSequenceLocks(tx, flags)); // Sequence locks fail

    for (int i = 0; i < CBlockIndex::nMedianTimeSpan; i++)
        chainActive.Tip()->GetAncestor(chainActive.Tip()->nHeight - i)->nTime += 512; //Trick the MedianTimePast
    BOOST_CHECK(SequenceLocks(tx, flags, &prevheights, CreateBlockIndex(chainActive.Tip()->nHeight + 1))); // Sequence locks pass 512 seconds later
    for (int i = 0; i < CBlockIndex::nMedianTimeSpan; i++)
        chainActive.Tip()->GetAncestor(chainActive.Tip()->nHeight - i)->nTime -= 512; //undo tricked MTP

    // BOOST_TEST_MESSAGE("Skipping check of absolute height locked.");
    BOOST_TEST_MESSAGE("Check of absolute height locked.");
    // absolute height locked
    tx.vin[0].prevout.hash = txFirst[2]->GetHash();
    tx.vin[0].nSequence = CTxIn::SEQUENCE_FINAL - 1;
    prevheights[0] = baseheight + 3;
    tx.nLockTime = chainActive.Tip()->nHeight + 1;
    hash = tx.GetHash();
    mempool.addUnchecked(hash, entry.Time(GetTime()).FromTx(tx));
    BOOST_CHECK(!CheckFinalTx(tx, flags)); // Locktime fails
    BOOST_CHECK(TestSequenceLocks(tx, flags)); // Sequence locks pass
    BOOST_CHECK(IsFinalTx(tx, chainActive.Tip()->nHeight + 2, chainActive.Tip()->GetMedianTimePast())); // Locktime passes on 2nd block

    // BOOST_TEST_MESSAGE("Skipping check of absolute time locked.");
    BOOST_TEST_MESSAGE("Check of absolute time locked.");
    // absolute time locked
    tx.vin[0].prevout.hash = txFirst[3]->GetHash();
    tx.nLockTime = chainActive.Tip()->GetMedianTimePast();
    prevheights.resize(1);
    prevheights[0] = baseheight + 4;
    hash = tx.GetHash();
    mempool.addUnchecked(hash, entry.Time(GetTime()).FromTx(tx));
    BOOST_CHECK(!CheckFinalTx(tx, flags)); // Locktime fails
    BOOST_CHECK(TestSequenceLocks(tx, flags)); // Sequence locks pass
    BOOST_CHECK(IsFinalTx(tx, chainActive.Tip()->nHeight + 2, chainActive.Tip()->GetMedianTimePast() + 1)); // Locktime passes 1 second later

    // BOOST_TEST_MESSAGE("Skipping check of mempool-dependent transactions (not added).");
    BOOST_TEST_MESSAGE("Check of mempool-dependent transactions (not added).");
    // mempool-dependent transactions (not added)
    tx.vin[0].prevout.hash = hash;
    prevheights[0] = chainActive.Tip()->nHeight + 1;
    tx.nLockTime = 0;
    tx.vin[0].nSequence = 0;
    BOOST_CHECK(CheckFinalTx(tx, flags)); // Locktime passes
    BOOST_CHECK(TestSequenceLocks(tx, flags)); // Sequence locks pass
    tx.vin[0].nSequence = 1;
    BOOST_CHECK(!TestSequenceLocks(tx, flags)); // Sequence locks fail
    tx.vin[0].nSequence = CTxIn::SEQUENCE_LOCKTIME_TYPE_FLAG;
    BOOST_CHECK(TestSequenceLocks(tx, flags)); // Sequence locks pass
    tx.vin[0].nSequence = CTxIn::SEQUENCE_LOCKTIME_TYPE_FLAG | 1;
    BOOST_CHECK(!TestSequenceLocks(tx, flags)); // Sequence locks fail

    BOOST_CHECK(pblocktemplate = AssemblerForTest(chainparams).CreateNewBlock(scriptPubKey));

    // BOOST_TEST_MESSAGE("Skip remaining tests.");
    // None of the of the absolute height/time locked tx should have made
    // it into the template because we still check IsFinalTx in CreateNewBlock,
    // but relative locked txs will if inconsistently added to mempool.
    // For now these will still generate a valid template until BIP68 soft fork
    BOOST_CHECK_EQUAL(pblocktemplate->block.vtx.size(), 3);
    // However if we advance height by 1 and time by 512, all of them should be mined
    for (int i = 0; i < CBlockIndex::nMedianTimeSpan; i++)
        chainActive.Tip()->GetAncestor(chainActive.Tip()->nHeight - i)->nTime += 512; //Trick the MedianTimePast
    chainActive.Tip()->nHeight++;
    SetMockTime(chainActive.Tip()->GetMedianTimePast() + 1);

    BOOST_CHECK(pblocktemplate = AssemblerForTest(chainparams).CreateNewBlock(scriptPubKey));
    BOOST_CHECK_EQUAL(pblocktemplate->block.vtx.size(), 5);

    chainActive.Tip()->nHeight--;
    SetMockTime(0);
    mempool.clear();

    TestPackageSelection(chainparams, scriptPubKey, txFirst);

    fCheckpointsEnabled = true;
}

BOOST_AUTO_TEST_SUITE_END()
