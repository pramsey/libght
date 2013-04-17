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


/******************************************************************************/
/* GhtAttribute */

GhtErr ght_attribute_new(const GhtDimension *dim, double val, GhtAttribute **attr)
{
    GhtAttribute *a;
    a = ght_malloc(sizeof(GhtAttribute));
    if ( ! a ) return GHT_ERROR;
    memset(a, 0, sizeof(GhtAttribute));
    a->dim = dim;
    GHT_TRY(ght_attribute_set_value(a, val));
    *attr = a;
    return GHT_OK;
}

GhtErr ght_attribute_free(GhtAttribute *attr)
{
    if ( attr ) ght_free(attr);
    return GHT_OK;
}

/** Convert a real value double into a value suitable for storage */
static GhtErr
ght_attribute_real_to_storage(const GhtDimension *dim, double *val)
{
    if ( dim->offset )
    {
        *val -= dim->offset;
    }
    if ( dim->scale != 1 )
    {
        *val /= dim->scale;
    }
    return GHT_OK;
}

/** Convert a real value double into a value suitable for storage */
static GhtErr
ght_attribute_storage_to_real(const GhtDimension *dim, double *val)
{
    if ( dim->scale != 1 )
    {
        *val *= dim->scale;
    }
    if ( dim->offset )
    {
        *val += dim->offset;
    }
    return GHT_OK;
}

GhtErr ght_attribute_get_value(const GhtAttribute *attr, double *val)
{
    const GhtType type = attr->dim->type;
    size_t size = GhtTypeSizes[type];
    switch(type)
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
            break;
        }
        case GHT_UINT8:
        {
            uint8_t v;
            memcpy(&v, attr->val, size);
            *val = v;
            break;
        }
        case GHT_INT16:
        {
            int16_t v;
            memcpy(&v, attr->val, size);
            *val = v;
            break;
        }
        case GHT_UINT16:
        {
            uint16_t v;
            memcpy(&v, attr->val, size);
            *val = v;
            break;
        }
        case GHT_INT32:
        {
            int32_t v;
            memcpy(&v, attr->val, size);
            *val = v;
            break;
        }
        case GHT_UINT32:
        {
            uint32_t v;
            memcpy(&v, attr->val, size);
            *val = v;
            break;
        }
        case GHT_INT64:
        {
            int64_t v;
            memcpy(&v, attr->val, size);
            *val = v;
            break;
        }
        case GHT_UINT64:
        {
            uint64_t v;
            memcpy(&v, attr->val, size);
            *val = v;
            break;
        }
        case GHT_DOUBLE:
        {
            double v;
            memcpy(&v, attr->val, size);
            *val = v;
            break;
        }
        case GHT_FLOAT:
        {
            float v;
            memcpy(&v, attr->val, size);
            *val = v;
            break;
        }
        default:
        {
            return GHT_ERROR;
        }
    }
    GHT_TRY(ght_attribute_storage_to_real(attr->dim, val));
    return GHT_OK;    
}


GhtErr ght_attribute_set_value(GhtAttribute *attr, double val)
{
    const GhtType type = attr->dim->type;
    size_t size = GhtTypeSizes[type];
    double dv = val;
    
    GHT_TRY(ght_attribute_real_to_storage(attr->dim, &dv));
    
    switch(type)
    {
        case GHT_UNKNOWN:
        {
            return GHT_ERROR;
        }
        case GHT_INT8:
        {
            int8_t v = dv;
            memcpy(attr->val, &v, size);
            return GHT_OK;
        }
        case GHT_UINT8:
        {
            uint8_t v = dv;
            memcpy(attr->val, &v, size);
            return GHT_OK;
        }
        case GHT_INT16:
        {
            int16_t v = dv;
            memcpy(attr->val, &v, size);
            return GHT_OK;
        }
        case GHT_UINT16:
        {
            uint16_t v = dv;
            memcpy(attr->val, &v, size);
            return GHT_OK;
        }
        case GHT_INT32:
        {
            int32_t v = dv;
            memcpy(attr->val, &v, size);
            return GHT_OK;
        }
        case GHT_UINT32:
        {
            uint32_t v = dv;
            memcpy(attr->val, &v, size);
            return GHT_OK;
        }
        case GHT_INT64:
        {
            int64_t v = dv;
            memcpy(attr->val, &v, size);
            return GHT_OK;
        }
        case GHT_UINT64:
        {
            uint64_t v = dv;
            memcpy(attr->val, &v, size);
            return GHT_OK;
        }
        case GHT_DOUBLE:
        {
            double v = dv;
            memcpy(attr->val, &v, size);
            return GHT_OK;
        }
        case GHT_FLOAT:
        {
            float v = dv;
            memcpy(attr->val, &v, size);
            return GHT_OK;
        }
        default:
        {
            return GHT_ERROR;
        }
    }
    return GHT_OK;
}

GhtErr ght_attribute_to_string(const GhtAttribute *attr, stringbuffer_t *sb)
{
    double d;
    GHT_TRY(ght_attribute_get_value(attr, &d));
    stringbuffer_aprintf(sb, "%s=%g", attr->dim->name, d);
    return GHT_OK;
}


/******************************************************************************/
/* GhtAttributeList */

GhtErr ght_attributelist_new(GhtAttributeList **attrlist)
{
    GhtAttributeList *a = ght_malloc(sizeof(GhtAttributeList));
    if ( ! a ) return GHT_ERROR;
    memset(a, 0, sizeof(GhtAttributeList));
    *attrlist = a;
    return GHT_OK;
}

GhtErr ght_attributelist_free(GhtAttributeList *al)
{
    if ( ! al ) return GHT_OK;
    if ( al->attributes )
    {
        int i;
        for ( i = 0; i < al->num_attributes; i++ )
        {
            if ( al->attributes[i] )
                ght_attribute_free(al->attributes[i]);
        }
        ght_free(al->attributes);
    }
    ght_free(al);
    return GHT_OK;
}

GhtErr 
ght_attributelist_to_string(const GhtAttributeList *al, stringbuffer_t *sb)
{
    int i;
    for ( i = 0; i < al->num_attributes; i++ )
    {
        if ( i ) stringbuffer_append(sb, ":");
        ght_attribute_to_string(al->attributes[i], sb);
    }
    return GHT_OK;
}
    
GhtErr
ght_attributelist_add_attribute(GhtAttributeList *al, GhtAttribute *attr)
{
    if ( al->max_attributes == 0 )
    {
        al->max_attributes = 2;
        al->attributes = ght_malloc(al->max_attributes * sizeof(GhtAttribute*));
    }
    if ( al->num_attributes == al->max_attributes )
    {
        al->max_attributes *= 2;
        al->attributes = ght_realloc(al->attributes, al->max_attributes * sizeof(GhtAttribute*));
    }
    al->attributes[al->num_attributes] = attr;
    al->num_attributes++;
    return GHT_OK;
}

GhtErr
ght_attributelist_delete_attribute(GhtAttributeList *al, int i)
{
    size_t sz = sizeof(GhtAttribute*);
    
    if ( i < 0 || i >= al->num_attributes )
        return GHT_ERROR;
    
    /* We're going to have one less attribute */
    al->num_attributes--;
    
    /* Free the attribute we're deleting */
    ght_free(al->attributes[i]);
    
    /* If this removal creates a gap, fill it in */
    if ( i < al->num_attributes )
        memmove(al->attributes + i * sz, al->attributes + i * (sz+1), al->num_attributes - i);
        
    /* Null out the end of the array */
    al->attributes[al->num_attributes] = NULL;
    
    return GHT_OK;
}


