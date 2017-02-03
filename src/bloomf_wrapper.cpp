/**
 * \file bloomf_wrapper.cpp
 * \author Pavel Krobot <Pavel.Krobot@cesnet.cz>
 * \brief Bloom filter c++ implementation wrapper
 *
 * Copyright (C) 2016, 2017 CESNET, z.s.p.o.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of the Company nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * ALTERNATIVELY, provided that this notice is retained in full, this
 * product may be distributed under the terms of the GNU General Public
 * License (GPL) version 2 or later, in which case the provisions
 * of the GPL apply INSTEAD OF those given above.
 *
 * This software is provided ``as is, and any express or implied
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose are disclaimed.
 * In no event shall the company or contributors be liable for any
 * direct, indirect, incidental, special, exemplary, or consequential
 * damages (including, but not limited to, procurement of substitute
 * goods or services; loss of use, data, or profits; or business
 * interruption) however caused and on any theory of liability, whether
 * in contract, strict liability, or tort (including negligence or
 * otherwise) arising in any way out of the use of this software, even
 * if advised of the possibility of such damage.
 *
 */

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "bloomf_wrapper.h"
#include "BloomFilter.hpp"

extern "C" {
    // Bloom filter parameters /////////////////////////////////////////////////
    // Constructor
    bloom_parameters_h *new_bloom_parameters()
    {
        return reinterpret_cast<bloom_parameters_h *>(new bloom_parameters);
    }

    // Some getters & setters
    unsigned long long int bp_get_proj_elem_cnt (bloom_parameters_h* bp)
    {
        return reinterpret_cast<bloom_parameters *>(bp)->projected_element_count;
    }

    double bp_get_false_pos_prob (bloom_parameters_h *bp)
    {
        return reinterpret_cast<bloom_parameters *>(bp)->false_positive_probability;
    }

    void bp_set_proj_elem_cnt (bloom_parameters_h* bp, unsigned long long int cnt)
    {
        reinterpret_cast<bloom_parameters *>(bp)->projected_element_count = cnt;
    }

    void bp_set_false_pos_prob (bloom_parameters_h* bp, double prob)
    {
        reinterpret_cast<bloom_parameters *>(bp)->false_positive_probability = prob;
    }

    // Public methods and operators
    bool bp_not(bloom_parameters_h* bp)
    {
        return !(*(reinterpret_cast<bloom_parameters *>(bp)));
    }

    bool bp_compute_optimal_parameters(bloom_parameters_h* bp)
    {
        return reinterpret_cast<bloom_parameters *>(bp)->compute_optimal_parameters();
    }

    void del_bloom_parameters(bloom_parameters_h *bp)
    {
        delete reinterpret_cast<bloom_parameters *>(bp);
    }

    // Bloom filter ////////////////////////////////////////////////////////////
    // Constructors
    bloom_filter_h *new_bloom_filter()
    {
        return reinterpret_cast<bloom_filter_h *>(new bloom_filter());
    }

    bloom_filter_h *new_bloom_filter_bp(bloom_parameters_h *bp)
    {
        return reinterpret_cast<bloom_filter_h *>(new bloom_filter(*(reinterpret_cast<bloom_parameters *>(bp))));
    }

    bloom_filter_h *new_bloom_filter_f(bloom_filter_h *bf)
    {
        return reinterpret_cast<bloom_filter_h *>(new bloom_filter(
                const_cast<bloom_filter&>(*(reinterpret_cast<bloom_filter*>(bf)))));
    }

    // Public methods and operators
    void bf_clear(bloom_filter_h *bf)
    {
        reinterpret_cast<bloom_filter*>(bf)->clear();
    }

    bool bf_contains(bloom_filter_h *bf, const unsigned char* key_begin, const size_t *length)
    {
        return reinterpret_cast<bloom_filter*>(bf)->contains(key_begin, *length);
    }

    void bf_insert(bloom_filter_h *bf, const unsigned char* key_begin, const size_t *length)
    {
        reinterpret_cast<bloom_filter*>(bf)->insert(key_begin, *length);
    }

    bool bf_containsinsert(bloom_filter_h *bf, const unsigned char* key_begin, const size_t *length)
    {
        return reinterpret_cast<bloom_filter*>(bf)->containsinsert(key_begin, *length);
    }

    std::size_t bf_element_count(bloom_filter_h *bf)
    {
        return reinterpret_cast<bloom_filter*>(bf)->element_count();
    }

    uint32_t bf_get_filter_as_bytes(bloom_filter_h *bf, char **buff)
    {
        return reinterpret_cast<bloom_filter*>(bf)->get_filter_as_bytes(buff);
    }

    int bf_load_filter_from_bytes(bloom_filter_h *bf, const char *buff, uint32_t len)
    {
        return reinterpret_cast<bloom_filter*>(bf)->load_filter_from_bytes(buff, len);
    }

    void bf_clear_bytes(bloom_filter_h *bf, char **buff)
    {
        return reinterpret_cast<bloom_filter*>(bf)->clear_bytes(buff);
    }

    void bf_delete_filter(bloom_filter_h *bf)
    {
        delete reinterpret_cast<bloom_filter*>(bf);
    }

    // Getter
    uint64_t bf_get_inserted_element_cnt (bloom_filter_h *bf)
    {
        return reinterpret_cast<bloom_filter*>(bf)->get_inserted_element_count();
    }
}
