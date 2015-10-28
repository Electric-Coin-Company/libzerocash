/** @file
 *****************************************************************************

 Using Google Test for testing various functionality.

 *****************************************************************************
 * @author     This file is part of libzerocash, developed by the Zerocash
 *             project and contributors (see AUTHORS).
 * @copyright  MIT license (see LICENSE file)
 *****************************************************************************/

using namespace std;

#include <iostream>

#include "libzerocash/utils/util.h"

#include "gtest/gtest.h"

TEST(convertBytesToVector, InverseEquality)
{
    const unsigned char* input = "Hello World!";
    std::vector<bool> intermediate;
    unsigned char output[14]; // input + '\0' + canary.

    output[13] = 0x03; // canary.

    libzerocash::convertBytesToVector(input, intermediate);
    libzerocash::convertVectorToBytes(intermediate, output);

    EXPECT_EQ(output[13], 0x03); // check canary.
    EXPECT_EQ(output[12], '\0'); // check nul-terminator.

    // Since we now believe output is nul-terminated:
    EXPECT_STREQ(output, input);
}
