#include <cstdint>
#include "Tools.h"
#include <cstdlib>
#include <string>



/** 
 * builds a 64-bit long out of an array of 8 bytes
 *
 * for example, suppose bytes[0] == 0x12
 *              and     bytes[1] == 0x34
 *              and     bytes[2] == 0x56
 *              and     bytes[3] == 0x78
 *              and     bytes[4] == 0x9a
 *              and     bytes[5] == 0xbc
 *              and     bytes[6] == 0xde
 *              and     bytes[7] == 0xf0
 * then buildLong(bytes) returns 0xf0debc9a78563412
 *
 * @param array of 8 bytes
 * @re://github.com/CS3481/cs3481-lab1-maeganbrownturn uint64_t where the low order byte is bytes[0] and
 *         the high order byte is bytes[7]
*/
uint64_t Tools::buildLong(uint8_t bytes[LONGSIZE])
{
    //do i need to verify that the array size is 8?

    uint64_t compiledbytes = 0x0000000000000000;
    uint64_t mask;
    uint64_t sized;
    
    for (int i = 0; i < 8; i++) {

        mask = 0xFF00000000000000;
        mask = mask >> (8 * (7 - i));

        sized = bytes[i];
        sized = sized << (8 * i);

        compiledbytes += (sized & mask);
    }

    return compiledbytes;
}

/** 
 * accepts as input an uint64_t and returns the designated byte
 * within the uint64_t; returns 0 if the indicated byte number
 * is out of range 
 *
 * for example, getByte(0x1122334455667788, 7) returns 0x11
 *              getByte(0x1122334455667788, 1) returns 0x77
 *              getByte(0x1122334455667788, 8) returns 0
 *
 * @param uint64_t source that is the source data
 * @param int32_t byteNum that indicates the byte to return (0 through 7)
 * @return 0 if byteNum is out of range
 *         byte 0, 1, .., or 7 of source if byteNum is within range
 *
 * RESTRICTIONS: You cannot use an if statement.  This means you
 *               need to come up with some clever method to get the
 *               code to return 0 if bytenum is out of range and
 *               the selected byte otherwise.
*/
uint64_t Tools::getByte(uint64_t source, int32_t byteNum)
{    
    /** uint64_t byte = source >> (byteNum * 8);

    uint64_t scalingMask = 0;

    int64_t isNegative = (0x80000000) & (byteNum);
    int64_t isTooLarge = (0x80000000) & (7 - byteNum);
    uint64_t isValid = !(isNegative || isTooLarge);

    for (int i = 0; i < 8; i++) {
        scalingMask += (isValid << i);
    }

    byte = byte & scalingMask;

    return byte;
    */

    uint64_t retMask = ((int64_t)(!(byteNum >> 3)) << 63) >> 63;
    uint64_t mask = 0xff;

    source = source >> (byteNum * 8);
    uint64_t byte = source & mask;

    return byte & retMask;
}

/**
 * accepts as input an uint64_t and returns the bits low through 
 * high of the uint64_t.  bit 0 is the low order bit and bit 63
 * is the high order bit. returns 0 if the low or high bit numbers 
 * are out of range
 *
 * for example, getBits(0x8877665544332211, 0, 7) returns 0x11
 *              getBits(0x8877665544332211, 4, 11) returns 0x21
 *              getBits(0x8877665544332211, 0, 63) returns 0x8877665544332211
 *
 * @param uint64_t source that holds the bits to be grabbed and 
 *        returned
 * @param int32_t low that is the bit number of the lowest numbered
 *        bit to be returned
 * @param int32_t high that is the bit number of the highest numbered
 *        bit to be returned
 * @return an uint64_t that holds a subset of the source bits
 *         that is returned in the low order bits; 0 if low or high 
 *         is out of range
 *
 * RESTRICTIONS: You can only use an if statement to determine whether
 *               the low and high values are valid. 
 */
uint64_t Tools::getBits(uint64_t source, int32_t low, int32_t high)
{
    /* if (low < 0 || low > 63 || high < 0 || high > 63
            || low > high) {
        return 0;
    }

    uint64_t desiredbits = 0;

    for (int i = low; i <= high; i++) {
        uint64_t temp = 0x8000000000000000; // temp is used to pull the desired bit from source
        temp = temp >> (63 - i);    // shifting temp to hold 1 in the desired position
        uint64_t bit = 0;   // bit is used to hold the desired bit obtained using temp
        bit += (source & temp); // placing desired bit in bit
        bit = bit >> low;   // shifting the bit to the correct position
        desiredbits += bit; // adding the bit to the desiredbits number
    }

    return desiredbits;
    */

    if (high > 63 || low > 63 || high < 0 || low < 0 || low > high)
        return 0;

    source = source << (63 - high);
    source = source >> ((63 - high) + low);

    return source;
}

/**
 * sets the bits of source in the range specified by the low and high
 * parameters to 1 and returns that value. returns source if the low or high
 * bit numbers are out of range
 *
 * for example, setBits(0x1122334455667788, 0, 7) returns 0x11223344556677ff
 *              setBits(0x1122334455667788, 8, 0xf) returns 0x112233445566ff88
 *              setBits(0x1122334455667788, 8, 64) returns 0x1122334455667788
 *                      note: 64 is out of range
 *
 * @param uint64_t source 
 * @param int32_t low that is the bit number of the lowest numbered
 *        bit to be set to 1
 * @param int32_t high that is the bit number of the highest numbered
 *        bit to be set to 1
 * @return an uint64_t that holds the modified source
 *
 * RESTRICTIONS: You can only use an if statement to determine whether
 *               the low and high values are valid. 
 */
uint64_t Tools::setBits(uint64_t source, int32_t low, int32_t high)
{
    if (low < 0 || low > 63 || high < 0 || high > 63
            || low > high) {
        return source;
    }

    uint64_t updatedSource = source;
    
    for (int i = low; i <= high; i++) {
        uint64_t temp = 0x8000000000000000; // temp is used to move the 1 bit to the desired position
        temp = temp >> (63 - i);    // 1 bit is moved to desired position
        updatedSource = updatedSource | temp;   // 1 bit is added to source
    }

    return updatedSource;
}

/**
 * sets the bits of source in the range low to high to 0 (clears them) and
 * returns that value. returns source if the low or high
 * bit numbers are out of range
 *
 * for example, clearBits(0x1122334455667788, 0, 7) returns 0x1122334455667700
 *              clearBits(0x1122334455667788, 8, 15) returns 0x1122334455660088
 *
 * @param uint64_t source 
 * @param int32_t low that is the bit number of the lowest numbered
 *        bit to be set to 0
 * @param int32_t high that is the bit number of the highest numbered
 *        bit to be set to 0
 * @return an uint64_t that holds the modified source
 *
 * RESTRICTIONS: You can only use an if statement to determine whether
 *               the low and high values are valid. 
 */
uint64_t Tools::clearBits(uint64_t source, int32_t low, int32_t high)
{
    if (low < 0 || low > 63 || high < 0 || high > 63
            || low > high) {
        return source;
    }

    uint64_t updatedSource = source;

    for (int i = low; i <= high; i++) {
        uint64_t temp = 0x8000000000000000; // temp is used to move the 0 bit to the desired position
        temp = temp >> (63 - i);    // 0 bit is moved to desired position
        temp = ~temp;   // making 0 bit actually be the zero bit
        updatedSource = updatedSource & temp;   // 0 bit is added to source
    }

    return updatedSource;
}

/**
 * copies length bits from the source to a destination and returns the
 * modified destination. If low bit number of the source or 
 * dest is out of range or the calculated source or dest high bit 
 * number is out of range, then the unmodified destination is returned.
 *
 * for example,
 *   copyBits(0x1122334455667788, 0x8877665544332211, 0, 0, 8) 
 *           returns 0x8877665544332288
 *   copyBits(0x1122334455667788, 0x8877665544332211, 0, 8, 8) 
 *           returns 0x8877665544338811
 *
 * @param uint64_t source 
 * @param uint64_t dest 
 * @param int32_t srclow that is the bit number of the lowest numbered
 *        bit of the source to be copied
 * @param int32_t destlow that is the bit number of the lowest numbered
 *        bit of the destination to be modified
 * @param int32_t length that is the number of bits to be copied
 * @return uint64_t that is the modifed dest
 *
 * RESTRICTIONS: You can only use an if statement to determine whether
 *               the low and high values are valid. 
 */
uint64_t Tools::copyBits(uint64_t source, uint64_t dest, 
                         int32_t srclow, int32_t dstlow, int32_t length)
{
    /* int32_t srchigh = srclow + length - 1;
    int32_t dsthigh = dstlow + length - 1;

    if (srclow < 0 || srclow > 63 || dstlow < 0 || dstlow > 63
            || srchigh < 0 || srchigh > 63 || dsthigh < 0 || dsthigh > 63
            || length < 0) {
        return dest;
    }

    uint64_t clearedDest = clearBits(dest, dstlow, dsthigh);
    uint64_t sourceBits = getBits(source, srclow, srchigh);

    for (int i = 0; i < length; i++) {  // i tracks the position in both src and dest
        uint64_t sbObtainer = 0x1;  // used to obtain bits from sourceBits
        uint64_t bit = sbObtainer & sourceBits; //obtained bit from sourceBits

        uint64_t sbInserter = bit;  // used to insert bits into clearedDest
        sbInserter = sbInserter << (dstlow + i);    // positioned the bit to insert

        clearedDest = clearedDest | sbInserter; //inserted the bit

        sourceBits = sourceBits >> 1;
    }

    return clearedDest;
    */

    if (srclow < 0 || srclow > 63 || dstlow < 0 || dstlow > 63 || srclow + (length-1) > 63 || dstlow + (length-1) > 63 || length < 0)
        return dest;

    uint64_t bits = getBits(source, srclow, srclow + length - 1);
    bits = bits << dstlow;
    dest = clearBits(dest, dstlow, dstlow + length - 1);

    return dest | bits;

}

/**
 * sets the bits of source identfied by the byte number to 1 and
 * returns that value. if the byte number is out of range then source
 * is returned unchanged.
 *
 * for example, setByte(0x1122334455667788, 0) returns 0x11223344556677ff
 *              setByte(0x1122334455667788, 1) returns 0x112233445566ff88
 *              setByte(0x1122334455667788, 8) returns 0x1122334455667788
 *
 * @param uint64_t source 
 * @param int32_t byteNum that indicates the number of the byte to be
 *        set to 0xff; the low order byte is byte number 0
 * @return uint64_t that is source with byte byteNum set to 0xff
 *
 * RESTRICTIONS: You cannot use an if statement.  This means you
 *               need to come up with some clever method to get the
 *               code to return 0 if bytenum is out of range and
 *               the source otherwise.

 */
uint64_t Tools::setByte(uint64_t source, int32_t byteNum)
{
    int64_t isNegative = (0x80000000) & (byteNum);
    int64_t isTooLarge = (0x80000000) & (7 - byteNum);
    uint64_t isValid = !(isNegative || isTooLarge);
    uint64_t validityMask = 0;
    for (int i = 0; i < 64; i++) {
        validityMask += (isValid << i);
    }

    uint64_t mask = 0xFF00000000000000; // mask to insert 1's at proper position
    mask = mask >> (8 * (7 - byteNum));  // moveing 1's to their proper positions
    mask = mask & validityMask; // verifying byte in acceptable range

    uint64_t updatedSource = source | mask;

    return updatedSource;
}

/**
 * assumes source contains a 64 bit two's complement value and
 * returns the sign (1 or 0)
 *
 * for example, sign(0xffffffffffffffff) returns 1
 *              sign(0x0000000000000000) returns 0
 *              sign(0x8000000000000000) returns 1
 *
 * @param uint64_t source
 * @return 1 if source is negative when treated as a two's complement 
 *         value and 0 otherwise
 *
 * RESTRICTIONS: You cannot use an if statement.
 */
uint8_t Tools::sign(uint64_t source)
{
    // I can just write return (source >> 63);, however this uses the fact that source is unsigned.
    // Let's also do this assuming source was signed:
    
    uint64_t mask = 0x8000000000000000;
    uint64_t sign = mask & source;
    sign = sign >> 63;

    return sign;
}

/**
 * assumes that op1 and op2 contain 64 bit two's complement values
 * and returns true if an overflow would occur if they are summed
 * and false otherwise
 *
 * for example, addOverflow(0x8000000000000000, 0x8000000000000000) returns 1
 *              addOverflow(0x7fffffffffffffff, 0x7fffffffffffffff) returns 1
 *              addOverflow(0x8000000000000000, 0x7fffffffffffffff) returns 0
 *
 * @param uint64_t op1 that is one of the operands of the addition
 * @param uint64_t op2 that is the other operand of the addition
 * @return true if op1 + op2 would result in an overflow assuming that op1
 *         and op2 contain 64-bit two's complement values
 *
 * RESTRICTIONS: You cannot use an if statement.
 */
bool Tools::addOverflow(uint64_t op1, uint64_t op2)
{
    uint64_t sum = op1 + op2;

    uint8_t op1sign = sign(op1);
    uint8_t op2sign = sign(op2);
    uint8_t sumsign = sign(sum);

    /**
     *  If different signs --> no chance of overflow
     *  If same signs --> overflow if sum sign is diff from operands' sign
     *
     *  If both signs are the same, then this statement will always be false:
     *      !op1sign & op2sign
     *  So, if both signs are the same, then this statement will be true:
     *      !(!op1sign & op2sign)
     *  There is only a possibility for overflow when the above statement is true.
     *  If there is overflow, then there is only overflow when the following statement is true:
     *      (!op1sign & op2sign) & (sumsign ^ op1sign)
     *  Thus, it is true that there is overflow when:
     *      !(!op1sign & op2sign) & (sumsign ^ op1sign)
     */

    //return !(!op1sign & op2sign) & (sumsign ^ op1sign);
    return (op1sign == op2sign) && (sumsign != op1sign);
}

/**
 * assumes that op1 and op2 contain 64 bit two's complement values
 * and returns true if an overflow would occur from op2 - op1
 * and false otherwise
 *
 * for example, subOverflow(0x8000000000000000, 0x8000000000000000) returns 0
 *              subOverflow(0x7fffffffffffffff, 0x7fffffffffffffff) returns 0
 *              subOverflow(0x8000000000000000, 0x7fffffffffffffff) returns 1
 *
 * @param uint64_t op1 that is one of the operands of the subtraction
 * @param uint64_t op2 that is the other operand of the subtraction
 * @return true if op2 - op1 would result in an overflow assuming that op1
 *         and op2 contain 64-bit two's complement values
 *
 * RESTRICTIONS: You cannot use an if statement.
 */
bool Tools::subOverflow(uint64_t op1, uint64_t op2)
{
   //Note: the result computed is op2 - op1 (not op1 - op2)
   
    uint64_t sub = op2 - op1;

    uint8_t op1sign = sign(op1);
    uint8_t op2sign = sign(op2);
    uint8_t subsign = sign(sub);

    /**
     *  Overflow occurs when op1 and op2 have different signs AND
     *      the sum has the same sign as op1 (the thing being subtracted).
     *  
     *  If both signs are different, then this statement will always be true:
     *      op1sign XOR op2sign
     *  There is only a possiblity for overflow when the above statement is true.
     *  Furthermore, there will only be overflow when the following is false:
     *      !subsign & op1sign
     *  So, if there is overflow, this will always be true:
     *      (!subsign ^ op1sign)
     *  Thus, there is only overflow when both cases are met:
     *      (op1sign XOR op2sign) & (!subsign ^ op1sign)
     */

    //return (op1sign ^ op2sign) & (!subsign ^ op1sign);
    return (op2sign != op1sign) && (subsign != op2sign);
}








