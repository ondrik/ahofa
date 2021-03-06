/// @author Jakub Semric
/// 2018

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <cassert>
#include <map>

#include "nfa.hpp"

using namespace reduction;
using namespace std;

static uint8_t hex_to_int(const string &str)
{
    assert(str.size() > 2);

    int x = 0;
    int m = 1;
    for (int i = str.length()-1; i > 1; i--) {
        int c = tolower(str[i]);
        x += m * (c - (c > '9' ? 'a' - 10 : '0'));
        m *= 16;
    }
    assert(x >= 0 && x < 256);

    return static_cast<uint8_t>(x);
}

static string int_to_hex(const unsigned num)
{
    assert (num <= 255);
    char buf[16] = "";
    sprintf(buf, "0x%.2x", num);
    return buf;
}

template<typename T>
static void set_union(set<T> &s1, const set<T> &s2) {
    for (auto i : s2) {
        s1.insert(i);
    }
}

//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// implementation of Nfa class methods
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Nfa::Nfa(const Nfa &nfa)
{
    initial_state = nfa.initial_state;
    final_states = nfa.final_states;
    transitions = nfa.transitions;
}

Nfa Nfa::read_from_file(const string input)
{
    ifstream in{input};
    if (!in.is_open()) {
        throw runtime_error("cannot open NFA file");
    }
    auto res = read_from_file(in);
    in.close();
    return res;
}

Nfa Nfa::read_from_file(ifstream &input)
{
    bool no_final = true;
    string buf;
    vector<TransFormat> trans;
    set<State> finals;
    State init;

    try {
        // reading initial state
        getline(input, buf);
        init = stoul(buf);

        // reading transitions
        while (getline(input, buf)) {
            istringstream iss(buf);
            string s1, s2, a;
            if (!(iss >> s1 >> s2 >> a )) {
                no_final = false;
                break;
            }
            trans.push_back(TransFormat{stoul(s1), stoul(s2), hex_to_int(a)});
        }
        // set final states
        if (!no_final) {
            do {
                finals.insert(stoul(buf));
            } while (getline(input, buf));
        }
    }
    catch (...) {
        throw runtime_error("invalid NFA syntax");
    }

    return Nfa(init, trans, finals);
}

Nfa::Nfa(State init, const vector<TransFormat> &t, const set<State> &finals) :
    initial_state{init}, final_states{finals}
{
    transitions[init];
    for (auto i : t) {
        transitions[i.first][i.third].insert(i.second);
        transitions[i.second];
    }

    for (auto i : final_states) {
        transitions[i];
    }
}
set<State> Nfa::get_states() const
{
    set<State> ret;
    for (auto i : transitions) {
        ret.insert(i.first);
    }
    return ret;
}

void Nfa::print(ostream &out) const
{
    out << initial_state << "\n";

    for (auto i : transitions) {
        for (auto j : i.second) {
            for (auto k : j.second) {
                out << i.first << " " << k << " " << int_to_hex(j.first)
                    << "\n";
            }
        }
    }

    for (auto i : final_states) {
        out << i << "\n";
    }
}


//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// implementation of NfaArray class methods
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

NfaArray::NfaArray(const Nfa &nfa) : Nfa{nfa}
{
    // map states
    State cnt = 0;
    state_map.clear();
    for (auto i : transitions)
        state_map[i.first] = cnt++;

    trans_vector = vector<vector<State>>(state_count() * alph_size);
    for (auto i : transitions) {
        size_t idx_state = state_map[i.first] << shift;
        for (auto j : i.second) {
            size_t symbol = j.first;
            for (auto state : j.second) {
                assert(idx_state + symbol < trans_vector.size());
                trans_vector[idx_state + symbol].push_back(state_map[state]);
            }
        }
    }
}

NfaArray::NfaArray(const NfaArray &nfa) : NfaArray{static_cast<Nfa>(nfa)}
{
}

/// Compute indexes mapped to the states labels of original NFA.
/// @return mapping of indexes mapped to the states labels
map<State,State> NfaArray::get_reversed_state_map() const
{
    map<State,State> ret;
    for (auto i : state_map)
    {
        ret[i.second] = i.first;
    }
    return ret;
}

/// Compute indexes of final states mapped to the states labels of original NFA.
/// @return mapping of indexes of final states mapped to the states labels
vector<State> NfaArray::get_final_state_idx() const
{
    vector<State> ret;
    for (auto i : final_states)
    {
        ret.push_back(state_map.at(i));
    }
    return ret;
}

/// Computes packet frequency over a giver string (payload).
/// @param state_freq mapping of indexes to state packet frequency
/// @param payload string data
/// @param len the length of payload
void NfaArray::label_states(
    vector<size_t> &state_freq, const unsigned char *payload,
    unsigned len) const
{
    vector<bool> bm(state_count());
    parse_word(payload, len, [&bm](State s){ bm[s] = 1; });
    for (size_t i = 0; i < state_freq.size(); i++)
        state_freq[i] += bm[i];

    state_freq[get_initial_state_idx()]++;
}
