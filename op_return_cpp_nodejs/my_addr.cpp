#include <bitcoin/bitcoin.hpp>
#include <iostream>
#include <vector>

/*

./my_addr -idx 0 -backaddr muu4TgEouWU9PmXmJAzqpywFE4Z9TQcAd7 -backsats 0.00009500 -utxo 65e68f6e2f90fe19114c1bdbde7fd9a9b0007618f7cb4a2880701ac7abdeb4fb
 // dostaniemy tekst, wpiszmy go w bitcoin-cli:
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
        if (s.size() && (s[0] == '-'))
            k = s;
        else
            arguments[k.substr(1)] = s;
    }
    // -utxo, -idx -backsats -backaddr -msg

    // select input and index of the input
    std::string hashString = arguments.at("utxo");
    uint32_t index1 = std::stoi(arguments.at("idx"));
    hash_digest utxoHash;
    decode_hash(utxoHash, hashString);
    output_point utxo(utxoHash, index1);
    input input1 = input();
    input1.set_previous_output(utxo);
    input1.set_sequence(0xffffffff);

    payment_address destinationAddy(arguments.at("backaddr"));
    script outputScript = script().to_pay_key_hash_pattern(destinationAddy.hash());

    std::string BTC = arguments.at("backsats");
    uint64_t Satoshis;
    decode_base10(Satoshis, BTC, 8);
    output output1(Satoshis, outputScript);
//    std::cout << "\nAmount: " << encode_base10(output1.value(), 8) << "BTC : Output Script: " << output1.script().to_string(0) << "\n"
//              << std::endl;

    // OP RETURN  :D

    std::string messageString = (arguments.count("msg"))?arguments.at("msg"):"ICT-56";
    std::cerr << "message: " << messageString << std::endl;
    data_chunk data(messageString.size() + 2);
    auto source = make_safe_deserializer(data.begin(), data.end());
    auto sink = make_unsafe_serializer(data.begin());
    sink.write_string(messageString);

    const auto nullData = source.read_bytes(messageString.size() + 2);
//    std::cout << "Message: " << encode_base16(nullData) << std::endl;

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
    std::cout << encode_base16(tx.to_data()) << std::endl;
}