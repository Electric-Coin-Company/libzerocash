/** @file
 *****************************************************************************

 Implementation of interfaces for the class PourOutput.

 See PourOutput.h .

 *****************************************************************************
 * @author     This file is part of libzerocash, developed by the Zerocash
 *             project and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

#include "PourOutput.h"

namespace libzerocash {

PourOutput::PourOutput() {
	Address dummy_to_address;

	this->to_address = dummy_to_address.getPublicAddress();
	this->new_coin = Coin(dummy_to_address.getPublicAddress(), 0);
}

// TODO: this is a generalization of the other constructor
PourOutput::PourOutput(uint64_t val) {
	Address dummy_to_address;

	this->to_address = dummy_to_address.getPublicAddress();
	this->new_coin = Coin(dummy_to_address.getPublicAddress(), val);
}

PourOutput::PourOutput(const Coin new_coin,
          const PublicAddress to_address) : new_coin(new_coin), to_address(to_address) {
}

} /* namespace libzerocash */