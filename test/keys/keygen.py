#!/usr/bin/env python3
# created by appleflyer, courtesy of HavenOverflow

# Hannah here! This will be used to generate valid RSA keys that have a keyid that matches the
# allowed key list on Haven and Dauntless. Because the public keys are hardcoded, they can't
# actually pass verification, but they can get a lot farther into the launch process if needed.

import math, secrets, sys
from Crypto.Util import number
from Crypto.PublicKey import RSA

def credits():
    print("vulnerability found by HavenOverflow and its members.")
    print("PoC created by appleflyer")

def gen_rsa(keytype):
    # mod M = 2^32 to tie the primes to the keyid
    M = 2**32
    e = keytype[3]
    keyname = keytype[0]
    keyid = keytype[1]
    bits = keytype[2]

    print(f"generating {keyname} key...")

    # now, compute r so that r * (-keyid) = 1 mod M
    r = pow(-keyid % M, -1, M)
    # p and q should be about half the total bit len.
    half = bits // 2

    def k_bounds(p0):
        # given p0 mod M, find int k so that p = p0 + k * M
        lo = 1 << (half - 1) # smallest half bit prime
        hi = (1 << half) - 1 # largest half bit prime

        # now, we solve for lo =< p0 + kM =< hi for k
        return (lo - p0 + M - 1) // M, (hi - p0) // M

    while True:
        # pick a rnadom half bit prime and put it as q
        q = number.getPrime(half)
        q0 = int(q % M)

        # ensure q0 is INVERTIBLE with mod M
        if math.gcd(q0, M) != 1:
            continue

        # compute p0 = r * q0^(-1) (mod M)
        inv_q0 = pow(q0, -1, M)
        p0 = (r * inv_q0) % M

        # find the k range so p_candidate = p0 + k*M has correct bit length
        kmin, kmax = k_bounds(p0)
        rng = kmax - kmin + 1
        if rng <= 0:
            # no valid p in the range, we shall try with a NEW q
            continue

        # we shall sample k UNIFORMLY with rng, to ensure we will get 3072 bits after 1-2 calculations, and then we check if p is prime.
        while True:
            k = kmin + secrets.randbelow(rng)
            p_candidate = p0 + k * M
            if number.isPrime(p_candidate):
                p = p_candidate
                break

        # did we get a 3072 bit modulus? if we did, exit.
        n = p * q
        if n.bit_length() != bits:
            # got a 3071 bit or shorter n, so we discard and retry
            continue

        # compute d = e^(-1) mod phi(n) -> (p-1)(q-1)
        # ensure this works with the changing exponent. we need to make sure that one of the primes meet gcd(e, phi(n))
        try:
            d = pow(e, -1, (p - 1) * (q - 1))
        except:
            continue

        # yes! we got a key.
        break

    # finally, construct the key LETS GOOOOO
    n = p * q
    key = RSA.construct((n, e, d, p, q))
    if (n * keyid) % M != M - 1:
        print("keyid mismatch! this is a critical error!!!")
        print(f"keyid mismatch: (n*{keyid:#x} mod 2^32) = {(n*keyid)%M:#x}, but expected {M-1:#x}")
        return False
    else:
        print(f"keyid verification passed: (n*{keyid:#x} mod 2^32) = {M-1:#x}")
        with open(f'{keyname}.pem','wb') as f:
            f.write(key.export_key('PEM'))
        with open(f'{keyname}.pem.pub','wb') as f:
            f.write(key.publickey().export_key('PEM'))
        print(f"written {keyname} private and public keys!")
    return True

def main():
    key_groups = [
        # Format: ["key name", 0xkeyid, bit count, public exponent]
        ["haven-rom-prod",      0xaa66150f, 3072, 0x3],      # Cr50 RO 0.0.9 and less
        ["haven-rom-dev",       0x3716ee6b, 2048, 0x3],      # Cr50 RO 0.0.10 and up

        ["cr50-rw-b2-prod",     0x87b73b67, 2048, 0x3],
        ["cr50-rw-b1-prod",     0xde88588d, 2048, 0x3],
        ["cr50-rw-b1-dev",      0xb93d6539, 2048, 0x3],
    ]
    credits()
    for a in key_groups:
        ret = gen_rsa(a)
        if not ret:
            print("PoC failed. something went wrong...")
            sys.exit()
    print("PoC success!!")
main()