/*
 * Copyright (C) 2015-2020 Alibaba Group Holding Limited
 *
 */
#include "string.h"
#include "uai_odla.h"

int32_t uai_get_total_elements(const odla_dims dims)
{
    int32_t i    = 0;
    int32_t size = 1;

    for (i = 0; i < dims->size; i++) {
        size *= dims->dims[i];
    }

    return size;
}

/* NHWC*/
uai_tensor_s *uai_odla_tensor_transofrm(uai_tensor_s *input, const odla_value value, odla_dims dims, odla_element_type type, odla_layout layout)
{
    if(dims->size > UAI_MAX_DIMENSION) {
        return NULL;
    }
    if((layout != CHANNELS_LAST) && (layout != SIO)) {
        return NULL;
    }

    if (layout == CHANNELS_LAST) {
        if (dims->size == 1) {
            input->dims.size = 4;
            input->dims.dims[0] = 1;
            input->dims.dims[1] = 1;
            input->dims.dims[2] = dims->dims[0];
            input->dims.dims[3] = 1;
        } else if (dims->size == 2) {
            input->dims.size = 4;
            input->dims.dims[0] = 1;
            input->dims.dims[1] = dims->dims[0];
            input->dims.dims[2] = dims->dims[1];
            input->dims.dims[3] = 1;
        } else if (dims->size == 3) {
            input->dims.size = 4;
            input->dims.dims[0] = 1;
            input->dims.dims[1] = dims->dims[0];
            input->dims.dims[2] = dims->dims[1];
            input->dims.dims[3] = dims->dims[2];
        }
        input->dims.format = UAI_NHWC;
    }

    if ((dims->size > 3) || (layout != CHANNELS_LAST)) {
        input->dims.size   = dims->size;
        input->dims.format = (layout == CHANNELS_LAST) ? UAI_NHWC : UAI_SIO;
        memcpy(input->dims.dims, dims->dims, dims->size * sizeof(size_t));
    }

    input->size   = uai_get_total_elements(dims);
    input->buffer = value;
    input->dtype  = type;

    return input;
}

uai_mem_list_t *uai_odla_init_memlist()
{
    uai_mem_list_t *mem_head = (uai_mem_list_t *)uai_zalloc(sizeof(uai_mem_list_t));
    UAI_VALID_PTR_CHECK_NULL(mem_head);

    mem_head->next    = NULL;
    mem_head->tail    = NULL;
    mem_head->mem_ptr = NULL;

    return mem_head;
}

int uai_odla_add_memlist(uai_mem_list_t *memlist, void *ptr)
{
    uai_mem_list_t *new_mem = (uai_mem_list_t *)uai_zalloc(sizeof(uai_mem_list_t));

    UAI_VALID_PTR_CHECK_INT(new_mem, UAI_FAIL);
    new_mem->mem_ptr = ptr;

    if(memlist->next == NULL) {
        memlist->next = new_mem;
        memlist->tail = new_mem;
    } else {
        memlist->tail->next = new_mem;
        memlist->tail       = new_mem;
    }

    return 0;
}

void uai_odla_free_from_memlist(uai_mem_list_t *memlist, odla_value value)
{
    uai_mem_list_t *node = memlist;

    while(node) {
        if(node->mem_ptr == value) {
            uai_free(node->mem_ptr);
            node->mem_ptr = NULL;
            break;
        }
        node = node->next;
    }
}

void uai_odla_free_memlist(uai_mem_list_t *memlist)
{
    uai_mem_list_t *node = memlist->next;
    uai_mem_list_t *next = NULL;

    while(node) {
        if(node->mem_ptr) {
            uai_free(node->mem_ptr);
        }
        next = node->next;
        uai_free(node);
        node = next;
    }
    memlist->next = NULL;
}

