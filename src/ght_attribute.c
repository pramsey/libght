/******************************************************************************
*  LibGHT, software to manage point clouds.
*  LibGHT is free and open source software provided by the Government of Canada
*  Copyright (c) 2012 Natural Resources Canada
*
*  Nouri Sabo <nsabo@NRCan.gc.ca>, Natural Resources Canada
*  Paul Ramsey <pramsey@opengeo.org>, OpenGeo
*
******************************************************************************/

#include "ght_internal.h"
#include <strings.h>

GhtErr ght_type_size(GhtType type, size_t *size)
{
    assert(size);
    assert(type);
    *size = GhtTypeSizes[type];
    return GHT_OK;
}

GhtErr ght_type_from_str(const char *str, GhtType *type)
{
    int i;
    for ( i = 0; i < GHT_NUM_TYPES; i++ )
    {
        if ( strcasecmp(str, GhtTypeStrings[i]) == 0 )
        {
            *type = i;
            return GHT_OK;
        }
    }
    return GHT_ERROR;
}

GhtErr ght_attribute_get_value(const GhtAttribute *attr, double *val)
{
    const GhtDimension *dim = attr->dim;
    size_t size = GhtTypeSizes[dim->type];
    switch(dim->type)
    {
        case GHT_UNKNOWN:
        {
            return GHT_ERROR;
        }
        case GHT_INT8:
        {
            int8_t v;
            memcpy(&v, attr->val, size);
            *val = v;
            return GHT_OK;
        }
        case GHT_UINT8:
        {
            uint8_t v;
            memcpy(&v, attr->val, size);
            *val = v;
            return GHT_OK;
        }
        case GHT_INT16:
        {
            int16_t v;
            memcpy(&v, attr->val, size);
            *val = v;
            return GHT_OK;
        }
        case GHT_UINT16:
        {
            uint16_t v;
            memcpy(&v, attr->val, size);
            *val = v;
            return GHT_OK;
        }
        case GHT_INT32:
        {
            int32_t v;
            memcpy(&v, attr->val, size);
            *val = v;
            return GHT_OK;
        }
        case GHT_UINT32:
        {
            uint32_t v;
            memcpy(&v, attr->val, size);
            *val = v;
            return GHT_OK;
        }
        case GHT_INT64:
        {
            int64_t v;
            memcpy(&v, attr->val, size);
            *val = v;
            return GHT_OK;
        }
        case GHT_UINT64:
        {
            uint64_t v;
            memcpy(&v, attr->val, size);
            *val = v;
            return GHT_OK;
        }
        case GHT_DOUBLE:
        {
            double v;
            memcpy(&v, attr->val, size);
            *val = v;
            return GHT_OK;
        }
        case GHT_FLOAT:
        {
            float v;
            memcpy(&v, attr->val, size);
            *val = v;
            return GHT_OK;
        }
        
    }
}


GhtErr ght_attribute_set_value(GhtAttribute *attr, double val)
{
    const GhtDimension *dim = attr->dim;
    size_t size = GhtTypeSizes[dim->type];
    switch(dim->type)
    {
        case GHT_UNKNOWN:
        {
            return GHT_ERROR;
        }
        case GHT_INT8:
        {
            int8_t v;
            memcpy(attr->val, &v, size);
            return GHT_OK;
        }
        case GHT_UINT8:
        {
            uint8_t v;
            memcpy(attr->val, &v, size);
            return GHT_OK;
        }
        case GHT_INT16:
        {
            int16_t v;
            memcpy(attr->val, &v, size);
            return GHT_OK;
        }
        case GHT_UINT16:
        {
            uint16_t v;
            memcpy(attr->val, &v, size);
            return GHT_OK;
        }
        case GHT_INT32:
        {
            int32_t v;
            memcpy(attr->val, &v, size);
            return GHT_OK;
        }
        case GHT_UINT32:
        {
            uint32_t v;
            memcpy(attr->val, &v, size);
            return GHT_OK;
        }
        case GHT_INT64:
        {
            int64_t v;
            memcpy(attr->val, &v, size);
            return GHT_OK;
        }
        case GHT_UINT64:
        {
            uint64_t v;
            memcpy(attr->val, &v, size);
            return GHT_OK;
        }
        case GHT_DOUBLE:
        {
            double v;
            memcpy(attr->val, &v, size);
            return GHT_OK;
        }
        case GHT_FLOAT:
        {
            float v;
            memcpy(attr->val, &v, size);
            return GHT_OK;
        }
        
    }
}