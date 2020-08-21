#include <bitcoin/bitcoin.hpp>
#include <iostream>
#include <vector>
#include <tuple>

// sporo wziete z http://aaronjaramillo.org/libbitcoin-first-program

//#define MAINNET_ "mainnet"
#define TESTNET_ "testnet"

#ifdef TESTNET_
const std::string BITCOIN_NETWORK_TYPE = "testnet";
#else
const std::string BITCOIN_NETWORK_TYPE = "mainnet";
#endif

//Generowanie adresów w wersji legacy - najłatwiejsze i na początek nam wystarczy :)

std::pair<std::string,std::string> generate_key_pair(std::string privkey="") {

    using namespace bc;


    std::random_device rd;
    //std::mt19937 gen((args.size() > 1) ? std::stoll(args.at(1)) : rd());
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 255);

    data_chunk seed(ec_secret_size); //256bits
    for (auto &e : seed)
    {
        e = distrib(gen);
    }
    ec_secret secretKey = bitcoin_hash(seed);
    if (privkey.size() > 1)
    {
//private_key_WIF = input("WIF: ")
//first_encode = decode_base58(privkey)
//private_key_full = binascii.hexlify(first_encode)
//private_key = private_key_full[2:-8]
//print(private_key)
//        decode_base16(secretKey, args.at(1));
    }
    // Derive pubkey point
    ec_compressed my_pubkey;
    secret_to_public(my_pubkey, secretKey);

    // Pubkeyhash: sha256 + hash160
    auto my_pubkeyhash = bitcoin_short_hash(my_pubkey);
// Prefix for mainnet = 0x00
#ifdef MAINNET_
    one_byte addr_prefix = {{0x00}};
#else
    one_byte addr_prefix = {{0x6f}};
#endif

    // Byte sequence = prefix + pubkey + checksum(4-bytes)
    data_chunk prefix_pubkey_checksum(to_chunk(addr_prefix));
    extend_data(prefix_pubkey_checksum, my_pubkeyhash);
    append_checksum(prefix_pubkey_checksum);

    // Base58 encode byte sequence -> Bitcoin Address
    std::cout << BITCOIN_NETWORK_TYPE << " Address " << encode_base58(prefix_pubkey_checksum) << std::endl; ////////////////////////////////////////

// WIF encoded private key
// Additional Information: Mainnet, PubKey compression
#ifdef MAINNET_
    one_byte secret_prefix = {{0x80}}; //Testnet Prefix: 0xEF
#else
    one_byte secret_prefix = {{0xEF}}; //Testnet Prefix: 0xEF
#endif
    one_byte secret_compressed = {{0x01}}; //Omitted if uncompressed

    // Apply prefix, suffix & append checksum
    auto prefix_secret_comp_checksum = to_chunk(secret_prefix);
    extend_data(prefix_secret_comp_checksum, secretKey);
    extend_data(prefix_secret_comp_checksum, secret_compressed);
    append_checksum(prefix_secret_comp_checksum);

    // WIF (mainnet/compressed)
    std::cout << BITCOIN_NETWORK_TYPE << " WIF (compressed) " << encode_base58(prefix_secret_comp_checksum) << std::endl;

    std::string hexKey = encode_base16(secretKey);
    std::cout << "Hex secret: " << hexKey << std::endl;
    return {btcaddr,privkey};
}

int main(int argc, char **argv)
{
    std::vector<std::string> args(argv, argv + argc);
    auto [addr, privkey] = generate_key_pair();
    std::cout << addr << " " << privkey << std::endl;
    return 0;
}
