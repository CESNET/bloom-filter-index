/**
 * \file bf_index.c
 * \author Pavel Krobot <Pavel.Krobot@cesnet.cz>
 * \brief Bloom filter indexes for IP addresses in flow data
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

#include <string.h>
#include <stdint.h>

#include <ipfixcol.h>
#include <unistd.h>
#include <stdbool.h>

#include "bf_index_internal.h"
#include "bloomf_wrapper.h"

/**
 * Holds error code and detailed description of last error.
 */
struct {
    bfi_error_codes_t ecode;
    char err_detail[1024];
}last_error_desc;


/**
 * \brief Print b error message to stderr.
 *
 * \param[in] ecode Bloom filter index library error code.
 */
void print_last_index_error()
{
    fprintf(stderr, "BFIndex: %s: %s (%s:%d)\n",
            bfi_error_messages[last_error_desc.ecode],
            last_error_desc.err_detail,
            __FILE__, __LINE__);
}

/**
 * \brief Create empty index structure.
 *
 * Create empty index structure, which should be subsequently initialized by
 * init_index() function or filled by load_index() function. Created structure
 * should be deleted by destroy_index() function.
 * \return As malloc(): on success returns pointer to empty index_t structure,
 *         on error returns NULL.
 */
index_t *create_index()
{
    return (index_t *) malloc(sizeof(index_t));
}

/**
 * \brief Initialize index.
 *
 * Set false positive probability and element count, compute optimal parameters.
 * Initialize bloom filter index according to computed parameters.
 * \param[in] bp_conf_par Configuration structure - contains parsed parameters.
 * \param[out] index Contains index structure to initialize.
 * \return On success returns 0, on error returns error code 1.
 */
int init_index(struct index_params bp_conf_par, index_t *index)
{
    index->bf_fname = NULL;

    struct bloom_parameters *bp = new_bloom_parameters();

    bp_set_false_pos_prob(bp, bp_conf_par.fp_prob);
    bp_set_proj_elem_cnt(bp, bp_conf_par.est_item_cnt);

    if (!bp_compute_optimal_parameters(bp)){
//      fprint(stderr, "Error: cannot compute optimal params.\n");
        return BFI_BF_ERR;
    }
    index->bf_ip = new_bloom_filter_bp(bp);

    del_bloom_parameters(bp);

    return BFI_OK;
}

/**
 * \brief Destroy index
 *
 * Destroy created index structure.
 * \param[in] index Pointer to index structure to free.
 */
void destroy_index(index_t *index)
{
    if (index) {
        if (index->bf_fname){
            free(index->bf_fname);
        }
        bf_delete_filter(index->bf_ip);
        free(index);
    }
}

/**
 * \brief Add address to bloom filter
 *
 * Add ip address to bloom filter. If address is already present, nothing
 * happens. To allow optimization of bloom filter size based on
 * inserted_element_count_ variable (see BloomFilter.hpp) containsinsert() is
 * used instead of insert() since it increments this variable only if inserted
 * item is new to the filter (i.e. inserted_element_count_ is unique element
 * count).                              .
 * \param[in/out] index Pointer to index structure (contains bloom filter).
 * \param[in] buffer Pointer to buffer containing value to insert (ip address)
 * \param[in] len Length of value in buffer
 */
void add_addr_index(index_t *index, const unsigned char *buffer,
                    const size_t len)
{
    bf_containsinsert(index->bf_ip, buffer, &len);
}

/**
 * \brief Gets count of items stored in bloom filter index
 *
 * \param[in] index Pointer to index structure (contains bloom filter).
 * \return Returns count of items.
 */
unsigned int stored_item_cnt(index_t *index){
    return bf_get_inserted_element_cnt(index->bf_ip);
}

/**
 * \brief Clear index
 *
 * Clear bloom filter index.
 * \param[in] index Pointer to index structure to clear.
 */
void clear_index(index_t *index)
{
    bf_clear(index->bf_ip);
}


void set_index_filename(index_t *index, char *filename){
    if (index->bf_fname){
        free(index->bf_fname);
    }

    index->bf_fname = filename;
}


/**
 * \brief Store bloom filter index to file
 *
 * Stores //TODO>>//magic//<<//, index length and bloom filter index structure
 * itself. Structure of stored bloom filter is given by BloomFilter.hpp code.
 * \param[in] index Pointer to index structure (contains bloom filter and
 * pointer to output file).
 * \return On success returns 0, on error returns error code.
 */
int store_index(index_t *index)
{
    uint32_t index_len;
    char *bf_bytes;
    FILE *bf_file_ptr;

    if (!index){
        // nothing to store
        sprintf(last_error_desc.err_detail,
                "No index passed, nothing to store.");
        last_error_desc.ecode = BFI_OK;
        return BFI_OK;
    }

    bf_file_ptr = fopen(index->bf_fname, "wb");

    if (!bf_file_ptr){
        sprintf(last_error_desc.err_detail,
                "Failed to create new index file '%s'", index->bf_fname);
        last_error_desc.ecode = BFI_FILE_ERR;
        return BFI_FILE_ERR;
    }

    // Get filter header and filter itself
    index_len = bf_get_filter_as_bytes(index->bf_ip, &bf_bytes);
    if (index_len == 0){
        sprintf(last_error_desc.err_detail,
                "Unable to get index bytes (file %s)", index->bf_fname);
        last_error_desc.ecode = BFI_BF_ERR;
        return BFI_BF_ERR;
    }

    /// TODO write magic

    if (fwrite(&index_len, sizeof(uint32_t), 1, bf_file_ptr) != 1){
        sprintf(last_error_desc.err_detail,
                "Unable to write index header (file %s)", index->bf_fname);
        last_error_desc.ecode = BFI_FILE_ERR;
        return BFI_FILE_ERR;
    }

    // Write bloom filter header and filter array
    if (fwrite(bf_bytes, sizeof(char), index_len, bf_file_ptr) != index_len){
        sprintf(last_error_desc.err_detail,
                "Unable to write index (file %s)", index->bf_fname);
        last_error_desc.ecode = BFI_FILE_ERR;
        return BFI_FILE_ERR;
    }

    fclose(bf_file_ptr);

    bf_clear_bytes(index->bf_ip, &bf_bytes);

    return BFI_OK;
}

/**
 * \brief Load bloom filter index from file
 *
 * Load bloom filter index from file and store it into a index structure.
 * \param[out] index Pointer to index structure.
 * \param[in] check_only Flag: if true, check mode is on: it is not error if
 *            file does not exists
 * \return On success returns 0, on error returns error code.
 */
int load_index(index_t *index)
{
    uint32_t index_len = 0;
    FILE *bf_file_ptr;

    bf_file_ptr = fopen(index->bf_fname, "rb");

    if (!bf_file_ptr){
        sprintf(last_error_desc.err_detail,
                "Failed to load index from file '%s'", index->bf_fname);
        last_error_desc.ecode = BFI_FILE_ERR;
        return BFI_FILE_ERR;
    }

    /// TODO read MAGIC

    if (fread(&index_len, sizeof(uint32_t), 1, bf_file_ptr) != 1) {
        sprintf(last_error_desc.err_detail,
                "Unable to read index (file %s)", index->bf_fname);
        last_error_desc.ecode = BFI_FILE_ERR;
        return BFI_FILE_ERR;
    }

    if (index_len == 0){
        sprintf(last_error_desc.err_detail,
                "Read empty index (file %s)", index->bf_fname);
        last_error_desc.ecode = BFI_INDEX_ERR;
        return BFI_INDEX_ERR;
    }

    char *index_bytes = (char *) malloc(index_len * sizeof(char));
    if (!index_bytes){
        sprintf(last_error_desc.err_detail,
                "Unable to allocate memory (file %s)", index->bf_fname);
        last_error_desc.ecode = BFI_MEM_ERR;
        return BFI_MEM_ERR;
    }

    if (fread(index_bytes, sizeof(char), index_len, bf_file_ptr)
        != index_len){
        sprintf(last_error_desc.err_detail,
                "Unable to read index size (file %s)", index->bf_fname);
        last_error_desc.ecode = BFI_FILE_ERR;
        return BFI_FILE_ERR;
    }

    if (bf_load_filter_from_bytes(index->bf_ip, index_bytes, index_len) != 0){
        sprintf(last_error_desc.err_detail,
                "Unable to load index (file %s)", index->bf_fname);
        last_error_desc.ecode = BFI_BF_ERR;
        return BFI_BF_ERR;
    }

    free(index_bytes);

    fclose(bf_file_ptr);

    return BFI_OK;
}
