/*
 * $Id: json.h,v 1.6 2006/01/26 02:16:28 mclark Exp $
 *
 * Copyright (c) 2004, 2005 Metaparadigm Pte. Ltd.
 * Michael Clark <michael@metaparadigm.com>
 * Copyright (c) 2009 Hewlett-Packard Development Company, L.P.
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 */

/**
 * @file
 * @brief A convenience header that may be included instead of other individual ones.
 */
#ifndef _json_h_
#define _json_h_

#if defined(__clang__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wpadded"
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wreserved-id-macro"
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wdocumentation-deprecated-sync"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "arraylist.h"
#include "debug.h"
#include "json_c_version.h"
#include "json_object.h"
#include "json_object_iterator.h"
#include "json_pointer.h"
#include "json_tokener.h"
#include "json_util.h"
#include "linkhash.h"

#ifdef __cplusplus
}
#endif

#if defined(__clang__)
#  pragma GCC diagnostic pop
#  pragma GCC diagnostic pop
#  pragma GCC diagnostic pop
#endif

#endif
