/**
 * \file bf_index.h
 * \author Pavel Krobot <Pavel.Krobot@cesnet.cz>
 * \brief Bloom filter indexes for IP addresses in flow data (header file)
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

#ifndef _BLOOM_FILTER_INDEX_H
#define _BLOOM_FILTER_INDEX_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

typedef enum {
    BFI_E_OK = 0,
    BFI_E_BP_COMP_PARAMS,
    BFI_E_NO_INDEX,
    BFI_E_STO_FILE_ERR,
    BFI_E_STO_BYTES,
    BFI_E_STO_MAGIC,
    BFI_E_STO_IDX_LEN,
    BFI_E_STO_INDEX,
    BFI_E_LOAD_MEM,
    BFI_E_LOAD_FILE_ERR,
    BFI_E_LOAD_BYTES,
    BFI_E_LOAD_MAGIC,
    BFI_E_LOAD_BAD_MAGIC,
    BFI_E_LOAD_IDX_LEN,
    BFI_E_LOAD_ZERO_LEN,
    BFI_E_LOAD_INDEX,
}bfi_ecode_t;

typedef void *bfi_index_ptr_t;

#if defined (__cplusplus)
extern "C" {
#endif

/**
 * \brief Print error message according to BFI error code
 *
 * \note Error is printed to stderr.
 * \param[in] ecode Bloom filter index error code
 */
const char *bfi_get_error_msg(bfi_ecode_t ecode);

/**
 * \brief Initialize Bloom filter index
 *
 * Computes optimal Bloom filter parameters and create new Bloom filter.
 * \param[in] index_ptr Pointer to Bloom filter index
 * \param[in] est_item_cnt Estimated count of items in Bloom filter
 * \param[in] fp_prob Required false positive probability of Bloom filter
 * \return Returns BFI_OK on success, error code otherwise.
 */
bfi_ecode_t bfi_init_index(bfi_index_ptr_t *index_ptr, uint64_t est_item_cnt,
							double fp_prob);

/**
 * \brief Destroy Bloom filter index
 *
 * \note Sets pointer to index to NULL.
 * \param[in] index_ptr Pointer to pointer to index to free
 */
void bfi_destroy_index(bfi_index_ptr_t *index_ptr);

/**
 * \brief Add item to Bloom filter
 *
 * Add an item to Bloom filter. If the item is already present, nothing
 * happens. To allow optimization of Bloom filter size based on
 * inserted_element_count_ variable (see BloomFilter.hpp) containsinsert() is
 * used instead of insert() since it increments this variable only if inserted
 * item is new to the filter (i.e. inserted_element_count_ is unique element
 * count).                              .
 * \param[in/out] index_ptr Bloom filter index
 * \param[in] buffer Buffer containing value to insert
 * \param[in] len Length of value in buffer
 * \return Returns BFI_OK on success, error code otherwise.
 */
bfi_ecode_t bfi_add_addr_index(bfi_index_ptr_t index_ptr,
                    const unsigned char *buffer, const size_t len);

/**
 * \brief Clear Bloom filter index.
 * \param[in] index_ptr Pointer to index structure to clear
 * \return Returns BFI_OK on success, error code otherwise.
 */
bfi_ecode_t bfi_clear_index(bfi_index_ptr_t index_ptr);

/**
 * \brief Check if address is contained in Bloom filter
 *
 * \param[in/out] index_ptr Bloom filter index
 * \param[in] buffer Buffer containing value to check
 * \param[in] len Length of value in buffer
 * \return True if value in the buffer is present in the Bloom filter, False
*    otherwise.
 */
bool bfi_addr_is_stored(bfi_index_ptr_t index_ptr, const unsigned char *buffer,
                    const size_t len);

/**
 * \brief Gets count of items stored in Bloom filter index
 *
 * \param[in] index_ptr Bloom filter index
 * \return Returns count of items.
 */
uint64_t bfi_stored_item_cnt(bfi_index_ptr_t index_ptr);

/**
 * \brief Store Bloom filter index to a file
 *
 * Stores index length and Bloom filter index structure itself. Binary
 * representation of stored Bloom filter is given by BloomFilter.hpp code.
 *
 * \note Every file begins with special 16-bit integer for format and endianity
 *   check.
 * \param[in] index_ptr Bloom filter index (index to store)
 * \param[in] filename Destination file path
 * \return Returns BFI_OK on success, error code otherwise.
 */
bfi_ecode_t bfi_store_index(bfi_index_ptr_t index_ptr, char *filename);

/**
 * \brief Load Bloom filter index from a file
 *
 * Load index length and Bloom filter index binary representation from the file.
 * Fill an index structure with loaded data.
 *
 * \note Every file begins with special 16-bit integer for format and endianity
 *   check.
 * \param[in] index_ptr Bloom filter index (where to load the index)
 * \param[in] filename Destination file path (load index from here)
 * \return Returns BFI_OK on success, error code otherwise.
 */
bfi_ecode_t bfi_load_index(bfi_index_ptr_t *index_ptr, char *filename);


#if defined (__cplusplus)
}
#endif

#endif //_BLOOM_FILTER_INDEX_H
