#include <nanobind/nanobind.h>
#include <nanobind/stl/vector.h>

#include <sstream>
#include <string_view>

#include "btok/tokenizer.h"
#include "btok/trainer.h"

namespace nb = nanobind;
using namespace nb::literals;

NB_MODULE(btok_bind, m)
{
    using Tokenizer = btok::Tokenizer;
    nb::class_<Tokenizer>(m, "Tokenizer")
        .def(nb::init())
        .def("__init__", [] ( Tokenizer * self, nb::bytes pack) { 
            new (self) Tokenizer(pack.c_str(), pack.size());
        }, "tokenizer_pack"_a)
        .def("add_token", [] (Tokenizer & self, nb::bytes tok) {
            self.add_token({tok.c_str(), tok.size()});
        }, "token"_a)
        .def("num_tokens", &Tokenizer::num_tokens)
        .def("token", [] (Tokenizer const& self, int i) {
            auto const& t = self.token(i);
            return nb::bytes(t.data(), t.size());
        }, "i"_a)
        .def("encode", [] (Tokenizer const& self, nb::bytes text) {
            std::vector<int> out;
            self.encode(text.c_str(), text.size(), [&] (int tok) {
                out.push_back(tok);
                return 0;
            });
            return out;
        }, "text"_a)
        .def("decode", [] (Tokenizer const& self, std::vector<int> const& toks) {
            std::vector<char> text;
            self.decode(toks.data(), toks.size(), [&] (char const* s, int l) {
                for(char c : std::string_view(s, l))
                { 
                    text.push_back(c);
                }
                return 0;
            });
            return nb::bytes(text.data(), text.size());
        }, "tokens"_a)
        .def("pack", [] (Tokenizer const& self) {
            auto p = self.pack();
            return nb::bytes(p.data(), p.size());
        })
        ;

    using Trainer = btok::Trainer;
    nb::class_<Trainer>(m, "Trainer")
        .def(nb::init<int, bool, bool>(), 
            "n_tokens"_a,
            "whitespace_barrier"_a = false,
            "whitespace_unigram"_a = true
        )
        .def("add_token", [] (Trainer & self, nb::bytes tok) {
            self.add_token({tok.c_str(), tok.size()});
        }, "token"_a)
        .def("num_tokens", &Trainer::num_tokens)
        .def("token", [] (Trainer const& self, int i) {
            auto & t = self.token(i);
            return nb::bytes(t.data(), t.size());
        }, "i"_a)
        .def("update", [] (Trainer & self, nb::bytes data, int add) {
            return self.update({data.c_str(), data.size()}, add);
        }, "data"_a, "tokens_to_add"_a = 1)
        .def_prop_ro("tokenizer", [] (Trainer & self) { return self.tokenizer; })
        ;
}