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

std::pair<std::string, std::string> generate_key_pair(std::string wif_privkey = "", bool testnet = true)
{

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
    if (wif_privkey.size() > 1)
    {
        //private_key_WIF = input("WIF: ")
        data_chunk first_encode;
        decode_base58(first_encode, wif_privkey);

        data_chunk e;
        for (int i = 1; i < first_encode.size() - 4; i++)
        {
            e.push_back(first_encode.at(i));
        }
        for (int i = 0; i < 32; i++)
            secretKey[i] = e.at(i); //bitcoin_hash(e);
    }
    // Derive pubkey point
    ec_compressed my_pubkey;
    secret_to_public(my_pubkey, secretKey);

    // Pubkeyhash: sha256 + hash160
    auto my_pubkeyhash = bitcoin_short_hash(my_pubkey);
// Prefix for mainnet = 0x00
    one_byte addr_prefix_mn = {{0x00}};
    one_byte addr_prefix_tn = {{0x6f}};
    one_byte addr_prefix = testnet?addr_prefix_tn:addr_prefix_mn;

    // Byte sequence = prefix + pubkey + checksum(4-bytes)
    data_chunk prefix_pubkey_checksum(to_chunk(addr_prefix));
    extend_data(prefix_pubkey_checksum, my_pubkeyhash);
    append_checksum(prefix_pubkey_checksum);

    // Base58 encode byte sequence -> Bitcoin Address
    //std::cout << BITCOIN_NETWORK_TYPE << " Address " << encode_base58(prefix_pubkey_checksum) << std::endl; ////////////////////////////////////////

// WIF encoded private key
// Additional Information: Mainnet, PubKey compression
    one_byte secret_prefix_mn = {{0x80}}; //Testnet Prefix: 0xEF
    one_byte secret_prefix_tn = {{0xEF}}; //Testnet Prefix: 0xEF
    one_byte secret_prefix = testnet?secret_prefix_tn:secret_prefix_mn;
    one_byte secret_compressed = {{0x01}}; //Omitted if uncompressed

    // Apply prefix, suffix & append checksum
    auto prefix_secret_comp_checksum = to_chunk(secret_prefix);
    extend_data(prefix_secret_comp_checksum, secretKey);
    extend_data(prefix_secret_comp_checksum, secret_compressed);
    append_checksum(prefix_secret_comp_checksum);

    // WIF (mainnet/compressed)
    //std::cout << BITCOIN_NETWORK_TYPE << " WIF (compressed) " << encode_base58(prefix_secret_comp_checksum) << std::endl;

    std::string hexKey = encode_base16(secretKey);
    //std::cout << "Hex secret: " << hexKey << std::endl;
    //return {btcaddr,wif_privkey};
    return {encode_base58(prefix_pubkey_checksum), encode_base58(prefix_secret_comp_checksum)};
}

int main(int argc, char **argv)
{
    std::vector<std::string> args(argv, argv + argc);
    auto [addr, wif_privkey] = generate_key_pair((args.size() > 1) ? args[1] : "");
    std::cout << "{\"addr\":\"" << addr << "\",\"wif\":\"" << wif_privkey << "\"}" << std::endl;
    return 0;
}
