#pragma once

#include <vector>
#include <cctype>

#include "btok/tokenizer.h"

namespace btok {

struct Trainer
{
    int max_tokens;
    bool whitespace_barrier;
    bool whitespace_unigram;

    btok::Tokenizer tokenizer;
    std::vector<int> matrix;

    Trainer(
        int n_tokens, 
        bool whitespace_barrier=false,
        bool whitespace_unigram=true)
    :   max_tokens(n_tokens),
        whitespace_barrier(whitespace_barrier),
        whitespace_unigram(whitespace_unigram)
    {
        for(int i=0 ; i<256 ; i++)
        {
            add_token(std::string(1, char(i)));
        }
        matrix.reserve(n_tokens * n_tokens);
    }

    void add_token(std::string t)
    {
        tokenizer.add_token(std::move(t));
    }

    int num_tokens() const { return tokenizer.num_tokens(); }
    std::string const& token(int i) const { return tokenizer.token(i); }

    int update(std::string_view data, int tokens_to_add = 1)
    {
        tokens_to_add = std::min(tokens_to_add, max_tokens - num_tokens());
        if(tokens_to_add == 0) { return 0; }

        int ntok = num_tokens();
        matrix.resize(ntok * ntok);
        std::fill(matrix.begin(), matrix.end(), 0);

        int prev_tok = 0;
        tokenizer.encode(data.data(), data.size(), [&] (int tok) {
            matrix[tok * ntok + prev_tok] += 1;
            prev_tok = tok;
            return 0;
        });

        int added=0;
        for( ; added<tokens_to_add ; added++)
        {
            int max_i = 0;
            int max_v = 0;
            for(int i=0 ; i<matrix.size() ; i++)
            {
                int l = i % ntok;
                int r = i / ntok;
                if(whitespace_unigram)
                {
                    if(l<255 && std::isspace(l)) continue;
                    if(r<255 && std::isspace(r)) continue;
                }
                else if(whitespace_barrier)
                {
                    // if(token(l).size() > 1 && std::isspace(token(l).back())) continue;
                    // if(token(r).size() > 1 && std::isspace(token(r).front())) continue;
                    if(std::isspace(token(r)[0])) continue;
                }
                if(matrix[i] > max_v)
                {
                    max_v = matrix[i];
                    max_i = i;
                }
            }
            if(max_v == 0) { break; }
            matrix[max_i] = 0;

            int l = max_i % ntok;
            int r = max_i / ntok;
            add_token(token(l) + token(r));
        }
        return added;
    }
};

} // namespace btok