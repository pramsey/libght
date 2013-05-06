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

GhtErr ght_attribute_new_from_double(const GhtDimension *dim, double val, GhtAttribute **attr)
{
    GhtAttribute *a;
    a = ght_malloc(sizeof(GhtAttribute));
    if ( ! a ) return GHT_ERROR;
    memset(a, 0, sizeof(GhtAttribute));
    a->dim = dim;
    a->next = NULL;
    GHT_TRY(ght_attribute_set_value(a, val));
    *attr = a;
    return GHT_OK;
}

GhtErr ght_attribute_new_from_bytes(const GhtDimension *dim, uint8_t *bytes, GhtAttribute **attr)
{
    GhtAttribute *a;
    a = ght_malloc(sizeof(GhtAttribute));
    if ( ! a ) return GHT_ERROR;
    memset(a, 0, sizeof(GhtAttribute));
    a->dim = dim;
    a->next = NULL;
    memcpy(a->val, bytes, GhtTypeSizes[dim->type]);
    *attr = a;
    return GHT_OK;
}

GhtErr
ght_attribute_free(GhtAttribute *attr)
{
    /* No-op on null */
    if ( ! attr ) return GHT_OK;
    
    if ( attr->next )
    {
        ght_attribute_free(attr->next);
    }
    ght_free(attr);
    return GHT_OK;
}

GhtErr
ght_attribute_get_next(const GhtAttribute *attr, GhtAttribute **nextattr)
{
    if ( attr->next )
    {
        *nextattr = attr->next;
        return GHT_OK;
    }
    *nextattr = NULL;
    return GHT_ERROR;
}

GhtErr
ght_attribute_get_dimension(const GhtAttribute *attr, const GhtDimension **dim)
{
    if ( attr->dim )
    {
        *dim = attr->dim;
        return GHT_OK;
    }
    *dim = NULL;
    return GHT_ERROR;
}

GhtErr
ght_attribute_get_by_dimension(const GhtAttribute *attr, const GhtDimension *dim, GhtAttribute *found)
{
    if ( ! attr ) return GHT_ERROR;
    while ( attr->dim != dim )
    {
        if ( ! attr->next ) return GHT_ERROR;
        attr = attr->next;
    }
    memcpy(found, attr, sizeof(GhtAttribute));
    return GHT_OK;
}

/** Convert a real value double into a value suitable for storage */
static GhtErr
ght_attribute_double_to_storage(const GhtDimension *dim, double *val)
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
ght_attribute_storage_to_double(const GhtDimension *dim, double *val)
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
            ght_error("%s: unknown attribute type", __func__);
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
            ght_error("%s: unknown attribute type %d", __func__, type);
            return GHT_ERROR;
        }
    }
    GHT_TRY(ght_attribute_storage_to_double(attr->dim, val));
    return GHT_OK;    
}


GhtErr ght_attribute_set_value(GhtAttribute *attr, double val)
{
    const GhtType type = attr->dim->type;
    size_t size = GhtTypeSizes[type];
    double dv = val;
    
    GHT_TRY(ght_attribute_double_to_storage(attr->dim, &dv));
    
    switch(type)
    {
        case GHT_UNKNOWN:
        {
            ght_error("%s: unknown attribute type %d", __func__, type);
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
            ght_error("%s: unknown attribute type %d", __func__, type);
            return GHT_ERROR;
        }
    }
    return GHT_OK;
}

GhtErr ght_attribute_to_string(const GhtAttribute *attr, stringbuffer_t *sb)
{
    double d;
    GHT_TRY(ght_attribute_get_value(attr, &d));
    ght_stringbuffer_aprintf(sb, "%s=%g", attr->dim->name, d);
    return GHT_OK;
}

GhtErr ght_attribute_get_size(const GhtAttribute *attr, size_t *sz)
{
    *sz = GhtTypeSizes[attr->dim->type];
    return GHT_OK;
}

GhtErr ght_attribute_write(const GhtAttribute *attr, GhtWriter *writer)
{
    uint8_t position;
    size_t attrsize;
    uint8_t buffer[128];
    uint8_t *ptr = buffer;
    GHT_TRY(ght_dimension_get_position(attr->dim, &position));
    GHT_TRY(ght_attribute_get_size(attr, &attrsize));

    /* Write in dimension position number */
    memcpy(ptr, &position, 1);
    ptr++;
    
    /* Write in the attribute value */
    memcpy(ptr, attr->val, attrsize);

    /* Copy the buffer into the writer */
    return ght_write(writer, buffer, attrsize + 1);
}

GhtErr ght_attribute_read(GhtReader *reader, GhtAttribute **attr)
{
    uint8_t dimnum;
    GhtDimension *dim;
    GhtAttribute *a;
    const GhtSchema *schema = reader->schema;

    ght_read(reader, &dimnum, 1);
    if ( dimnum >= schema->num_dims )
    {
        ght_error("%s: attribute dimension %d does not exist in schema %p", __func__, dimnum, schema); 
        return GHT_ERROR;
    }
        
    dim = schema->dims[dimnum];
    a = ght_malloc(sizeof(GhtAttribute));
    if ( ! a ) return GHT_ERROR;
    memset(a, 0, sizeof(GhtAttribute));
    a->dim = dim;
    a->next = NULL;
    ght_read(reader, a->val, GhtTypeSizes[dim->type]);
    *attr = a;
    return GHT_OK;
}


GhtErr ght_attribute_clone(const GhtAttribute *attr, GhtAttribute **attr_out)
{
    /* Clone attribute and all siblings */
    GhtAttribute *a;
    
    if ( ! attr )
    {
        *attr_out = NULL;
        return GHT_OK;
    }

    /* Copy the first one in */
    a = ght_malloc(sizeof(GhtAttribute));
    memcpy(a, attr, sizeof(GhtAttribute));
    a->next = NULL;
    *attr_out = a;
    
    return ght_attribute_clone(attr->next, &((*attr_out)->next));
    
}

static GhtErr
ght_attribute_append(GhtAttribute *attr1, GhtAttribute *attr2)
{
    while ( attr1->next )
    {
        attr1 = attr1->next;
    }
    
    attr1->next = attr2;
    return GHT_OK;
}

GhtErr
ght_attribute_union(GhtAttribute *attr1, GhtAttribute *attr2, GhtAttribute **attr)
{    
    GhtAttribute *a1 = attr1;
    GhtAttribute *a2 = attr2;
    
    /* Null input, null output */
    if ( ! a1 && ! a2 )
    {
        *attr = NULL;
        return GHT_OK;
    }
    
    if ( ! a1 )
    {
        ght_attribute_clone(a2, attr);
        return GHT_OK;
    }

    /* Contents of first list will be starting point */
    ght_attribute_clone(a1, attr);

    /* Nothing in second list? Done. */
    if ( ! a2 ) return GHT_OK;

    /* For each attribute in the second list... */
    while ( a2 )
    {
        int unique = 1;
        a1 = attr1;
        /* Is this attribute in the first list? */
        while ( a1 )
        {
            /* It is. Break. */
            if ( a2->dim == a1->dim )
            {
                unique = 0;
                break;
            }
            a1 = a1->next;
        }
        /* Unique attribute, add a copy to our output list */
        if ( unique )
        {
            GhtAttribute *a = ght_malloc(sizeof(GhtAttribute));
            memcpy(a, a2, sizeof(GhtAttribute));
            a->next = NULL;
            GHT_TRY(ght_attribute_append(*attr, a));
        }
        a2 = a2->next;
    }
    return GHT_OK;
}