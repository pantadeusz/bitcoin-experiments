#include <bitcoin/bitcoin.hpp>
#include <iostream>
#include <vector>

/*

./my_addr -idx 0 -backaddr muu4TgEouWU9PmXmJAzqpywFE4Z9TQcAd7 -backsats 0.00009500 -utxo 65e68f6e2f90fe19114c1bdbde7fd9a9b0007618f7cb4a2880701ac7abdeb4fb
to powinno podpisać transakcję
./my_addr -idx 0 -backaddr muu4TgEouWU9PmXmJAzqpywFE4Z9TQcAd7 -backsats 0.00008000 -utxo 5b946ef154a963fcd8753c2cb0044ca7a72464b985109e3004e519e824593316 -wif cS8jiipSQCxfsaJCJyRBGoi6fJLWEq7ZaxcDo79gVJ3WZDH7kXUH // dostaniemy tekst, wpiszmy go w bitcoin-cli:
 bitcoin-cli signrawtransactionwithwallet 0000000001fbb4deabc71a7080284acbf7187600b0a9d97fdedb1b4c1119fe902f6e8fe6650000000000ffffffff021c250000000000001976a9149dc00f2693fd2f9de495cb5abd80db28bcd92d1688ac00000000000000000a6a08064943542d35360000000000

// dostaniemy hex, teraz go przepiszmy
bitcoin-cli sendrawtransaction 0000000001fbb4deabc71a7080284acbf7187600b0a9d97fdedb1b4c1119fe902f6e8fe665000000006a47304402201c158db580fc03131f7bc96b779850dba6e1938a3d39dddddd2b8a9715fd094a02201b7524bdaa72e4971f5007eb2efb82b08d8a78bde9bbe4e6074334cb48d70cbb012103f140e758f4822d7add629bacef974c0b5408f959730829fad185dd168b3a730cffffffff021c250000000000001976a9149dc00f2693fd2f9de495cb5abd80db28bcd92d1688ac00000000000000000a6a08064943542d35360000000000

i mamy transakcję, która powinna pojawić się w eksploratorze bloków wersji testnet
https://live.blockcypher.com/btc-testnet/tx/37bf9a4c111f9d51d36518a5706d32f657ce1a122ed8138e3b92713cc12df66e/
*/

// sporo wziete z http://aaronjaramillo.org/libbitcoin-first-program

//#define MAINNET_ "mainnet"
#define TESTNET_ "testnet"

#ifdef TESTNET_
const std::string BITCOIN_NETWORK_TYPE = "testnet";
#else
const std::string BITCOIN_NETWORK_TYPE = "mainnet";
#endif

// OP_RETURN example in C++
// synteza tego co w tutorialach powyżej (link) z moimi rozwiązaniami + uproszczenie.

bc::ec_secret wif_to_secret(std::string wif_privkey)
{
    using namespace bc;

    ec_secret secretKey;
    data_chunk first_encode;
    decode_base58(first_encode, wif_privkey);

    data_chunk e;
    for (int i = 1; i < first_encode.size() - 4; i++)
        e.push_back(first_encode.at(i));
    for (int i = 0; i < 32; i++)
        secretKey[i] = e.at(i); //bitcoin_hash(e);
    return secretKey;
}

bc::data_chunk wif_to_addr(std::string wif_privkey, bool testnet = true)
{
    using namespace bc;
    if (wif_privkey.size() < 4)
        throw std::invalid_argument("please provide wif_privkey");
    ec_secret secretKey = wif_to_secret(wif_privkey);

    // Derive pubkey point
    ec_compressed my_pubkey;
    secret_to_public(my_pubkey, secretKey);

    // Pubkeyhash: sha256 + hash160
    auto my_pubkeyhash = bitcoin_short_hash(my_pubkey);
    // Prefix for mainnet = 0x00
    static const one_byte addr_prefix_mn = {{0x00}};
    static const one_byte addr_prefix_tn = {{0x6f}};
    one_byte addr_prefix = testnet ? addr_prefix_tn : addr_prefix_mn;

    // Byte sequence = prefix + pubkey + checksum(4-bytes)
    data_chunk prefix_pubkey_checksum(to_chunk(addr_prefix));
    extend_data(prefix_pubkey_checksum, my_pubkeyhash);
    append_checksum(prefix_pubkey_checksum);
    return prefix_pubkey_checksum;
}

int main(int argc, char **argv)
{
    using namespace bc;
    using namespace bc::wallet;
    using namespace bc::machine;
    using namespace bc::chain;

    std::map<std::string, std::string> arguments;
    std::string k = "-.";
    for (auto &s : std::vector<std::string>(argv, argv + argc))
    {
        if (s.size() && (s[0] == '-')) {
            k = s;
            arguments[k.substr(1)]; // create empty
        } else
            arguments[k.substr(1)] = s;
    }
    // todo: automatically find transactions that have our address in unspent

    // -utxo, -idx -backsats -backaddr -msg

    // select input and index of the input
    std::string hashString = arguments.at("utxo");    ///<
    uint32_t index1 = std::stoi(arguments.at("idx")); ///< unspent transaction index
    hash_digest utxoHash;
    decode_hash(utxoHash, hashString);
    output_point utxo(utxoHash, index1);
    input input1 = input();
    input1.set_previous_output(utxo);
    input1.set_sequence(0xffffffff);

    // now the pay back value to the given addr
    payment_address destinationAddy(arguments.at("backaddr")); ///< put the rest of the value into this addr
    script outputScript = script().to_pay_key_hash_pattern(destinationAddy.hash());

    std::string BTC = arguments.at("backsats");
    uint64_t Satoshis;
    decode_base10(Satoshis, BTC, 8);
    output output1(Satoshis, outputScript);

    // OP RETURN  :D
    std::string messageString = (arguments.count("msg")) ? arguments.at("msg") : "ICT-56";
    data_chunk data(messageString.size());
    auto source = make_safe_deserializer(data.begin(), data.end());
    auto sink = make_unsafe_serializer(data.begin());
    sink.write_string(messageString);

    const auto nullData = source.read_bytes(messageString.size());
    chain::output output2; // = program::output();
    output2.set_script(chain::script(chain::script().to_null_data_pattern(nullData)));
    output2.set_value(0);

    //    std::cout << "\nAmount: " << encode_base10(output1.value(), 8) << "BTC : Output Script: " << output2.script().to_string(0) << "\n"
    //              << std::endl;

    //build TX
    chain::transaction tx = transaction();
    tx.inputs().push_back(input1);
    tx.outputs().push_back(output1);
    tx.outputs().push_back(output2);

    // unsigned transaction - this must be signed by wallet
    if (arguments.count("onlytx")) {
        if (arguments.count("wif") == 0) std::cout << "bitcoin-cli signrawtransactionwithwallet " << encode_base16(tx.to_data()) << std::endl;
    } else {
        std::cout << "bitcoin-cli signrawtransactionwithwallet " << encode_base16(tx.to_data()) << std::endl;
    }
    // let's sign this transaction
    if (arguments.count("wif"))
    {
        if (arguments.count("onlytx") == 0)std::cout << "addr: " << encode_base58(wif_to_addr(arguments.at("wif"))) << std::endl;

        //Previous Locking Script
        ec_secret my_secret0 = wif_to_secret(arguments.at("wif"));

        ec_private my_private0(my_secret0, ec_private::testnet, true);
        ec_compressed pubkey0 = my_private0.to_public().point();
        payment_address my_address0 = my_private0.to_payment_address();
        //std::cout << "MYADDR: " << my_address0.encoded() << std::endl;
        // Signature
        endorsement sig_0;
        script prev_script_0 = script::to_pay_key_hash_pattern(my_address0.hash());
        uint8_t input0_index(index1);
        std::cerr << "IDX: " << (int)input0_index << " " << index1 << std::endl;
        script::create_endorsement(sig_0, my_secret0, prev_script_0, tx,
                                   input0_index, 0x01);

        // Create input script
        operation::list sig_script_0;
        sig_script_0.push_back(operation(sig_0));
        sig_script_0.push_back(operation(to_chunk(pubkey0)));
        script my_input_script_0(sig_script_0);

        // Add input script to first input in transaction
        tx.inputs()[0].set_script(my_input_script_0);

        // Print serialised transaction
        if (arguments.count("onlytx") == 0) std::cout << "Raw Transaction: " << std::endl;
        std::cout << encode_base16(tx.to_data()) << std::endl;
    }
    else
    {
        if (arguments.count("onlytx") == 0) std::cerr << "please sign this transaction" << std::endl;
    }
}