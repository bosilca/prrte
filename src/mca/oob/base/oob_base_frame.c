/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2004-2005 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2005 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2004-2005 High Performance Computing Center Stuttgart,
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
 * Copyright (c) 2007-2020 Cisco Systems, Inc.  All rights reserved
 * Copyright (c) 2013-2017 Los Alamos National Security, LLC. All rights
 *                         reserved.
 * Copyright (c) 2015-2019 Research Organization for Information Science
 *                         and Technology (RIST).  All rights reserved.
 * Copyright (c) 2017-2019 Intel, Inc.  All rights reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */


#include "prte_config.h"
#include "constants.h"

#include "src/class/prte_bitmap.h"
#include "src/mca/mca.h"
#include "src/runtime/prte_progress_threads.h"
#include "src/util/output.h"
#include "src/mca/base/base.h"

#include "src/mca/rml/base/base.h"
#include "src/mca/oob/base/base.h"

/*
 * The following file was created by configure.  It contains extern
 * statements and the definition of an array of pointers to each
 * component's public prte_mca_base_component_t struct.
 */

#include "src/mca/oob/base/static-components.h"

/*
 * Global variables
 */
prte_oob_base_t prte_oob_base = {0};

static int prte_oob_base_close(void)
{
    prte_oob_base_component_t *component;
    prte_mca_base_component_list_item_t *cli;
    prte_object_t *value;
    uint64_t key;

    /* shutdown all active transports */
    while (NULL != (cli = (prte_mca_base_component_list_item_t *) prte_list_remove_first (&prte_oob_base.actives))) {
        component = (prte_oob_base_component_t*)cli->cli_component;
        if (NULL != component->shutdown) {
            component->shutdown();
        }
        PRTE_RELEASE(cli);
    }

    /* destruct our internal lists */
    PRTE_DESTRUCT(&prte_oob_base.actives);

    /* release all peers from the hash table */
    PRTE_HASH_TABLE_FOREACH(key, uint64, value, &prte_oob_base.peers) {
        if (NULL != value) {
            PRTE_RELEASE(value);
        }
    }

    PRTE_DESTRUCT(&prte_oob_base.peers);

    return prte_mca_base_framework_components_close(&prte_oob_base_framework, NULL);
}

/**
 * Function for finding and opening either all MCA components,
 * or the one that was specifically requested via a MCA parameter.
 */
static int prte_oob_base_open(prte_mca_base_open_flag_t flags)
{
    /* setup globals */
    prte_oob_base.max_uri_length = -1;
    PRTE_CONSTRUCT(&prte_oob_base.peers, prte_hash_table_t);
    prte_hash_table_init(&prte_oob_base.peers, 128);
    PRTE_CONSTRUCT(&prte_oob_base.actives, prte_list_t);

     /* Open up all available components */
    return prte_mca_base_framework_components_open(&prte_oob_base_framework, flags);
}

PRTE_MCA_BASE_FRAMEWORK_DECLARE(prte, oob, "Out-of-Band Messaging Subsystem",
                                 NULL, prte_oob_base_open, prte_oob_base_close,
                                 prte_oob_base_static_components, 0);


PRTE_CLASS_INSTANCE(prte_oob_send_t,
                   prte_object_t,
                   NULL, NULL);

static void pr_cons(prte_oob_base_peer_t *ptr)
{
    ptr->component = NULL;
    PRTE_CONSTRUCT(&ptr->addressable, prte_bitmap_t);
    prte_bitmap_init(&ptr->addressable, 8);
}
static void pr_des(prte_oob_base_peer_t *ptr)
{
    PRTE_DESTRUCT(&ptr->addressable);
}
PRTE_CLASS_INSTANCE(prte_oob_base_peer_t,
                   prte_object_t,
                   pr_cons, pr_des);
