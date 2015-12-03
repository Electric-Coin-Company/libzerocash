/** @file
 *****************************************************************************

 A test for Zerocash.

 *****************************************************************************
 * @author     This file is part of libzerocash, developed by the Zerocash
 *             project and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#include <stdlib.h>
#include <iostream>

#define BOOST_TEST_MODULE zerocashTest
#include <boost/test/included/unit_test.hpp>

#include "timer.h"

#include "libzerocash/Zerocash.h"
#include "libzerocash/ZerocashParams.h"
#include "libzerocash/Address.h"
#include "libzerocash/CoinCommitment.h"
#include "libzerocash/Coin.h"
#include "libzerocash/IncrementalMerkleTree.h"
#include "libzerocash/MerkleTree.h"
#include "libzerocash/MintTransaction.h"
#include "libzerocash/PourTransaction.h"
#include "libzerocash/PourInput.h"
#include "libzerocash/PourOutput.h"
#include "libzerocash/utils/util.h"

using namespace std;
using namespace libsnark;

vector<bool> convertIntToVector(uint64_t val) {
	vector<bool> ret;

	for(unsigned int i = 0; i < sizeof(val) * 8; ++i, val >>= 1) {
		ret.push_back(val & 0x01);
	}

	reverse(ret.begin(), ret.end());
	return ret;
}

#define TEST_TREE_DEPTH 4

BOOST_AUTO_TEST_CASE( AddressTest ) {
    cout << "\nADDRESS TEST\n" << endl;

    libzerocash::timer_start("Address");
    libzerocash::Address newAddress;
    libzerocash::timer_stop("Address");

    cout << "Successfully created an address.\n" << endl;

    CDataStream serializedAddress(SER_NETWORK, 7002);
    serializedAddress << newAddress;

    cout << "Successfully serialized an address.\n" << endl;

    libzerocash::Address addressNew;
    serializedAddress >> addressNew;
    cout << "Successfully deserialized an address.\n" << endl;

    libzerocash::PublicAddress pubAddress = newAddress.getPublicAddress();

    CDataStream serializedPubAddress(SER_NETWORK, 7002);
    serializedPubAddress << pubAddress;

    cout << "Successfully serialized a public address.\n" << endl;

    libzerocash::PublicAddress pubAddressNew;
    serializedPubAddress >> pubAddressNew;
    cout << "Successfully deserialized a public address.\n" << endl;

    bool result = ((newAddress == addressNew) && (pubAddress == pubAddressNew));

    BOOST_CHECK(result);
}

BOOST_AUTO_TEST_CASE( SaveAndLoadKeysFromFiles ) {
    cout << "\nSaveAndLoadKeysFromFiles TEST\n" << endl;

    cout << "Creating Params...\n" << endl;

    libzerocash::timer_start("Param Generation");
    auto keypair = libzerocash::ZerocashParams::GenerateNewKeyPair(TEST_TREE_DEPTH);
    libzerocash::ZerocashParams p(
        TEST_TREE_DEPTH,
        &keypair
    );
    libzerocash::timer_stop("Param Generation");
    print_mem("after param generation");

    cout << "Successfully created Params.\n" << endl;

    std::string vk_path = "./zerocashTest-verification-key";
    std::string pk_path = "./zerocashTest-proving-key";

    libzerocash::timer_start("Saving Proving Key");

    libzerocash::ZerocashParams::SaveProvingKeyToFile(
        &p.getProvingKey(),
        pk_path
    );

    libzerocash::timer_stop("Saving Proving Key");

    libzerocash::timer_start("Saving Verification Key");

    libzerocash::ZerocashParams::SaveVerificationKeyToFile(
        &p.getVerificationKey(),
        vk_path
    );

    libzerocash::timer_stop("Saving Verification Key");

    libzerocash::timer_start("Loading Proving Key");
    auto pk_loaded = libzerocash::ZerocashParams::LoadProvingKeyFromFile(pk_path, TEST_TREE_DEPTH);
    libzerocash::timer_stop("Loading Proving Key");

    libzerocash::timer_start("Loading Verification Key");
    auto vk_loaded = libzerocash::ZerocashParams::LoadVerificationKeyFromFile(vk_path, TEST_TREE_DEPTH);
    libzerocash::timer_stop("Loading Verification Key");

    cout << "Comparing Proving and Verification key.\n" << endl;

    if ( !( p.getProvingKey() == pk_loaded && p.getVerificationKey() == vk_loaded) ) {
        BOOST_ERROR("Proving and verification key are not equal.");
    }

    vector<libzerocash::Coin> coins(5);
    vector<libzerocash::Address> addrs(5);

    cout << "Creating Addresses and Coins...\n" << endl;
    for(size_t i = 0; i < coins.size(); i++) {
        addrs.at(i) = libzerocash::Address();
        coins.at(i) = libzerocash::Coin(addrs.at(i).getPublicAddress(), i);
    }
    cout << "Successfully created address and coins.\n" << endl;

    cout << "Creating a Mint Transaction...\n" << endl;
    libzerocash::MintTransaction minttx(coins.at(0));
    cout << "Successfully created a Mint Transaction.\n" << endl;

    cout << "Serializing a mint transaction...\n" << endl;
    CDataStream serializedMintTx(SER_NETWORK, 7002);
    serializedMintTx << minttx;
    cout << "Successfully serialized a mint transaction.\n" << endl;

    libzerocash::MintTransaction minttxNew;
    serializedMintTx >> minttxNew;
    cout << "Successfully deserialized a mint transaction.\n" << endl;

    cout << "Verifying a Mint Transaction...\n" << endl;
    bool minttx_res = minttxNew.verify();

    vector<std::vector<bool>> coinValues(5);
    vector<bool> temp_comVal(cm_size * 8);
    for(size_t i = 0; i < coinValues.size(); i++) {
        libzerocash::convertBytesVectorToVector(coins.at(i).getCoinCommitment().getCommitmentValue(), temp_comVal);
        coinValues.at(i) = temp_comVal;
    }

    cout << "Creating Merkle Tree...\n" << endl;
    libzerocash::MerkleTree merkleTree(coinValues, TEST_TREE_DEPTH);
    cout << "Successfully created Merkle Tree.\n" << endl;

    cout << "Creating Witness 1...\n" << endl;
    merkle_authentication_path witness_1(TEST_TREE_DEPTH);
    merkleTree.getWitness(coinValues.at(1), witness_1);
    cout << "Successfully created Witness 1.\n" << endl;

    cout << "Creating Witness 2...\n" << endl;
    merkle_authentication_path witness_2(TEST_TREE_DEPTH);
    merkleTree.getWitness(coinValues.at(3), witness_2);
    cout << "Successfully created Witness 2.\n" << endl;

    cout << "Creating coins to spend...\n" << endl;
    libzerocash::Address newAddress3;
    libzerocash::PublicAddress pubAddress3 = newAddress3.getPublicAddress();

    libzerocash::Address newAddress4;
    libzerocash::PublicAddress pubAddress4 = newAddress4.getPublicAddress();

    libzerocash::Coin c_1_new(pubAddress3, 2);
    libzerocash::Coin c_2_new(pubAddress4, 2);
    cout << "Successfully created coins to spend.\n" << endl;

    vector<bool> root_bv(root_size * 8);
    merkleTree.getRootValue(root_bv);
    vector<unsigned char> rt(root_size);
    libzerocash::convertVectorToBytesVector(root_bv, rt);


    vector<unsigned char> as(sig_pk_size, 'a');

    cout << "Creating a pour transaction...\n" << endl;
    libzerocash::PourTransaction pourtx(1, p,
    		rt,
    		coins.at(1), coins.at(3),
    		addrs.at(1), addrs.at(3),
                1, 3,
    		witness_1, witness_2,
    		pubAddress3, pubAddress4,
    		0,
            0,
    		as,
    		c_1_new, c_2_new);
    cout << "Successfully created a pour transaction.\n" << endl;

    cout << "Serializing a pour transaction...\n" << endl;
    CDataStream serializedPourTx(SER_NETWORK, 7002);
    serializedPourTx << pourtx;
    cout << "Successfully serialized a pour transaction.\n" << endl;

    libzerocash::PourTransaction pourtxNew;
    serializedPourTx >> pourtxNew;
    cout << "Successfully deserialized a pour transaction.\n" << endl;

	std::vector<unsigned char> pubkeyHash(sig_pk_size, 'a');

    cout << "Verifying a pour transaction...\n" << endl;
    bool pourtx_res = pourtxNew.verify(p, pubkeyHash, rt);

    BOOST_CHECK(minttx_res && pourtx_res);
}

BOOST_AUTO_TEST_CASE( PourInputOutputTest ) {
    // dummy input
    {
        libzerocash::PourInput input(TEST_TREE_DEPTH);

        BOOST_CHECK(input.old_coin.getValue() == 0);
        BOOST_CHECK(input.old_address.getPublicAddress() == input.old_coin.getPublicAddress());
    }

    // dummy output
    {
        libzerocash::PourOutput output(0);

        BOOST_CHECK(output.new_coin.getValue() == 0);
        BOOST_CHECK(output.to_address == output.new_coin.getPublicAddress());
    }
}

// testing with general situational setup
bool test_pour(libzerocash::ZerocashParams& p,
          uint64_t vpub_in,
          uint64_t vpub_out,
          std::vector<uint64_t> inputs, // values of the inputs (max 2)
          std::vector<uint64_t> outputs) // values of the outputs (max 2)
{
    using pour_input_state = std::tuple<libzerocash::Address, libzerocash::Coin, std::vector<bool>>;

    // Construct incremental merkle tree
    libzerocash::IncrementalMerkleTree merkleTree(TEST_TREE_DEPTH);

    // Dummy sig_pk
    vector<unsigned char> as(sig_pk_size, 'a');

    vector<libzerocash::PourInput> pour_inputs;
    vector<libzerocash::PourOutput> pour_outputs;

    vector<pour_input_state> input_state;

    for(std::vector<uint64_t>::iterator it = inputs.begin(); it != inputs.end(); ++it) {
        libzerocash::Address addr;
        libzerocash::Coin coin(addr.getPublicAddress(), *it);

        // commitment from coin
        std::vector<bool> commitment(cm_size * 8);
        libzerocash::convertBytesVectorToVector(coin.getCoinCommitment().getCommitmentValue(), commitment);

        // insert commitment into the merkle tree
        std::vector<bool> index;
        merkleTree.insertElement(commitment, index);

        // store the state temporarily
        input_state.push_back(std::make_tuple(addr, coin, index));
    }

    // compute the merkle root we will be working with
    vector<unsigned char> rt(root_size);
    {
        vector<bool> root_bv(root_size * 8);
        merkleTree.getRootValue(root_bv);
        libzerocash::convertVectorToBytesVector(root_bv, rt);
    }

    // get witnesses for all the input coins and construct the pours
    for(vector<pour_input_state>::iterator it = input_state.begin(); it != input_state.end(); ++it) {
        merkle_authentication_path path(TEST_TREE_DEPTH);

        auto index = std::get<2>(*it);
        merkleTree.getWitness(index, path);

        pour_inputs.push_back(libzerocash::PourInput(std::get<1>(*it), std::get<0>(*it), libzerocash::convertVectorToInt(index), path));
    }

    // construct dummy outputs with the given values
    for(vector<uint64_t>::iterator it = outputs.begin(); it != outputs.end(); ++it) {
        pour_outputs.push_back(libzerocash::PourOutput(*it));
    }

    try {
        libzerocash::PourTransaction pourtx(p, as, rt, pour_inputs, pour_outputs, vpub_in, vpub_out);

        assert(pourtx.verify(p, as, rt));

        return true;
    } catch(...) {
        return false;
    }
}

BOOST_AUTO_TEST_CASE( PourVpubInTest ) {
    auto keypair = libzerocash::ZerocashParams::GenerateNewKeyPair(TEST_TREE_DEPTH);
    libzerocash::ZerocashParams p(
        TEST_TREE_DEPTH,
        &keypair
    );

    // Things that should work..
    BOOST_CHECK(test_pour(p, 0, 0, {1}, {1}));
    BOOST_CHECK(test_pour(p, 0, 0, {2}, {1, 1}));
    BOOST_CHECK(test_pour(p, 0, 0, {2, 2}, {3, 1}));
    BOOST_CHECK(test_pour(p, 0, 1, {1}, {}));
    BOOST_CHECK(test_pour(p, 0, 1, {2}, {1}));
    BOOST_CHECK(test_pour(p, 0, 1, {2, 2}, {2, 1}));
    BOOST_CHECK(test_pour(p, 1, 0, {}, {1}));
    BOOST_CHECK(test_pour(p, 1, 0, {1}, {1, 1}));
    BOOST_CHECK(test_pour(p, 1, 0, {2, 2}, {2, 3}));

    // Things that should not work...
    BOOST_CHECK(!test_pour(p, 0, 1, {1}, {1}));
    BOOST_CHECK(!test_pour(p, 0, 1, {2}, {1, 1}));
    BOOST_CHECK(!test_pour(p, 0, 1, {2, 2}, {3, 1}));
    BOOST_CHECK(!test_pour(p, 0, 2, {1}, {}));
    BOOST_CHECK(!test_pour(p, 0, 2, {2}, {1}));
    BOOST_CHECK(!test_pour(p, 0, 2, {2, 2}, {2, 1}));
    BOOST_CHECK(!test_pour(p, 1, 1, {}, {1}));
    BOOST_CHECK(!test_pour(p, 1, 1, {1}, {1, 1}));
    BOOST_CHECK(!test_pour(p, 1, 1, {2, 2}, {2, 3}));

    BOOST_CHECK(!test_pour(p, 0, 0, {2, 2}, {2, 3}));
}

BOOST_AUTO_TEST_CASE( CoinTest ) {
    cout << "\nCOIN TEST\n" << endl;

    libzerocash::Address newAddress;
    libzerocash::PublicAddress pubAddress = newAddress.getPublicAddress();

    libzerocash::Coin coin(pubAddress, 0);

    cout << "Successfully created a coin.\n" << endl;

    CDataStream serializedCoin(SER_NETWORK, 7002);
    serializedCoin << coin;

    cout << "Successfully serialized a coin.\n" << endl;

    libzerocash::Coin coinNew;
    serializedCoin >> coinNew;

    cout << "Successfully deserialized a coin.\n" << endl;

    ///////////////////////////////////////////////////////////////////////////

    libzerocash::timer_start("Coin");
    libzerocash::Coin coin2(pubAddress, 0);
    libzerocash::timer_stop("Coin");

    cout << "Successfully created a coin.\n" << endl;

    CDataStream serializedCoin2(SER_NETWORK, 7002);
    serializedCoin2 << coin2;

    cout << "Successfully serialized a coin.\n" << endl;

    libzerocash::Coin coinNew2;
    serializedCoin2 >> coinNew2;

    cout << "Successfully deserialized a coin.\n" << endl;

    bool result = ((coin == coinNew) && (coin2 == coinNew2));

    BOOST_CHECK(result);
}

BOOST_AUTO_TEST_CASE( MintTxTest ) {
    cout << "\nMINT TRANSACTION TEST\n" << endl;

    libzerocash::Address newAddress;
    libzerocash::PublicAddress pubAddress = newAddress.getPublicAddress();

    vector<unsigned char> value(v_size, 0);

    libzerocash::timer_start("Coin");
    const libzerocash::Coin coin(pubAddress, 0);
    libzerocash::timer_stop("Coin");

    libzerocash::timer_start("Mint Transaction");
    libzerocash::MintTransaction minttx(coin);
    libzerocash::timer_stop("Mint Transaction");

    cout << "Successfully created a mint transaction.\n" << endl;

    CDataStream serializedMintTx(SER_NETWORK, 7002);
    serializedMintTx << minttx;

    cout << "Successfully serialized a mint transaction.\n" << endl;

    libzerocash::MintTransaction minttxNew;

    serializedMintTx >> minttxNew;

    cout << "Successfully deserialized a mint transaction.\n" << endl;

    libzerocash::timer_start("Mint Transaction Verify");
    bool minttx_res = minttxNew.verify();
    libzerocash::timer_stop("Mint Transaction Verify");

    BOOST_CHECK(minttx_res);
}

BOOST_AUTO_TEST_CASE( PourTxTest ) {
    cout << "\nPOUR TRANSACTION TEST\n" << endl;

    cout << "Creating Params...\n" << endl;

    libzerocash::timer_start("Param Generation");
    auto keypair = libzerocash::ZerocashParams::GenerateNewKeyPair(TEST_TREE_DEPTH);
    libzerocash::ZerocashParams p(
        TEST_TREE_DEPTH,
        &keypair
    );
    libzerocash::timer_stop("Param Generation");
    print_mem("after param generation");

    cout << "Successfully created Params.\n" << endl;

    vector<libzerocash::Coin> coins(5);
    vector<libzerocash::Address> addrs(5);

    for(size_t i = 0; i < coins.size(); i++) {
        addrs.at(i) = libzerocash::Address();
        coins.at(i) = libzerocash::Coin(addrs.at(i).getPublicAddress(), i);
    }

    cout << "Successfully created coins.\n" << endl;

    vector<std::vector<bool>> coinValues(5);

    vector<bool> temp_comVal(cm_size * 8);
    for(size_t i = 0; i < coinValues.size(); i++) {
        libzerocash::convertBytesVectorToVector(coins.at(i).getCoinCommitment().getCommitmentValue(), temp_comVal);
        coinValues.at(i) = temp_comVal;
        libzerocash::printVectorAsHex("Coin => ", coinValues.at(i));
    }

    cout << "Creating Merkle Tree...\n" << endl;

    libzerocash::timer_start("Merkle Tree");
    libzerocash::IncrementalMerkleTree merkleTree(coinValues, TEST_TREE_DEPTH);
    libzerocash::timer_stop("Merkle Tree");

    cout << "Successfully created Merkle Tree.\n" << endl;

    merkle_authentication_path witness_1(TEST_TREE_DEPTH);

    libzerocash::timer_start("Witness");
    if (merkleTree.getWitness(convertIntToVector(1), witness_1) == false) {
        BOOST_ERROR("Could not get witness");
	}
    libzerocash::timer_stop("Witness");

    cout << "Witness 1: " << endl;
    for(size_t i = 0; i < witness_1.size(); i++) {
        libzerocash::printVectorAsHex(witness_1.at(i));
    }
    cout << "\n" << endl;

    merkle_authentication_path witness_2(TEST_TREE_DEPTH);
    if (merkleTree.getWitness(convertIntToVector(3), witness_2) == false) {
		cout << "Could not get witness" << endl;
	}

    cout << "Witness 2: " << endl;
    for(size_t i = 0; i < witness_2.size(); i++) {
        libzerocash::printVectorAsHex(witness_2.at(i));
    }
    cout << "\n" << endl;

    libzerocash::Address newAddress3;
    libzerocash::PublicAddress pubAddress3 = newAddress3.getPublicAddress();

    libzerocash::Address newAddress4;
    libzerocash::PublicAddress pubAddress4 = newAddress4.getPublicAddress();

    libzerocash::Coin c_1_new(pubAddress3, 2);
    libzerocash::Coin c_2_new(pubAddress4, 2);

    vector<bool> root_bv(root_size * 8);
    merkleTree.getRootValue(root_bv);
    vector<unsigned char> rt(root_size);
    libzerocash::convertVectorToBytesVector(root_bv, rt);

    vector<unsigned char> ones(v_size, 1);
    vector<unsigned char> twos(v_size, 2);
    vector<unsigned char> as(sig_pk_size, 'a');

    cout << "Creating a pour transaction...\n" << endl;

    libzerocash::timer_start("Pour Transaction");
    libzerocash::PourTransaction pourtx(1, p, rt, coins.at(1), coins.at(3), addrs.at(1), addrs.at(3), 1, 3, witness_1, witness_2, pubAddress3, pubAddress4, 0, 0, as, c_1_new, c_2_new);
    libzerocash::timer_stop("Pour Transaction");
    print_mem("after pour transaction");

    cout << "Successfully created a pour transaction.\n" << endl;

    CDataStream serializedPourTx(SER_NETWORK, 7002);
    serializedPourTx << pourtx;

    cout << "Successfully serialized a pour transaction.\n" << endl;

    libzerocash::PourTransaction pourtxNew;
    serializedPourTx >> pourtxNew;

    cout << "Successfully deserialized a pour transaction.\n" << endl;

	std::vector<unsigned char> pubkeyHash(sig_pk_size, 'a');

    libzerocash::timer_start("Pour Transaction Verify");
    bool pourtx_res = pourtxNew.verify(p, pubkeyHash, rt);
    libzerocash::timer_stop("Pour Transaction Verify");

    BOOST_CHECK(pourtx_res);
}

BOOST_AUTO_TEST_CASE( MerkleTreeSimpleTest ) {
    cout << "\nMERKLE TREE SIMPLE TEST\n" << endl;

    vector<libzerocash::Coin> coins(5);
    vector<libzerocash::Address> addrs(5);

    cout << "Creating coins...\n" << endl;

    for(size_t i = 0; i < coins.size(); i++) {
        addrs.at(i) = libzerocash::Address();
        coins.at(i) = libzerocash::Coin(addrs.at(i).getPublicAddress(), i);
    }

    cout << "Successfully created coins.\n" << endl;

    vector<std::vector<bool>> coinValues(coins.size());

    vector<bool> temp_comVal(cm_size * 8);
    for(size_t i = 0; i < coinValues.size(); i++) {
        libzerocash::convertBytesVectorToVector(coins.at(i).getCoinCommitment().getCommitmentValue(), temp_comVal);
        coinValues.at(i) = temp_comVal;
        libzerocash::printVectorAsHex(coinValues.at(i));
    }

    cout << "Creating Merkle Tree...\n" << endl;

    libzerocash::IncrementalMerkleTree merkleTree(64);
	vector<bool> root;
	merkleTree.getRootValue(root);
	cout << "Root: ";
	libzerocash::printVectorAsHex(root);
	cout << endl;

	libzerocash::MerkleTree christinaTree(coinValues, 16);
	christinaTree.getRootValue(root);
	cout << "Christina root: ";
	libzerocash::printVectorAsHex(root);
	cout << endl;

    cout << "Successfully created Merkle Tree.\n" << endl;

	cout << "Copying and pruning Merkle Tree...\n" << endl;
	libzerocash::IncrementalMerkleTree copyTree = merkleTree;
	copyTree.prune();

	cout << "Obtaining compact representation and reconstituting tree...\n" << endl;
	libzerocash::IncrementalMerkleTreeCompact compactTree;
	merkleTree.getCompactRepresentation(compactTree);

	cout << "Compact representation vector: ";
	libzerocash::printBytesVector(compactTree.hashListBytes);
	libzerocash::printVector(compactTree.hashList);

	libzerocash::IncrementalMerkleTree reconstitutedTree(compactTree);
	reconstitutedTree.getRootValue(root);
	cout << "New root: ";
	libzerocash::printVectorAsHex(root);
	cout << endl;

	reconstitutedTree.insertVector(coinValues);
	merkleTree.insertVector(coinValues);

	vector<bool> index;
	reconstitutedTree.getRootValue(root);
	cout << "New root (added a bunch more): ";
	libzerocash::printVectorAsHex(root);
	cout << endl;

	merkleTree.getRootValue(root);
	cout << "Old root (added a bunch more): ";
	libzerocash::printVectorAsHex(root);
	cout << endl;

    merkle_authentication_path witness(16);
	if (merkleTree.getWitness(convertIntToVector(3), witness) == false) {
		BOOST_ERROR("Witness generation failed.");
	}

    cout << "Successfully created witness.\n" << endl;

    cout << "Witness: " << endl;
    for(size_t i = 0; i < witness.size(); i++) {
        libzerocash::printVectorAsHex(witness.at(i));
    }
    cout << "\n" << endl;

    merkle_authentication_path christina_witness(16);
	christinaTree.getWitness(coinValues.at(3), christina_witness);

    cout << "Christina created witness.\n" << endl;

    cout << "Christina Witness: " << endl;
    for(size_t i = 0; i < christina_witness.size(); i++) {
        libzerocash::printVectorAsHex(christina_witness.at(i));
    }
    cout << "\n" << endl;

    vector<bool> wit1(SHA256_BLOCK_SIZE * 8);
    vector<bool> wit2(SHA256_BLOCK_SIZE * 8);
    vector<bool> wit3(SHA256_BLOCK_SIZE * 8);
    vector<bool> inter_1(SHA256_BLOCK_SIZE * 8);
    vector<bool> inter_2(SHA256_BLOCK_SIZE * 8);
    std::vector<bool> zeros(SHA256_BLOCK_SIZE * 8, 0);

    wit1 = coinValues.at(2);
    libzerocash::hashVectors(coinValues.at(0), coinValues.at(1), wit2);
    libzerocash::hashVectors(coinValues.at(4), zeros, inter_1);
    inter_2 = zeros;
    libzerocash::hashVectors(inter_1, inter_2, wit3);

    BOOST_CHECK(christina_witness.size() == 16);
    for (size_t i = 0; i < 13; i++) {
        BOOST_CHECK(christina_witness.at(i) == zeros);
    }
    BOOST_CHECK(
        (christina_witness.at(13) == wit3) &&
        (christina_witness.at(14) == wit2) &&
        (christina_witness.at(15) == wit1)
    );

    BOOST_CHECK(witness.size() == 64);
    for (size_t i = 0; i < 61; i++) {
        BOOST_CHECK(witness.at(i) == zeros);
    }
    BOOST_CHECK(
        (witness.at(61) == wit3) &&
        (witness.at(62) == wit2) &&
        (witness.at(63) == wit1)
    );
}

BOOST_AUTO_TEST_CASE( SimpleTxTest ) {
    cout << "\nSIMPLE TRANSACTION TEST\n" << endl;

    libzerocash::timer_start("Param Generation");
    auto keypair = libzerocash::ZerocashParams::GenerateNewKeyPair(TEST_TREE_DEPTH);
    libzerocash::ZerocashParams p(
        TEST_TREE_DEPTH,
        &keypair
    );
    libzerocash::timer_stop("Param Generation");

    vector<libzerocash::Coin> coins(5);
    vector<libzerocash::Address> addrs(5);

    cout << "Creating Addresses and Coins...\n" << endl;
    for(size_t i = 0; i < coins.size(); i++) {
        addrs.at(i) = libzerocash::Address();
        coins.at(i) = libzerocash::Coin(addrs.at(i).getPublicAddress(), i);
    }
    cout << "Successfully created address and coins.\n" << endl;

    cout << "Creating a Mint Transaction...\n" << endl;
    libzerocash::MintTransaction minttx(coins.at(0));
    cout << "Successfully created a Mint Transaction.\n" << endl;

    cout << "Serializing a mint transaction...\n" << endl;
    CDataStream serializedMintTx(SER_NETWORK, 7002);
    serializedMintTx << minttx;
    cout << "Successfully serialized a mint transaction.\n" << endl;

    libzerocash::MintTransaction minttxNew;
    serializedMintTx >> minttxNew;
    cout << "Successfully deserialized a mint transaction.\n" << endl;

    cout << "Verifying a Mint Transaction...\n" << endl;
    bool minttx_res = minttxNew.verify();

    vector<std::vector<bool>> coinValues(5);
    vector<bool> temp_comVal(cm_size * 8);
    for(size_t i = 0; i < coinValues.size(); i++) {
        libzerocash::convertBytesVectorToVector(coins.at(i).getCoinCommitment().getCommitmentValue(), temp_comVal);
        coinValues.at(i) = temp_comVal;
    }

    cout << "Creating Merkle Tree...\n" << endl;
    libzerocash::IncrementalMerkleTree merkleTree(coinValues, TEST_TREE_DEPTH);
    cout << "Successfully created Merkle Tree.\n" << endl;

    cout << "Creating Witness 1...\n" << endl;
    merkle_authentication_path witness_1(TEST_TREE_DEPTH);
    if (merkleTree.getWitness(convertIntToVector(1), witness_1) == false) {
		BOOST_ERROR("Could not get witness");
	}
    cout << "Successfully created Witness 1.\n" << endl;

    cout << "Creating Witness 2...\n" << endl;
    merkle_authentication_path witness_2(TEST_TREE_DEPTH);
    if (merkleTree.getWitness(convertIntToVector(3), witness_2) == false) {
		cout << "Could not get witness" << endl;
	}
    cout << "Successfully created Witness 2.\n" << endl;

    cout << "Creating coins to spend...\n" << endl;
    libzerocash::Address newAddress3;
    libzerocash::PublicAddress pubAddress3 = newAddress3.getPublicAddress();

    libzerocash::Address newAddress4;
    libzerocash::PublicAddress pubAddress4 = newAddress4.getPublicAddress();

    libzerocash::Coin c_1_new(pubAddress3, 2);
    libzerocash::Coin c_2_new(pubAddress4, 2);
    cout << "Successfully created coins to spend.\n" << endl;

    vector<bool> root_bv(root_size * 8);
    merkleTree.getRootValue(root_bv);
    vector<unsigned char> rt(root_size);
    libzerocash::convertVectorToBytesVector(root_bv, rt);


    vector<unsigned char> as(sig_pk_size, 'a');

    cout << "Creating a pour transaction...\n" << endl;
    libzerocash::PourTransaction pourtx(1, p,
    		rt,
    		coins.at(1), coins.at(3),
    		addrs.at(1), addrs.at(3),
                1, 3,
                witness_1, witness_2,
    		pubAddress3, pubAddress4,
    		0,
            0,
    		as,
    		c_1_new, c_2_new);
    cout << "Successfully created a pour transaction.\n" << endl;

    cout << "Serializing a pour transaction...\n" << endl;
    CDataStream serializedPourTx(SER_NETWORK, 7002);
    serializedPourTx << pourtx;
    cout << "Successfully serialized a pour transaction.\n" << endl;

    libzerocash::PourTransaction pourtxNew;
    serializedPourTx >> pourtxNew;
    cout << "Successfully deserialized a pour transaction.\n" << endl;

	std::vector<unsigned char> pubkeyHash(sig_pk_size, 'a');

    cout << "Verifying a pour transaction...\n" << endl;
    bool pourtx_res = pourtxNew.verify(p, pubkeyHash, rt);

    BOOST_CHECK(minttx_res && pourtx_res);
}
