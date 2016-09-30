/**
 * \file bf_internal_index.h
 * \author Pavel Krobot <Pavel.Krobot@cesnet.cz>
 * \brief Bloom filter indexes for IP addresses in flow data (header file)
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

#ifndef _BLOOMF_INDEXES_INTERNAL_H
#define _BLOOMF_INDEXES_INTERNAL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "bloomf_wrapper.h"

typedef enum {
    BFI_OK = 0,
    BFI_INDEX_ERR,
    BFI_BF_ERR,
    BFI_MEM_ERR,
    BFI_FILE_ERR,
}bfi_error_codes_t;

char *bfi_error_messages [] = {
    "OK",
    "Index error",
    "Bloom filter error",
    "Memory error",
    "File error",
};

/**
 * \brief Structure for bloom filter configuration.
 */
struct index_params {
    char *file_prefix;      /**< Bloom filter index file prefix - every bloom
                                filter index file starts with this prefix. The
                                rest of filename conventions should be as same
                                as in the case of data files.                 */
    bool indexing;          /**< Marks that bloom filter indexing is requested*/

    unsigned long long int est_item_cnt; /**< Estimated item count in bloom
                                              filter                          */
    double fp_prob;         /**< False positive probability of bloom filter   */
};

/**
 * \brief Structure for index.
 *
 *  Following index is implemented:
 *  - one bloom filter for IP addresses for both directions (source/destination)
 */
typedef struct index_s {
    struct bloom_filter *bf_ip;     /**< Bloom filter for source and destination
                                         IP addresses                         */
    char *bf_fname;                 /**< Filename for bloom filter storing or
                                         loading.                             */
}index_t;

void print_last_index_error();

index_t *create_index();
int init_index(struct index_params bp_conf_par, index_t *index);
void destroy_index(index_t *index);

void add_addr_index(index_t *index, const unsigned char *buffer,
                    const size_t len);
bool addr_contains(index_t *index, const unsigned char *buffer,
                    const size_t len);
void clear_index(index_t *index);
void set_index_filename(index_t *index, char *filename);
unsigned int stored_item_cnt(index_t *index);

int store_index(index_t *index);
int load_index(index_t *index);

#endif //_BLOOMF_INDEXES_INTERNAL_H
