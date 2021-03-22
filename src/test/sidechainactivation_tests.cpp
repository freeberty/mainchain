// Copyright (c) 2021 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include "sidechain.h"
#include "sidechaindb.h"
#include "validation.h"

#include "test/test_drivenet.h"

#include <boost/test/unit_test.hpp>


BOOST_FIXTURE_TEST_SUITE(sidechainactivation_tests, TestingSetup)

BOOST_AUTO_TEST_CASE(proposal_single)
{
    // Test adding one proposal to scdbTest
    SidechainDB scdbTest;

    // Create sidechain proposal
    Sidechain proposal;
    proposal.nSidechain = 0;
    proposal.title = "test";
    proposal.description = "description";
    proposal.sidechainKeyID = "80dca759b4ff2c9e9b65ec790703ad09fba844cd";
    proposal.sidechainHex = "76a91480dca759b4ff2c9e9b65ec790703ad09fba844cd88ac";
    proposal.sidechainPriv = "5Jf2vbdzdCccKApCrjmwL5EFc4f1cUm5Ah4L4LGimEuFyqYpa9r";
    proposal.hashID1 = uint256S("b55d224f1fda033d930c92b1b40871f209387355557dd5e0d2b5dd9bb813c33f");
    proposal.hashID2 = uint160S("31d98584f3c570961359c308619f5cf2e9178482");

    // Create transaction output with sidechain proposal
    CTxOut out;
    out.scriptPubKey = proposal.GetScript();
    out.nValue = 50 * CENT;

    BOOST_CHECK(out.scriptPubKey.IsSidechainProposalCommit());

    // Updated scdbTest to add the proposal
    uint256 hash = GetRandHash();
    scdbTest.Update(0, hash, uint256(), std::vector<CTxOut>{out});

    std::vector<SidechainActivationStatus> vActivation;
    vActivation = scdbTest.GetSidechainActivationStatus();

    // Verify scdbTest is tracking the proposal
    BOOST_CHECK((vActivation.size() == 1) && (vActivation.front().proposal.GetHash() == proposal.GetHash()));
}

BOOST_AUTO_TEST_CASE(proposal_multiple)
{
    // Test adding multiple proposal to scdbTest
    SidechainDB scdbTest;

    // Create sidechain proposal
    Sidechain proposal1;
    proposal1.nSidechain = 0;
    proposal1.title = "test1";
    proposal1.description = "description";
    proposal1.sidechainKeyID = "80dca759b4ff2c9e9b65ec790703ad09fba844cd";
    proposal1.sidechainHex = "76a91480dca759b4ff2c9e9b65ec790703ad09fba844cd88ac";
    proposal1.sidechainPriv = "5Jf2vbdzdCccKApCrjmwL5EFc4f1cUm5Ah4L4LGimEuFyqYpa9r";
    proposal1.hashID1 = uint256S("b55d224f1fda033d930c92b1b40871f209387355557dd5e0d2b5dd9bb813c33f");
    proposal1.hashID2 = uint160S("31d98584f3c570961359c308619f5cf2e9178482");

    // Create transaction output with sidechain proposal
    CTxOut out;
    out.scriptPubKey = proposal1.GetScript();
    out.nValue = 50 * CENT;

    BOOST_CHECK(out.scriptPubKey.IsSidechainProposalCommit());

    // Update scdbTest to add the proposal
    uint256 hash1 = GetRandHash();
    scdbTest.Update(0, hash1, uint256(), std::vector<CTxOut>{out});

    // Create sidechain proposal
    Sidechain proposal2;
    proposal2.nSidechain = 1;
    proposal2.title = "test2";
    proposal2.description = "description";
    proposal2.sidechainKeyID = "80dca759b4ff2c9e9b65ec790703ad09fba844cd";
    proposal2.sidechainHex = "76a91480dca759b4ff2c9e9b65ec790703ad09fba844cd88ac";
    proposal2.sidechainPriv = "5Jf2vbdzdCccKApCrjmwL5EFc4f1cUm5Ah4L4LGimEuFyqYpa9r";
    proposal2.hashID1 = uint256S("b55d224f1fda033d930c92b1b40871f209387355557dd5e0d2b5dd9bb813c33f");
    proposal2.hashID2 = uint160S("31d98584f3c570961359c308619f5cf2e9178482");

    // Create transaction output with sidechain proposal
    CTxOut out2;
    out2.scriptPubKey = proposal2.GetScript();
    out2.nValue = 50 * CENT;

    BOOST_CHECK(out2.scriptPubKey.IsSidechainProposalCommit());

    // Update scdbTest to add the proposal
    scdbTest.Update(1, GetRandHash(), hash1, std::vector<CTxOut>{out2});

    std::vector<SidechainActivationStatus> vActivation;
    vActivation = scdbTest.GetSidechainActivationStatus();

    // Verify scdbTest is tracking the proposals
    BOOST_CHECK((vActivation.size() == 2) &&
            (vActivation.front().proposal.GetHash() == proposal1.GetHash()) &&
            (vActivation.back().proposal.GetHash() == proposal2.GetHash()));
}

BOOST_AUTO_TEST_CASE(proposal_limit)
{
    // Test adding the maximum number of sidechain proposals
    // TODO
}

BOOST_AUTO_TEST_CASE(proposal_perblock_limit)
{
    // Make sure multiple sidechain proposals in one block will be rejected.
    SidechainDB scdbTest;

    // Creat sidechain proposal
    Sidechain proposal1;
    proposal1.nSidechain = 0;
    proposal1.title = "test1";
    proposal1.description = "description";
    proposal1.sidechainKeyID = "80dca759b4ff2c9e9b65ec790703ad09fba844cd";
    proposal1.sidechainHex = "76a91480dca759b4ff2c9e9b65ec790703ad09fba844cd88ac";
    proposal1.sidechainPriv = "5Jf2vbdzdCccKApCrjmwL5EFc4f1cUm5Ah4L4LGimEuFyqYpa9r";
    proposal1.hashID1 = uint256S("b55d224f1fda033d930c92b1b40871f209387355557dd5e0d2b5dd9bb813c33f");
    proposal1.hashID2 = uint160S("31d98584f3c570961359c308619f5cf2e9178482");

    // Create transaction output with sidechain proposal
    CTxOut out;
    out.scriptPubKey = proposal1.GetScript();
    out.nValue = 50 * CENT;

    BOOST_CHECK(out.scriptPubKey.IsSidechainProposalCommit());

    // Create sidechain proposal
    Sidechain proposal2;
    proposal2.nSidechain = 1;
    proposal2.title = "test2";
    proposal2.description = "description";
    proposal2.sidechainKeyID = "80dca759b4ff2c9e9b65ec790703ad09fba844cd";
    proposal2.sidechainHex = "76a91480dca759b4ff2c9e9b65ec790703ad09fba844cd88ac";
    proposal2.sidechainPriv = "5Jf2vbdzdCccKApCrjmwL5EFc4f1cUm5Ah4L4LGimEuFyqYpa9r";
    proposal2.hashID1 = uint256S("b55d224f1fda033d930c92b1b40871f209387355557dd5e0d2b5dd9bb813c33f");
    proposal2.hashID2 = uint160S("31d98584f3c570961359c308619f5cf2e9178482");

    // Create transaction output with sidechain proposal
    CTxOut out2;
    out2.scriptPubKey = proposal2.GetScript();
    out2.nValue = 50 * CENT;

    BOOST_CHECK(out2.scriptPubKey.IsSidechainProposalCommit());

    // Update scdbTest to add the proposal
    scdbTest.Update(0, GetRandHash(), uint256(), std::vector<CTxOut>{out, out2});

    std::vector<SidechainActivationStatus> vActivation;
    vActivation = scdbTest.GetSidechainActivationStatus();

    // Nothing should have been added
    BOOST_CHECK(vActivation.empty());
}

BOOST_AUTO_TEST_CASE(activate_single)
{
    SidechainDB scdbTest;

    // Creat sidechain proposal
    Sidechain proposal;
    proposal.nSidechain = 0;
    proposal.nVersion = 0;
    proposal.nVersion = 0;
    proposal.title = "test";
    proposal.description = "description";
    proposal.sidechainKeyID = "80dca759b4ff2c9e9b65ec790703ad09fba844cd";
    proposal.sidechainHex = "76a91480dca759b4ff2c9e9b65ec790703ad09fba844cd88ac";
    proposal.sidechainPriv = "5Jf2vbdzdCccKApCrjmwL5EFc4f1cUm5Ah4L4LGimEuFyqYpa9r";
    proposal.hashID1 = uint256S("b55d224f1fda033d930c92b1b40871f209387355557dd5e0d2b5dd9bb813c33f");
    proposal.hashID2 = uint160S("31d98584f3c570961359c308619f5cf2e9178482");


    BOOST_CHECK(scdbTest.GetActiveSidechainCount() == 0);
    BOOST_CHECK(ActivateSidechain(scdbTest, proposal, 0));
    BOOST_CHECK(scdbTest.GetActiveSidechainCount() == 1);
}

BOOST_AUTO_TEST_CASE(activate_multiple)
{
    SidechainDB scdbTest;

    Sidechain proposal1;
    proposal1.nSidechain = 0;
    proposal1.nVersion = 0;
    proposal1.title = "sidechain1";
    proposal1.description = "description";
    proposal1.sidechainKeyID = "80dca759b4ff2c9e9b65ec790703ad09fba844cd";
    proposal1.sidechainHex = "76a91480dca759b4ff2c9e9b65ec790703ad09fba844cd88ac";
    proposal1.sidechainPriv = "5Jf2vbdzdCccKApCrjmwL5EFc4f1cUm5Ah4L4LGimEuFyqYpa9r";
    proposal1.hashID1 = uint256S("b55d224f1fda033d930c92b1b40871f209387355557dd5e0d2b5dd9bb813c33f");
    proposal1.hashID2 = uint160S("31d98584f3c570961359c308619f5cf2e9178482");


    BOOST_CHECK(scdbTest.GetActiveSidechainCount() == 0);
    BOOST_CHECK(ActivateSidechain(scdbTest, proposal1, 0));
    BOOST_CHECK(scdbTest.GetActiveSidechainCount() == 1);

    // Proposal for a second sidechain
    Sidechain proposal2;
    proposal2.nSidechain = 1;
    proposal2.nVersion = 0;
    proposal2.title = "sidechain2";
    proposal2.description = "test";
    proposal2.sidechainKeyID = "c37afd89181060fa69deb3b26a0b95c02986ec78";
    proposal2.sidechainHex = "76a91480dca759b4ff2c9e9b65ec790703ad09fba844cd88ac"; // TODO
    proposal2.sidechainPriv = "5Jf2vbdzdCccKApCrjmwL5EFc4f1cUm5Ah4L4LGimEuFyqYpa9r"; // TODO
    proposal2.hashID1 = GetRandHash();
    proposal2.hashID2 = uint160S("31d98584f3c570961359c308619f5cf2e9178482");


    BOOST_CHECK(ActivateSidechain(scdbTest, proposal2, 0));
    BOOST_CHECK(scdbTest.GetActiveSidechainCount() == 2);

    // Copy sidechain 2
    Sidechain proposal3 = proposal2;
    proposal3.nSidechain = 2;
    proposal3.title = "abc";

    // TODO
    // This should fail because we cannot have sidechains that share params
    BOOST_CHECK(ActivateSidechain(scdbTest, proposal3, 0));
    BOOST_CHECK(scdbTest.GetActiveSidechainCount() == 3);
}

BOOST_AUTO_TEST_CASE(activate_max)
{
    SidechainDB scdbTest;

    BOOST_CHECK(scdbTest.GetActiveSidechainCount() == 0);

    Sidechain proposal;
    proposal.nVersion = 0;
    proposal.title = "sidechain";
    proposal.description = "test";
    proposal.sidechainKeyID = "c37afd89181060fa69deb3b26a0b95c02986ec78";
    proposal.sidechainHex = "76a91480dca759b4ff2c9e9b65ec790703ad09fba844cd88ac"; // TODO
    proposal.sidechainPriv = "5Jf2vbdzdCccKApCrjmwL5EFc4f1cUm5Ah4L4LGimEuFyqYpa9r"; // TODO
    proposal.hashID1 = GetRandHash();
    proposal.hashID2 = uint160S("31d98584f3c570961359c308619f5cf2e9178482");

    unsigned int nSidechains = 0;
    for (int i = 0; i < SIDECHAIN_ACTIVATION_MAX_ACTIVE; i++) {
        proposal.nSidechain = i;
        proposal.title = "sidechain" + std::to_string(i);

        BOOST_CHECK(ActivateSidechain(scdbTest, proposal, 0));

        nSidechains++;

        BOOST_CHECK(scdbTest.GetActiveSidechainCount() == nSidechains);
    }

    // Check that the maximum number have been activated
    BOOST_CHECK(scdbTest.GetActiveSidechainCount() == SIDECHAIN_ACTIVATION_MAX_ACTIVE);

    std::vector<Sidechain> vSidechain = scdbTest.GetSidechains();
    BOOST_CHECK(vSidechain.size() == SIDECHAIN_ACTIVATION_MAX_ACTIVE);

    // Check sidechain numbers and active status
    for (size_t i = 0; i < vSidechain.size(); i++) {
        BOOST_CHECK(vSidechain[i].fActive == true);
        BOOST_CHECK(vSidechain[i].nSidechain == i);
    }
}

BOOST_AUTO_TEST_CASE(activate_fail)
{
    // Test adding one proposal to scdbTest and fail to activate it
    SidechainDB scdbTest;

    // Creat sidechain proposal
    Sidechain proposal;
    proposal.nSidechain = 0;
    proposal.nVersion = 0;
    proposal.title = "test";
    proposal.description = "description";
    proposal.sidechainKeyID = "80dca759b4ff2c9e9b65ec790703ad09fba844cd";
    proposal.sidechainHex = "76a91480dca759b4ff2c9e9b65ec790703ad09fba844cd88ac";
    proposal.sidechainPriv = "5Jf2vbdzdCccKApCrjmwL5EFc4f1cUm5Ah4L4LGimEuFyqYpa9r";
    proposal.hashID1 = uint256S("b55d224f1fda033d930c92b1b40871f209387355557dd5e0d2b5dd9bb813c33f");
    proposal.hashID2 = uint160S("31d98584f3c570961359c308619f5cf2e9178482");

    // Create transaction output with sidechain proposal
    CTxOut out;
    out.scriptPubKey = proposal.GetScript();
    out.nValue = 50 * CENT;

    BOOST_CHECK(out.scriptPubKey.IsSidechainProposalCommit());

    uint256 hash1 = GetRandHash();
    scdbTest.Update(0, hash1, uint256(), std::vector<CTxOut>{out});

    std::vector<SidechainActivationStatus> vActivation;
    vActivation = scdbTest.GetSidechainActivationStatus();

    BOOST_CHECK((vActivation.size() == 1) && (vActivation.front().proposal.GetHash() == proposal.GetHash()));

    // Use the function from validation to generate the commit, and then
    // copy it from the block.
    CBlock block;
    CMutableTransaction mtx;
    mtx.vin.resize(1);
    mtx.vin[0].prevout.SetNull();
    block.vtx.push_back(MakeTransactionRef(std::move(mtx)));
    GenerateSidechainActivationCommitment(block, proposal.GetHash(), Params().GetConsensus());

    // Add votes until the sidechain is half way activated
    for (int i = 1; i <= SIDECHAIN_ACTIVATION_PERIOD / 2; i++) {
        uint256 hash2 = GetRandHash();
        scdbTest.Update(i, hash2, hash1, block.vtx.front()->vout);
        hash1 = hash2;
    }

    // Check activation status
    // Sidechain should still be in activation cache
    // Sidechain should not be in ValidSidechains
    std::vector<Sidechain> vSidechain = scdbTest.GetActiveSidechains();

    BOOST_CHECK(vSidechain.empty());
}

BOOST_AUTO_TEST_CASE(activate_remove_failed)
{
    // Test that sidechains which have no chance of success (based on their
    // rejection count) are pruned from the activation cache.
    SidechainDB scdbTest;

    // Creat sidechain proposal
    Sidechain proposal;
    proposal.nSidechain = 0;
    proposal.nVersion = 0;
    proposal.title = "test";
    proposal.description = "description";
    proposal.sidechainKeyID = "80dca759b4ff2c9e9b65ec790703ad09fba844cd";
    proposal.sidechainHex = "76a91480dca759b4ff2c9e9b65ec790703ad09fba844cd88ac";
    proposal.sidechainPriv = "5Jf2vbdzdCccKApCrjmwL5EFc4f1cUm5Ah4L4LGimEuFyqYpa9r";
    proposal.hashID1 = uint256S("b55d224f1fda033d930c92b1b40871f209387355557dd5e0d2b5dd9bb813c33f");
    proposal.hashID2 = uint160S("31d98584f3c570961359c308619f5cf2e9178482");

    // Create transaction output with sidechain proposal
    CTxOut out;
    out.scriptPubKey = proposal.GetScript();
    out.nValue = 50 * CENT;

    BOOST_CHECK(out.scriptPubKey.IsSidechainProposalCommit());

    uint256 hash1 = GetRandHash();
    scdbTest.Update(0, hash1, uint256(), std::vector<CTxOut>{out});

    std::vector<SidechainActivationStatus> vActivation;
    vActivation = scdbTest.GetSidechainActivationStatus();

    BOOST_CHECK((vActivation.size() == 1) && (vActivation.front().proposal.GetHash() == proposal.GetHash()));

    // Pass coinbase without sidechain activation commit into scdbTest enough times
    // that the proposal is rejected and pruned.
    out.scriptPubKey = OP_TRUE;
    out.nValue = 50 * CENT;

    for (int i = 1; i <= SIDECHAIN_ACTIVATION_MAX_FAILURES + 1; i++) {
        vActivation = scdbTest.GetSidechainActivationStatus();
        uint256 hash2 = GetRandHash();
        scdbTest.Update(i, hash2, hash1, std::vector<CTxOut>{out});
        hash1 = hash2;
    }

    // Check activation status
    // Sidechain should have been pruned from activation cache
    // Sidechain should not be in ValidSidechains
    std::vector<SidechainActivationStatus> vActivationFinal;
    vActivationFinal = scdbTest.GetSidechainActivationStatus();
    BOOST_CHECK(vActivationFinal.empty());

    std::vector<Sidechain> vSidechain = scdbTest.GetActiveSidechains();
    BOOST_CHECK(vSidechain.empty());
}

BOOST_AUTO_TEST_CASE(activate_duplicate)
{
    // Test proposing a sidechain that is an exact duplicate of a sidechain
    // that has already activated. Should be rejected.
}

BOOST_AUTO_TEST_CASE(none_active)
{
    // Test that when no sidechains have been activated, the sidechain list
    // lists all of them with inactive status and the correct sidechain number.
    SidechainDB scdbTest;

    // No sidechains should be active
    BOOST_CHECK(scdbTest.GetActiveSidechainCount() == 0);

    std::vector<Sidechain> vActive = scdbTest.GetActiveSidechains();
    BOOST_CHECK(vActive.empty());

    std::vector<Sidechain> vSidechain = scdbTest.GetSidechains();
    BOOST_CHECK(vSidechain.size() == SIDECHAIN_ACTIVATION_MAX_ACTIVE);

    // Check sidechain numbers and active status
    for (size_t i = 0; i < vSidechain.size(); i++) {
        BOOST_CHECK(vSidechain[i].fActive == false);
        BOOST_CHECK(vSidechain[i].nSidechain == i);
    }
}

BOOST_AUTO_TEST_CASE(max_active_reverse)
{
    // Test activating the maximum number of sidechains but in reverse order
    // from sidechain #255 to #0.
    SidechainDB scdbTest;

    BOOST_CHECK(scdbTest.GetActiveSidechainCount() == 0);

    Sidechain proposal;
    proposal.nVersion = 0;
    proposal.description = "test";
    proposal.sidechainKeyID = "c37afd89181060fa69deb3b26a0b95c02986ec78";
    proposal.sidechainHex = "76a91480dca759b4ff2c9e9b65ec790703ad09fba844cd88ac";
    proposal.sidechainPriv = "5Jf2vbdzdCccKApCrjmwL5EFc4f1cUm5Ah4L4LGimEuFyqYpa9r";
    proposal.hashID1 = GetRandHash();
    proposal.hashID2 = uint160S("31d98584f3c570961359c308619f5cf2e9178482");

    unsigned int nSidechains = 0;
    for (int i = SIDECHAIN_ACTIVATION_MAX_ACTIVE - 1; i >= 0; i--) {
        proposal.nSidechain = i;
        proposal.title = "sidechain" + std::to_string(i);

        BOOST_CHECK(ActivateSidechain(scdbTest, proposal, 0));

        nSidechains++;

        BOOST_CHECK(scdbTest.GetActiveSidechainCount() == nSidechains);
    }

    // Check that the maximum number have been activated
    BOOST_CHECK(scdbTest.GetActiveSidechainCount() == SIDECHAIN_ACTIVATION_MAX_ACTIVE);

    std::vector<Sidechain> vSidechain = scdbTest.GetSidechains();
    BOOST_CHECK(vSidechain.size() == SIDECHAIN_ACTIVATION_MAX_ACTIVE);

    // Check sidechain numbers and active status
    for (size_t i = 0; i < vSidechain.size(); i++) {
        BOOST_CHECK(vSidechain[i].fActive == true);
        BOOST_CHECK(vSidechain[i].nSidechain == i);
    }
}

BOOST_AUTO_TEST_CASE(every_other_active)
{
    // Test activating half of the maximum number of sidechains skipping one
    // sidechain slot between each activated sidechain.
    SidechainDB scdbTest;

    BOOST_CHECK(scdbTest.GetActiveSidechainCount() == 0);

    // TODO sidechains with the same key and IDs should be rejected
    Sidechain proposal;
    proposal.nVersion = 0;
    proposal.description = "test";
    proposal.sidechainKeyID = "c37afd89181060fa69deb3b26a0b95c02986ec78";
    proposal.sidechainHex = "76a91480dca759b4ff2c9e9b65ec790703ad09fba844cd88ac";
    proposal.sidechainPriv = "5Jf2vbdzdCccKApCrjmwL5EFc4f1cUm5Ah4L4LGimEuFyqYpa9r";
    proposal.hashID1 = GetRandHash();
    proposal.hashID2 = uint160S("31d98584f3c570961359c308619f5cf2e9178482");

    unsigned int nSidechains = 0;
    for (int i = 0; i < SIDECHAIN_ACTIVATION_MAX_ACTIVE; i++) {
        if (i % 2 == 0)
            continue;

        proposal.nSidechain = i;
        proposal.title = "sidechain" + std::to_string(i);

        BOOST_CHECK(ActivateSidechain(scdbTest, proposal, 0));

        nSidechains++;

        BOOST_CHECK(scdbTest.GetActiveSidechainCount() == nSidechains);
    }

    // Check that half of the maximum number have been activated
    BOOST_CHECK(scdbTest.GetActiveSidechainCount() == SIDECHAIN_ACTIVATION_MAX_ACTIVE / 2);

    std::vector<Sidechain> vSidechain = scdbTest.GetSidechains();
    BOOST_CHECK(vSidechain.size() == SIDECHAIN_ACTIVATION_MAX_ACTIVE);

    // Check sidechain numbers and active status
    for (size_t i = 0; i < vSidechain.size(); i++) {
        if (i % 2 == 0)
            BOOST_CHECK(vSidechain[i].fActive == false);
        else
            BOOST_CHECK(vSidechain[i].fActive == true);

        BOOST_CHECK(vSidechain[i].nSidechain == i);
    }
}

BOOST_AUTO_TEST_CASE(replace_sidechain)
{
    SidechainDB scdbTest;

    // Activate first sidechain

    // Creat sidechain proposal
    Sidechain proposal;
    proposal.nSidechain = 0;
    proposal.nVersion = 0;
    proposal.nVersion = 0;
    proposal.title = "test";
    proposal.description = "description";
    proposal.sidechainKeyID = "80dca759b4ff2c9e9b65ec790703ad09fba844cd";
    proposal.sidechainHex = "76a91480dca759b4ff2c9e9b65ec790703ad09fba844cd88ac";
    proposal.sidechainPriv = "5Jf2vbdzdCccKApCrjmwL5EFc4f1cUm5Ah4L4LGimEuFyqYpa9r";
    proposal.hashID1 = uint256S("b55d224f1fda033d930c92b1b40871f209387355557dd5e0d2b5dd9bb813c33f");
    proposal.hashID2 = uint160S("31d98584f3c570961359c308619f5cf2e9178482");


    BOOST_CHECK(scdbTest.GetActiveSidechainCount() == 0);
    BOOST_CHECK(ActivateSidechain(scdbTest, proposal, 0));
    BOOST_CHECK(scdbTest.GetActiveSidechainCount() == 1);

    // Create replacement sidechain proposal

    // Creat sidechain proposal
    Sidechain proposal2;
    proposal2.nSidechain = 0;
    proposal2.nVersion = 0;
    proposal2.nVersion = 0;
    proposal2.title = "replacement";
    proposal2.description = "description";
    proposal2.sidechainKeyID = "80dca759b4ff2c9e9b65ec790703ad09fba844cd";
    proposal2.sidechainHex = "76a91480dca759b4ff2c9e9b65ec790703ad09fba844cd88ac";
    proposal2.sidechainPriv = "5Jf2vbdzdCccKApCrjmwL5EFc4f1cUm5Ah4L4LGimEuFyqYpa9r";
    proposal2.hashID1 = uint256S("b55d224f1fda033d930c92b1b40871f209387355557dd5e0d2b5dd9bb813c33f");
    proposal2.hashID2 = uint160S("31d98584f3c570961359c308619f5cf2e9178482");

    // Create transaction output with sidechain proposal
    CTxOut out;
    out.scriptPubKey = proposal2.GetScript();
    out.nValue = 50 * CENT;

    BOOST_CHECK(out.scriptPubKey.IsSidechainProposalCommit());

    uint256 hash1 = GetRandHash();
    BOOST_CHECK(scdbTest.Update(0, hash1, scdbTest.GetHashBlockLastSeen(), std::vector<CTxOut>{out}));

    std::vector<SidechainActivationStatus> vActivation;
    vActivation = scdbTest.GetSidechainActivationStatus();

    BOOST_CHECK((vActivation.size() == 1) && (vActivation.front().proposal.GetHash() == proposal2.GetHash()));

    // Generate a sidechain activation commit for the replacement
    CBlock block;
    CMutableTransaction mtx;
    mtx.vin.resize(1);
    mtx.vin[0].prevout.SetNull();
    block.vtx.push_back(MakeTransactionRef(std::move(mtx)));
    GenerateSidechainActivationCommitment(block, proposal2.GetHash(), Params().GetConsensus());

    // Add the requirement to replace
    for (int i = 1; i <= SIDECHAIN_REPLACEMENT_PERIOD; i++) {
        uint256 hashNew = GetRandHash();
        BOOST_CHECK(scdbTest.Update(i, hashNew, scdbTest.GetHashBlockLastSeen(), block.vtx.front()->vout));
    }

    // Check that the proposal has half of the replacement requirement
    vActivation = scdbTest.GetSidechainActivationStatus();

    // Check activation status
    // replacement should have been pruned from activation cache
    std::vector<SidechainActivationStatus> vActivationFinal;
    vActivationFinal = scdbTest.GetSidechainActivationStatus();
    BOOST_CHECK(vActivationFinal.empty());

    // Sidechain 0 should now be "replacement"
    // Check that "replacement" sidechain was activated
    std::vector<Sidechain> vSidechain = scdbTest.GetSidechains();
    BOOST_CHECK(vSidechain[0].title == "replacement");
}

BOOST_AUTO_TEST_CASE(replace_sidechain_fail)
{
    SidechainDB scdbTest;

    // Activate first sidechain

    // Creat sidechain proposal
    Sidechain proposal;
    proposal.nSidechain = 0;
    proposal.nVersion = 0;
    proposal.nVersion = 0;
    proposal.title = "test";
    proposal.description = "description";
    proposal.sidechainKeyID = "80dca759b4ff2c9e9b65ec790703ad09fba844cd";
    proposal.sidechainHex = "76a91480dca759b4ff2c9e9b65ec790703ad09fba844cd88ac";
    proposal.sidechainPriv = "5Jf2vbdzdCccKApCrjmwL5EFc4f1cUm5Ah4L4LGimEuFyqYpa9r";
    proposal.hashID1 = uint256S("b55d224f1fda033d930c92b1b40871f209387355557dd5e0d2b5dd9bb813c33f");
    proposal.hashID2 = uint160S("31d98584f3c570961359c308619f5cf2e9178482");


    BOOST_CHECK(scdbTest.GetActiveSidechainCount() == 0);
    BOOST_CHECK(ActivateSidechain(scdbTest, proposal, 0));
    BOOST_CHECK(scdbTest.GetActiveSidechainCount() == 1);

    // Create replacement sidechain proposal

    // Creat sidechain proposal
    Sidechain proposal2;
    proposal2.nSidechain = 0;
    proposal2.nVersion = 0;
    proposal2.nVersion = 0;
    proposal2.title = "replacement";
    proposal2.description = "description";
    proposal2.sidechainKeyID = "80dca759b4ff2c9e9b65ec790703ad09fba844cd";
    proposal2.sidechainHex = "76a91480dca759b4ff2c9e9b65ec790703ad09fba844cd88ac";
    proposal2.sidechainPriv = "5Jf2vbdzdCccKApCrjmwL5EFc4f1cUm5Ah4L4LGimEuFyqYpa9r";
    proposal2.hashID1 = uint256S("b55d224f1fda033d930c92b1b40871f209387355557dd5e0d2b5dd9bb813c33f");
    proposal2.hashID2 = uint160S("31d98584f3c570961359c308619f5cf2e9178482");

    // Create transaction output with sidechain proposal
    CTxOut out;
    out.scriptPubKey = proposal2.GetScript();
    out.nValue = 50 * CENT;

    BOOST_CHECK(out.scriptPubKey.IsSidechainProposalCommit());

    uint256 hash1 = GetRandHash();
    BOOST_CHECK(scdbTest.Update(0, hash1, scdbTest.GetHashBlockLastSeen(), std::vector<CTxOut>{out}));

    std::vector<SidechainActivationStatus> vActivation;
    vActivation = scdbTest.GetSidechainActivationStatus();

    BOOST_CHECK((vActivation.size() == 1) && (vActivation.front().proposal.GetHash() == proposal2.GetHash()));

    // Generate a sidechain activation commit for the replacement
    CBlock block;
    CMutableTransaction mtx;
    mtx.vin.resize(1);
    mtx.vin[0].prevout.SetNull();
    block.vtx.push_back(MakeTransactionRef(std::move(mtx)));
    GenerateSidechainActivationCommitment(block, proposal2.GetHash(), Params().GetConsensus());

    // Add half of the requirement to replace
    for (int i = 1; i <= SIDECHAIN_REPLACEMENT_PERIOD / 2; i++) {
        uint256 hashNew = GetRandHash();
        BOOST_CHECK(scdbTest.Update(i, hashNew, scdbTest.GetHashBlockLastSeen(), block.vtx.front()->vout));
    }

    // Generate blocks without activation commitments to make proposal fail

    out.scriptPubKey = OP_TRUE;
    out.nValue = 50 * CENT;

    for (int i = 1; i <= SIDECHAIN_ACTIVATION_MAX_FAILURES + 1; i++) {
        uint256 hash2 = GetRandHash();
        BOOST_CHECK(scdbTest.Update(i, hash2, scdbTest.GetHashBlockLastSeen(), std::vector<CTxOut>{out}));
    }

    // Check activation status
    // replacement should have been pruned from activation cache
    // replacement should not be active
    std::vector<SidechainActivationStatus> vActivationFinal;
    vActivationFinal = scdbTest.GetSidechainActivationStatus();
    BOOST_CHECK(vActivationFinal.empty());

    // Check that "replacement" sidechain was not activated
    // Sidechain 0 should still be "test"
    std::vector<Sidechain> vSidechain = scdbTest.GetSidechains();
    BOOST_CHECK(vSidechain[0].title == "test");
}

BOOST_AUTO_TEST_SUITE_END()
