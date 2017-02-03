/**
 * \file bf_index.c
 * \author Pavel Krobot <Pavel.Krobot@cesnet.cz>
 * \brief Bloom filter indexes for IP addresses in flow data
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

#include <string.h>
#include <stdint.h>

#include <ipfixcol.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>

#include "bf_index_internal.h"
#include "bloomf_wrapper.h"

static uint16_t BFI_FILE_MAGIC = BFI_MAGIC;

const char *bfi_get_error_msg(bfi_ecode_t ecode)
{
    return bfi_error_messages[ecode];
}

bfi_ecode_t bfi_init_index(bfi_index_ptr_t *index_ptr, uint64_t est_item_cnt,
							double fp_prob)
{
    struct bloom_parameters_h *bp = new_bloom_parameters();

    bp_set_false_pos_prob(bp, fp_prob);
    bp_set_proj_elem_cnt(bp, est_item_cnt);

    if (!bp_compute_optimal_parameters(bp)){
        return BFI_E_BP_COMP_PARAMS;
    }
    *index_ptr = (bfi_index_ptr_t) new_bloom_filter_bp(bp);

    del_bloom_parameters(bp);

    return BFI_E_OK;
}


void bfi_destroy_index(bfi_index_ptr_t *index_ptr)
{
    if (index_ptr) {
        bf_delete_filter(*index_ptr);
    }
    index_ptr = NULL;
}


bfi_ecode_t bfi_add_addr_index(bfi_index_ptr_t index_ptr,
                    const unsigned char *buffer, const size_t len)
{
	if (!index_ptr) {
    	return BFI_E_NO_INDEX;
	}

	bf_containsinsert(index_ptr, buffer, &len);

	return BFI_E_OK;
}


bfi_ecode_t bfi_clear_index(bfi_index_ptr_t index_ptr)
{
	if (!index_ptr) {
    	return BFI_E_NO_INDEX;
	}

    bf_clear(index_ptr);

    return BFI_E_OK;
}


bool bfi_addr_is_stored(bfi_index_ptr_t index_ptr, const unsigned char *buffer,
                    const size_t len)
{
	if (index_ptr) {
    	return bf_contains(index_ptr, buffer, &len);
	}

	return false;
}


uint64_t bfi_stored_item_cnt(bfi_index_ptr_t index_ptr){
	if (!index_ptr) {
		return 0;
	}

    return bf_get_inserted_element_cnt(index_ptr);
}


bfi_ecode_t bfi_store_index(bfi_index_ptr_t index_ptr, char *filename)
{
    uint32_t index_len;
    char *bf_bytes;
    FILE *bf_file_ptr;

    if (!index_ptr){
        // nothing to store
        return BFI_E_NO_INDEX;
    }

	// Open file, mode: write binary
    bf_file_ptr = fopen(filename, "wb");

    if (!bf_file_ptr){
        return BFI_E_STO_FILE_ERR;
    }

    // Get filter header and filter itself
    index_len = bf_get_filter_as_bytes(index_ptr, &bf_bytes);
    if (index_len == 0){
        return BFI_E_STO_BYTES;
    }

    // Write magic (to provide file format and endianity check in loading phase)
    if (fwrite(&BFI_FILE_MAGIC, sizeof(uint16_t), 1, bf_file_ptr) != 1){
        return BFI_E_STO_MAGIC;
    }

	// Write length of the index (size of byte array)
    if (fwrite(&index_len, sizeof(uint32_t), 1, bf_file_ptr) != 1){
        return BFI_E_STO_IDX_LEN;
    }

    // Write Bloom filter header and filter array
    if (fwrite(bf_bytes, sizeof(char), index_len, bf_file_ptr) != index_len){
        return BFI_E_STO_INDEX;
    }

    fclose(bf_file_ptr);

    bf_clear_bytes(index_ptr, &bf_bytes);

    return BFI_E_OK;
}


bfi_ecode_t bfi_load_index(bfi_index_ptr_t *index_ptr, char *filename)
{
    uint32_t index_len = 0;
    uint16_t magic_check;
    FILE *bf_file_ptr;

	// Open file, mode: read binary
    bf_file_ptr = fopen(filename, "rb");

    if (!bf_file_ptr){
        return BFI_E_LOAD_FILE_ERR;
    }

	// Create empty index
    *index_ptr = (bfi_index_ptr_t) new_bloom_filter();

	// Read and check magic value (format and endianity check)
    if (fread(&magic_check, sizeof(uint16_t), 1, bf_file_ptr) != 1) {
        return BFI_E_LOAD_IDX_LEN;
    }
    if (magic_check != BFI_FILE_MAGIC) {
		return BFI_E_LOAD_BAD_MAGIC;
    }

	// Read and check index size
    if (fread(&index_len, sizeof(uint32_t), 1, bf_file_ptr) != 1) {
        return BFI_E_LOAD_IDX_LEN;
    }
    if (index_len == 0){
        return BFI_E_LOAD_ZERO_LEN;
    }

	// Read index byte array
    char *index_bytes = (char *) malloc(index_len * sizeof(char));
    if (!index_bytes){
        return BFI_E_LOAD_MEM;
    }
    if (fread(index_bytes, sizeof(char), index_len, bf_file_ptr) != index_len){
        return BFI_E_LOAD_INDEX;
    }

	// Re-create index from loaded bytes (i.e. from index binary representation)
    if (bf_load_filter_from_bytes(*index_ptr, index_bytes, index_len) != 0){
        return BFI_E_LOAD_BYTES;
    }

    free(index_bytes);

    fclose(bf_file_ptr);

    return BFI_E_OK;
}
