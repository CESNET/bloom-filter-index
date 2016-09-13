/**
 * \file bloomf_wrapper.h
 * \author Pavel Krobot <Pavel.Krobot@cesnet.cz>
 * \brief Bloom filter c++ implementation wrapper (header file).
 *
 * Copyright (C) 2016 CESNET, z.s.p.o.
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

#ifndef _BLOOMF_WRAPPER_H
#define _BLOOMF_WRAPPER_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Bloom filter parameters
typedef struct bloom_parameters bloom_parameters;
// Constructor
bloom_parameters *new_bloom_parameters();
// Some getters & setters
unsigned long long int bp_get_proj_elem_cnt (bloom_parameters* bp);
double bp_get_false_pos_prob (bloom_parameters *bp);
void bp_set_proj_elem_cnt (bloom_parameters* bp, unsigned long long int cnt);
void bp_set_false_pos_prob (bloom_parameters* bp, double prob);
// Public methods and operators
bool bp_not(bloom_parameters* bp);
bool bp_compute_optimal_parameters(bloom_parameters* bp);
void del_bloom_parameters(bloom_parameters *bp);


// Bloom filter
typedef struct bloom_filter bloom_filter;
// Constructors
bloom_filter *new_bloom_filter();
bloom_filter *new_bloom_filter_bp(bloom_parameters *bp);
bloom_filter *new_bloom_filter_f(bloom_filter *bf);
// Public methods and operators
void bf_clear(bloom_filter *bf);
bool bf_contains(bloom_filter *bf, const unsigned char* key_begin, const size_t *length);
void bf_insert(bloom_filter *bf, const unsigned char* key_begin, const size_t *length);
bool bf_containsinsert(bloom_filter *bf, const unsigned char* key_begin, const size_t *length);
size_t bf_element_count(bloom_filter *bf);
uint32_t bf_get_filter_as_bytes(bloom_filter *bf, char **buff);
int bf_load_filter_from_bytes(bloom_filter *bf, const char *buff, uint32_t len);
void bf_clear_bytes(bloom_filter *bf, char **buff);
void bf_delete_filter(bloom_filter *bf);
// Getter
unsigned int bf_get_inserted_element_cnt (bloom_filter* bf);

#ifdef __cplusplus
}
#endif

#endif //_BLOOMF_WRAPPER_H
