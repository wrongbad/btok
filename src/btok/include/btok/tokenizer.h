#pragma once

#include <cstdint>
#include <vector>

namespace btok {

struct Tokenizer
{
    struct Node
    {
        int token_id = -1;
        int next[256];
        Node() { for(int & i : next) { i = -1; } }
    };
    enum Error
    {
        ERR_OVERFLOW = -1,
        ERR_BUG = -999
    };

    std::vector<std::string> tokens;
    std::vector<Node> tree;

    Tokenizer()
    {
        _init(nullptr, 0);
    }

    Tokenizer(char const* pack, size_t pack_len)
    {
        _init(pack, pack_len);
    }

    void _init(char const* pack, size_t pack_len)
    {
        tree.push_back({}); // root node

        auto end = pack + pack_len;
        while(pack < end)
        {
            uint8_t len = *pack;
            add_token({pack+1, len});
            pack += len + 1;
        }
    }

    void add_token(std::string token)
    {
        int tok_id = tokens.size();
        tokens.push_back(std::move(token));

        int node = 0;
        for(int i=0 ; i<tokens[tok_id].size() ; i++)
        {
            uint8_t c = tokens[tok_id][i];
            if(tree[node].next[c] < 0)
            {
                tree[node].next[c] = tree.size();
                tree.push_back({});
            }
            node = tree[node].next[c];
        }
        tree[node].token_id = tok_id;
    }

    int num_tokens() const { return tokens.size(); }

    std::string const& token(int i) const { return tokens[i]; }

    template<class PutTok>
    int encode(char const* str, int str_len, PutTok && put_tok) const
    {
        int err = 0;
        int node = 0;
        int token = -1;
        int token_end = -1;
        for(int i=0 ; i<str_len ; )
        {
            int match = tree[node].token_id;
            if(match >= 0)
            {
                token = match;
                token_end = i;
            }

            uint8_t c = str[i];
            int next = tree[node].next[c];
            if(next >= 0)
            {
                node = next;
                i ++;
            }
            else if(token >= 0)
            {
                if(err = put_tok(token)) { return err; }
                i = token_end;
                node = 0;
                token = -1;
                token_end = -1;
            }
            else
            {
                return ERR_BUG;
            }
        }
        int match = tree[node].token_id;
        if(match >= 0)
        {
            if(err = put_tok(match)) { return err; }
        }
        return 0;
    }

    // return number of output tokens filled
    // return negative if failed
    int encode(char const* str, int str_len, int * out, int out_len) const
    {
        bool overflow = false;
        int out_fill = 0;
        int err = encode(str, str_len, [&] (int tok) {
            if(out_fill >= out_len) { return (int)ERR_OVERFLOW; }
            out[out_fill++] = tok;
            return 0;
        });
        if(err) return err;
        return out_fill;
    }


    template<class PutStr>
    int decode(int const* toks, int toks_len, PutStr && put_str) const
    {
        for(int i=0 ; i<toks_len ; i++)
        {
            auto & t = tokens[toks[i]];
            int err = put_str(t.data(), t.size());
            if(err) return err;
        }
        return 0;
    }

    // return number of output chars filled
    // return negative if failed
    int decode(int const* toks, int toks_len, char * out, int out_len) const
    {
        int out_fill = 0;
        int err = decode(toks, toks_len, [&] (char const* s, int l) {
            if(out_fill + l > out_len) { return (int)ERR_OVERFLOW; }
            std::copy(s, s + l, out + out_fill);
            out_fill += l;
            return 0;
        });
        if(err) return err;
        return out_fill;
    }

    std::vector<char> pack() const
    {
        std::vector<char> pack;
        for(int i=0 ; i<tokens.size() ; i++)
        {
            char len = char(uint8_t(tokens[i].size()));
            pack.push_back(len);
            for(char c : tokens[i]) { pack.push_back(c); }
        }
        return pack;
    }
};

} // namespace btok