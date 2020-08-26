#include <bitcoin/bitcoin.hpp>
#include <string.h>
#include <iostream>

using namespace bc;
using namespace wallet;
using namespace chain;
using namespace machine;

void build_and_print_raw_transaction(std::map<std::string, std::string> arguments, bool testnet = true)
{

    transaction tx;     ///< this is gonna be our transaction
    tx.set_version(1u); ///< transaction version - always 1

    {
        /// TODO: This part can be repeated, and we can support coin join
        /// Previous TX hash and the index of outpu in that transaction that we want to spend.
        hash_digest prev_tx_hash_0;
        decode_hash(prev_tx_hash_0, arguments.at("utxo"));
        output_point uxto_tospend_0(prev_tx_hash_0, std::stoi(arguments.at("idx")));

        // Build input_to_spend object.
        input input_to_spend;
        input_to_spend.set_previous_output(uxto_tospend_0);
        input_to_spend.set_sequence(0xffffffff); ///< this is what we want now

        tx.inputs().push_back(input_to_spend); ///< input to spend
        ///< we assume that we spend from legacy addresses
    }

    /// FIRST OUTPUT IS TO THE CHANGE ADDRESS - We want our money back
    auto my_address_raw = arguments.at("backaddr"); ///< this is the address weher the change will go
    payment_address my_address1(my_address_raw);

    // Create Output output script/scriptPubKey from template:
    operation::list output_script_0 = script::to_pay_key_hash_pattern(my_address1.hash());
    std::string btc_amount_string_0 = std::to_string((unsigned int)(std::stof(arguments.at("backsats"))*100000000.0)/100000000.0);
    uint64_t satoshi_amount_0;
    decode_base10(satoshi_amount_0, btc_amount_string_0, btc_decimal_places); // btc_decimal_places = 8
    output output_0(satoshi_amount_0, output_script_0);
    tx.outputs().push_back(output_0); //first output
    /// THE OP_RETURN PART - this is second output
    if (arguments.count("msg") == 0)
        throw std::invalid_argument("you must privide message to put into OP_RETURN code -msg");
    std::string messageString = (arguments.count("msg")) ? arguments.at("msg") : "ICT-56";
    data_chunk nullData(messageString.begin(), messageString.end());
    chain::output output_1; // = program::output();
    output_1.set_script(chain::script(chain::script().to_null_data_pattern(nullData)));
    output_1.set_value(0);
    tx.outputs().push_back(output_1);

    // we can write the transaction to be signed later
    // std::cout << "tx: " << encode_base16(tx.to_data()) << std::endl;

    // Signer: Secret > Pubkey > Address
    wif_compressed wif;
    decode_base58(wif, arguments.at("wif"));
    ec_private my_private0(wif, testnet ? ec_private::testnet_p2kh : ec_private::mainnet_p2kh);
    ec_compressed pubkey0 = my_private0.to_public().point();
    payment_address my_address0 = my_private0.to_payment_address();

    // Signature
    endorsement sig_0;
    script prev_script_0 = script::to_pay_key_hash_pattern(my_address0.hash()); ///< we recreate previous script - assume that we are spending pay to key hash output
    script::create_endorsement(sig_0, my_private0.secret(), prev_script_0, tx, 0 /* this is the index of output of the new transaction */, 0x01);

    // Create input script
    operation::list sig_script_0;
    sig_script_0.push_back(operation(sig_0));
    sig_script_0.push_back(operation(to_chunk(pubkey0)));
    script my_input_script_0(sig_script_0);

    // Add input script to first input in transaction
    tx.inputs()[0].set_script(my_input_script_0);

    // Print serialised transaction
    std::cout << encode_base16(tx.to_data()) << std::endl;
}

int main(int argc, char **argv)
{
    std::map<std::string, std::string> arguments;
    std::string k = "-.";
    for (auto &s : std::vector<std::string>(argv, argv + argc))
    {
        if (s.size() && (s[0] == '-'))
        {
            k = s;
            arguments[k.substr(1)]; // create empty
        }
        else
            arguments[k.substr(1)] = s;
    }
    // todo - get the list of unspent as a json, this will be our next example
    // -utxo, -idx -backsats -backaddr -msg
    build_and_print_raw_transaction(arguments);

    return 0;
}
