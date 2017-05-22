// Copyright (c) 2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

//#include <stdlib.h>

#include "chainparams.h"
#include "consensus/validation.h"
#include "core_io.h"
#include "miner.h"
#include "random.h"
#include "script/sigcache.h"
#include "sidechain.h"
#include "sidechaindb.h"
#include "uint256.h"
#include "utilstrencodings.h"
#include "validation.h"

#include "test/test_bitcoin.h"

#include <boost/test/unit_test.hpp>

//! KeyID for testing
// mx3PT9t2kzCFgAURR9HeK6B5wN8egReUxY
// cN5CqwXiaNWhNhx3oBQtA8iLjThSKxyZjfmieTsyMpG6NnHBzR7J
static const std::string testKey = "b5437dc6a4e5da5597548cf87db009237d286636";

BOOST_FIXTURE_TEST_SUITE(sidechaindb_tests, TestChain100Setup)

BOOST_AUTO_TEST_CASE(sidechaindb_isolated)
{
    // Test SidechainDB without blocks
    SidechainDB scdb;

    uint256 hashWTTest = GetRandHash();
    uint256 hashWTHivemind = GetRandHash();
    uint256 hashWTWimble = GetRandHash();

    scdb.Update(SIDECHAIN_TEST, 0, 100, hashWTTest);
    scdb.Update(SIDECHAIN_HIVEMIND, 0, 50, hashWTHivemind);
    scdb.Update(SIDECHAIN_WIMBLE, 0, 0, hashWTWimble);

    // WT^ 0 should pass with valid workscore (100/100)
    BOOST_CHECK(scdb.CheckWorkScore(SIDECHAIN_TEST, hashWTTest));
    // WT^ 1 should fail with unsatisfied workscore (50/100)
    BOOST_CHECK(!scdb.CheckWorkScore(SIDECHAIN_HIVEMIND, hashWTHivemind));
    // WT^ 2 should fail with unsatisfied workscore (0/100)
    BOOST_CHECK(!scdb.CheckWorkScore(SIDECHAIN_WIMBLE, hashWTWimble));
}

BOOST_AUTO_TEST_CASE(sidechaindb_MultipleTauPeriods)
{
    // Test SCDB with multiple tau periods,
    // approve multiple WT^s on the same sidechain.
    SidechainDB scdb;
    const Sidechain& test = ValidSidechains[SIDECHAIN_TEST];

    // WT^ hash for first period
    uint256 hashWTTest1 = GetRandHash();

    // Verify first transaction, check work score
    scdb.Update(SIDECHAIN_TEST, 0, test.nMinWorkScore, hashWTTest1);
    BOOST_CHECK(scdb.CheckWorkScore(SIDECHAIN_TEST, hashWTTest1));

    // Update SCDB (will clear out old state state from first period)
    CMutableTransaction mtx; // dummy transaction
    mtx.vout.push_back(CTxOut(50 * CENT, CScript() << OP_RETURN));
    scdb.Update(test.GetTau(), uint256(), MakeTransactionRef(mtx));

    // WT^ hash for second period
    uint256 hashWTTest2 = GetRandHash();

    // Partially verify second transaction but keep workscore < nMinWorkScore
    scdb.Update(SIDECHAIN_TEST, 0, test.nMinWorkScore - 1, hashWTTest2);
    BOOST_CHECK(!scdb.CheckWorkScore(SIDECHAIN_TEST, hashWTTest2));

    // Verify that SCDB has updated to correct WT^
    const std::vector<SidechainWTJoinState> vState = scdb.GetState(SIDECHAIN_TEST);
    BOOST_CHECK(vState.size() == 1);
    BOOST_CHECK(vState[0].wtxid == hashWTTest2);

    // Give second transaction sufficient workscore and check work score
    scdb.Update(SIDECHAIN_TEST, 0, test.nMinWorkScore, hashWTTest2);
    BOOST_CHECK(scdb.CheckWorkScore(SIDECHAIN_TEST, hashWTTest2));
}

BOOST_AUTO_TEST_CASE(sidechaindb_EmptyStateScript)
{
    // Test empty SCDB
    const Sidechain& sidechainTest = ValidSidechains[SIDECHAIN_TEST];
    CScript scriptEmptyExpected = CScript();
    SidechainDB scdbEmpty;
    BOOST_CHECK(scriptEmptyExpected == scdbEmpty.CreateStateScript(sidechainTest.GetTau() - 1));
}

BOOST_AUTO_TEST_CASE(sidechaindb_PopulatedStateScript)
{
    const Sidechain& sidechainTest = ValidSidechains[SIDECHAIN_TEST];

    // Test populated (but not full) SCDB
    CScript scriptPopulatedExpected;
    scriptPopulatedExpected << OP_RETURN << SCOP_VERSION << SCOP_VERSION_DELIM
                    << SCOP_VERIFY << SCOP_SC_DELIM
                    << SCOP_VERIFY << SCOP_SC_DELIM
                    << SCOP_VERIFY;

    SidechainDB scdbPopulated;

    uint256 hashWTTest = GetRandHash();
    uint256 hashWTHivemind = GetRandHash();
    uint256 hashWTWimble = GetRandHash();

    scdbPopulated.Update(SIDECHAIN_TEST, 0, 1, hashWTTest);
    scdbPopulated.Update(SIDECHAIN_HIVEMIND, 0, 1, hashWTHivemind);
    scdbPopulated.Update(SIDECHAIN_WIMBLE, 0, 1, hashWTWimble);

    BOOST_CHECK(scriptPopulatedExpected == scdbPopulated.CreateStateScript(sidechainTest.GetTau() - 1));
}

BOOST_AUTO_TEST_CASE(sidechaindb_FullStateScript)
{
    const Sidechain& sidechainTest = ValidSidechains[SIDECHAIN_TEST];

    // Test Full SCDB
    CScript scriptFullExpected;
    scriptFullExpected << OP_RETURN << SCOP_VERSION << SCOP_VERSION_DELIM
               << SCOP_VERIFY << SCOP_WT_DELIM << SCOP_REJECT << SCOP_WT_DELIM << SCOP_REJECT
               << SCOP_SC_DELIM
               << SCOP_VERIFY << SCOP_WT_DELIM << SCOP_REJECT << SCOP_WT_DELIM << SCOP_REJECT
               << SCOP_SC_DELIM
               << SCOP_VERIFY << SCOP_WT_DELIM << SCOP_REJECT << SCOP_WT_DELIM << SCOP_REJECT;

    SidechainDB scdbFull;

    uint256 hashWTTest1 = GetRandHash();
    uint256 hashWTTest2 = GetRandHash();
    uint256 hashWTTest3 = GetRandHash();

    scdbFull.Update(SIDECHAIN_TEST, 0, 1, hashWTTest1);
    scdbFull.Update(SIDECHAIN_TEST, 0, 0, hashWTTest2);
    scdbFull.Update(SIDECHAIN_TEST, 0, 0, hashWTTest3);

    uint256 hashWTHivemind1 = GetRandHash();
    uint256 hashWTHivemind2 = GetRandHash();
    uint256 hashWTHivemind3 = GetRandHash();

    scdbFull.Update(SIDECHAIN_HIVEMIND, 0, 1, hashWTHivemind1);
    scdbFull.Update(SIDECHAIN_HIVEMIND, 0, 0, hashWTHivemind2);
    scdbFull.Update(SIDECHAIN_HIVEMIND, 0, 0, hashWTHivemind3);

    uint256 hashWTWimble1 = GetRandHash();
    uint256 hashWTWimble2 = GetRandHash();
    uint256 hashWTWimble3 = GetRandHash();

    scdbFull.Update(SIDECHAIN_WIMBLE, 0, 1, hashWTWimble1);
    scdbFull.Update(SIDECHAIN_WIMBLE, 0, 0, hashWTWimble2);
    scdbFull.Update(SIDECHAIN_WIMBLE, 0, 0, hashWTWimble3);
    BOOST_CHECK(scriptFullExpected == scdbFull.CreateStateScript(sidechainTest.GetTau() - 1));
}

BOOST_AUTO_TEST_CASE(sidechaindb_CountStateScript)
{
    const Sidechain& sidechainTest = ValidSidechains[SIDECHAIN_TEST];

    // Test with different number of WT^s per sidechain
    CScript scriptWTCountExpected;
    scriptWTCountExpected << OP_RETURN << SCOP_VERSION << SCOP_VERSION_DELIM
                          << SCOP_VERIFY
                          << SCOP_SC_DELIM
                          << SCOP_REJECT << SCOP_WT_DELIM << SCOP_VERIFY
                          << SCOP_SC_DELIM
                          << SCOP_REJECT << SCOP_WT_DELIM << SCOP_VERIFY << SCOP_WT_DELIM << SCOP_REJECT;
    SidechainDB scdbCount;

    uint256 hashWTTest = GetRandHash();

    scdbCount.Update(SIDECHAIN_TEST, 0, 1, hashWTTest);

    uint256 hashWTHivemind1 = GetRandHash();
    uint256 hashWTHivemind2 = GetRandHash();

    scdbCount.Update(SIDECHAIN_HIVEMIND, 0, 0, hashWTHivemind1);
    scdbCount.Update(SIDECHAIN_HIVEMIND, 0, 1, hashWTHivemind2);

    uint256 hashWTWimble1 = GetRandHash();
    uint256 hashWTWimble2 = GetRandHash();
    uint256 hashWTWimble3 = GetRandHash();

    scdbCount.Update(SIDECHAIN_WIMBLE, 0, 0, hashWTWimble1);
    scdbCount.Update(SIDECHAIN_WIMBLE, 0, 1, hashWTWimble2);
    scdbCount.Update(SIDECHAIN_WIMBLE, 0, 0, hashWTWimble3);

    BOOST_CHECK(scriptWTCountExpected == scdbCount.CreateStateScript(sidechainTest.GetTau() - 1));
}

BOOST_AUTO_TEST_CASE(sidechaindb_PositionStateScript)
{
    // Verify that state scripts created based on known SidechainDB
    // state examples are formatted as expected
    const Sidechain& sidechainTest = ValidSidechains[SIDECHAIN_TEST];

    // Test WT^ in different position for each sidechain
    CScript scriptWTPositionExpected;
    scriptWTPositionExpected << OP_RETURN << SCOP_VERSION << SCOP_VERSION_DELIM
                             << SCOP_VERIFY << SCOP_WT_DELIM << SCOP_REJECT << SCOP_WT_DELIM << SCOP_REJECT
                             << SCOP_SC_DELIM
                             << SCOP_REJECT << SCOP_WT_DELIM << SCOP_VERIFY << SCOP_WT_DELIM << SCOP_REJECT
                             << SCOP_SC_DELIM
                             << SCOP_REJECT << SCOP_WT_DELIM << SCOP_REJECT << SCOP_WT_DELIM << SCOP_VERIFY;

    SidechainDB scdbPosition;

    uint256 hashWTTest1 = GetRandHash();
    uint256 hashWTTest2 = GetRandHash();
    uint256 hashWTTest3 = GetRandHash();

    scdbPosition.Update(SIDECHAIN_TEST, 0, 1, hashWTTest1);
    scdbPosition.Update(SIDECHAIN_TEST, 0, 0, hashWTTest2);
    scdbPosition.Update(SIDECHAIN_TEST, 0, 0, hashWTTest3);

    uint256 hashWTHivemind1 = GetRandHash();
    uint256 hashWTHivemind2 = GetRandHash();
    uint256 hashWTHivemind3 = GetRandHash();

    scdbPosition.Update(SIDECHAIN_HIVEMIND, 0, 0, hashWTHivemind1);
    scdbPosition.Update(SIDECHAIN_HIVEMIND, 0, 1, hashWTHivemind2);
    scdbPosition.Update(SIDECHAIN_HIVEMIND, 0, 0, hashWTHivemind3);

    uint256 hashWTWimble1 = GetRandHash();
    uint256 hashWTWimble2 = GetRandHash();
    uint256 hashWTWimble3 = GetRandHash();

    scdbPosition.Update(SIDECHAIN_WIMBLE, 0, 0, hashWTWimble1);
    scdbPosition.Update(SIDECHAIN_WIMBLE, 0, 0, hashWTWimble2);
    scdbPosition.Update(SIDECHAIN_WIMBLE, 0, 1, hashWTWimble3);

    BOOST_CHECK(scriptWTPositionExpected == scdbPosition.CreateStateScript(sidechainTest.GetTau() - 1));
}

BOOST_AUTO_TEST_SUITE_END()
