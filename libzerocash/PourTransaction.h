/** @file
 *****************************************************************************

 Declaration of interfaces for the class PourTransaction.

 *****************************************************************************
 * @author     This file is part of libzerocash, developed by the Zerocash
 *             project and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#ifndef POURTRANSACTION_H_
#define POURTRANSACTION_H_

#include "Coin.h"
#include "ZerocashParams.h"
#include "Zerocash.h"
#include "PourInput.h"
#include "PourOutput.h"
#include <stdexcept>

typedef std::vector<unsigned char> CoinCommitmentValue;

namespace libzerocash {

/***************************** Pour transaction ******************************/

class PourTransaction {
public:
    PourTransaction();
    PourTransaction(ZerocashParams& params,
                                 const std::vector<unsigned char>& pubkeyHash,
                                 const MerkleRootType& rt,
                                 const std::vector<PourInput> inputs,
                                 const std::vector<PourOutput> outputs,
                                 uint64_t vpub_old,
                                 uint64_t vpub_new
                                );
    /**
     * Generates a transaction pouring the funds  in  two existing coins into two new coins and optionally
     * converting some of those funds back into the base currency.
     *
     *
     * @param version_num the version of the transaction to create
     * @param params the cryptographic parameters  used to generate the proofs.
     * @param root the root of the merkle tree containing the two existing coins
     * @param c_1_old the first existing coin
     * @param c_2_old the second existing coin
     * @param addr_1_old the address the first coin was paid to
     * @param addr_2_old the address the second coin was paid to
     * @param path_1 path showing that first coin is in the merkle tree rooted at root
     * @param path_2 path showing that the second coin is n the merkle tree rooted at root
     * @param addr_1_new the public address to pay the first new coin to
     * @param addr_2_new the public address to pay the second new coin to
     * @param v_pub the amount of funds to convert back to the base currency
     * @param pubkeyHash the hash of a public key to bind into the transaction
     * @param c_1_new the first of the new coins the funds are being poured into
     * @param c_2_new the second of the new coins the funds are being poured into
     */
    PourTransaction(uint16_t version_num,
                    ZerocashParams& params,
                    const MerkleRootType& roott,
                    const Coin& c_1_old,
                    const Coin& c_2_old,
                    const Address& addr_1_old,
                    const Address& addr_2_old,
                    const size_t patMerkleIdx_1,
                    const size_t patMerkleIdx_2,
                    const merkle_authentication_path& path_1,
                    const merkle_authentication_path& path_2,
                    const PublicAddress& addr_1_new,
                    const PublicAddress& addr_2_new,
                    uint64_t v_pub_in,
                    uint64_t v_pub_out,
                    const std::vector<unsigned char>& pubkeyHash,
                    const Coin& c_1_new,
                    const Coin& c_2_new);

    void init(uint16_t version_num,
                ZerocashParams& params,
                const MerkleRootType& roott,
                const Coin& c_1_old,
                const Coin& c_2_old,
                const Address& addr_1_old,
                const Address& addr_2_old,
                const size_t patMerkleIdx_1,
                const size_t patMerkleIdx_2,
                const merkle_authentication_path& path_1,
                const merkle_authentication_path& path_2,
                const PublicAddress& addr_1_new,
                const PublicAddress& addr_2_new,
                uint64_t v_pub_in,
                uint64_t v_pub_out,
                const std::vector<unsigned char>& pubkeyHash,
                const Coin& c_1_new,
                const Coin& c_2_new);

    /**
     * Verifies the pour transaction.
     *
     * @param params the cryptographic parameters used to verify the proofs.
     * @param pubkeyHash the hash of a public key that we verify is bound to the transaction
     * @param merkleRoot the root of the merkle tree the coins were in.
     * @return ture if correct, false otherwise.
     */
    bool verify(ZerocashParams& params,
                std::vector<unsigned char> &pubkeyHash,
                const MerkleRootType &merkleRoot) const;

    const std::vector<unsigned char>& getSpentSerial1() const;
    const std::vector<unsigned char>& getSpentSerial2() const;
    const std::string& getCiphertext1() const;
    const std::string& getCiphertext2() const;

    /**
     * Returns the hash of the first new coin generated by this Pour.
     *
     * @return the coin hash
     */
    const CoinCommitmentValue& getNewCoinCommitmentValue1() const;

    /**
     * Returns the hash of the second new coin generated by this Pour.
     *
     * @return the coin hash
     */
    const CoinCommitmentValue& getNewCoinCommitmentValue2() const;

    uint64_t getPublicValueIn() const;

    uint64_t getPublicValueOut() const;


private:

    std::vector<unsigned char>  publicOldValue;      // public input value of the Pour transaction
    std::vector<unsigned char>  publicNewValue;     // public output value of the Pour transaction
    std::vector<unsigned char>  serialNumber_1;     // serial number of input (old) coin #1
    std::vector<unsigned char>  serialNumber_2;     // serial number of input (old) coin #1
    CoinCommitment              cm_1;               // coin commitment for output coin #1
    CoinCommitment              cm_2;               // coin commitment for output coin #2
    std::vector<unsigned char>  MAC_1;              // first MAC    (h_1 in paper notation)
    std::vector<unsigned char>  MAC_2;              // second MAC   (h_2 in paper notation)
    std::string                 ciphertext_1;       // ciphertext #1
    std::string                 ciphertext_2;       // ciphertext #2
    std::string                 zkSNARK;            // contents of the zkSNARK proof itself
    uint16_t                    version;            // version for the Pour transaction
};

} /* namespace libzerocash */

#endif /* POURTRANSACTION_H_ */
