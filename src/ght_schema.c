/******************************************************************************
*  LibGHT, software to manage point clouds.
*  LibGHT is free and open source software provided by the Government of Canada
*  Copyright (c) 2012 Natural Resources Canada
*
*  Nouri Sabo <nsabo@NRCan.gc.ca>, Natural Resources Canada
*  Paul Ramsey <pramsey@opengeo.org>, OpenGeo
*
******************************************************************************/

#include <libxml/parser.h>
#include <libxml/xpath.h>
#include "ght_internal.h"
#include <math.h>

/******************************************************************************
*  GhtDimension
******************************************************************************/

/** Create an empty dimension */
GhtErr ght_dimension_new(GhtDimension **dimension) 
{
    GhtDimension *dim;
    assert(dimension);

    dim = ght_malloc(sizeof(GhtDimension));
    memset(dim, 0, sizeof(GhtDimension));
    dim->scale = 1.0;
    *dimension = dim;
    return GHT_OK;
}

GhtErr ght_dimension_free(GhtDimension *dim)
{
    if ( dim->name ) ght_free(dim->name);
    if ( dim->description ) ght_free(dim->description);
    ght_free(dim);
    return GHT_OK;    
}

GhtErr ght_dimension_set_name(GhtDimension *dim, const char *name)
{
    dim->name = ght_strdup(name);
    return GHT_OK; 
}

GhtErr ght_dimension_set_description(GhtDimension *dim, const char *desc)
{
    dim->description = ght_strdup(desc);
    return GHT_OK; 
}

GhtErr ght_dimension_get_position(const GhtDimension *dim, uint8_t *position)
{
    *position = dim->position;
}

GhtErr ght_dimension_same(const GhtDimension *dim1, const GhtDimension *dim2, int *same)
{
    *same = 0;
    if ( dim1->position == dim2->position &&
         strcmp(dim1->name, dim2->name) == 0 &&
         dim1->type == dim2->type &&
         fabs(dim1->scale - dim2->scale) < GHT_EPSILON &&
         fabs(dim1->offset - dim2->offset) < GHT_EPSILON )
    {
        *same = 1;
    }
    return GHT_OK;    
}


/******************************************************************************
*  GhtSchema
******************************************************************************/

GhtErr ght_schema_same(const GhtSchema *s1, const GhtSchema *s2, int *same)
{
    int i;
    *same = 0;
    if ( s1->num_dims != s2->num_dims )
    {
        return GHT_OK;
    }
    for ( i = 0; i < s1->num_dims; i++ )
    {
        ght_dimension_same(s1->dims[i], s2->dims[i], same);
        if ( ! *same )
            return GHT_OK;
    }
    return GHT_OK;
}

GhtErr ght_schema_get_dimension_by_name(const GhtSchema *schema, const char *name, GhtDimension **dim, int *position)
{
    int i;
    assert(name);
    assert(schema);
    *dim = NULL;
    
    for ( i = 0; i < schema->num_dims; i++ )
    {
        const char *sname = schema->dims[i]->name;
        if ( sname && strcasecmp(name, sname) == 0 )
        {
            *dim = schema->dims[i];
            *position = i;
            return GHT_OK;
        }
    }
    return GHT_ERROR;
}

GhtErr ght_schema_get_dimension_by_index(const GhtSchema *schema, int i, GhtDimension **dim)
{
    assert(dim);
    *dim = NULL;
    if ( i >= 0 && i < schema->num_dims )
    {
        *dim = schema->dims[i];
        return GHT_OK;
    }
    return GHT_ERROR;
}

GhtErr ght_schema_new(GhtSchema **schema)
{
    static int max_dims = 8;
    size_t s_size = sizeof(GhtSchema);
    size_t d_size = sizeof(GhtDimension*) * max_dims;
    assert(schema);
    GhtSchema *s = ght_malloc(s_size);
    memset(s, 0, s_size);
    s->dims = ght_malloc(d_size);
    memset(s->dims, 0, sizeof(d_size));
    s->max_dims = max_dims;
    *schema = s;
    return GHT_OK;
}

GhtErr ght_schema_free(GhtSchema *schema)
{
    int i;
    assert(schema);
    for ( i = 0; i < schema->num_dims; i++ )
    {
        if ( schema->dims[i] )
            ght_dimension_free(schema->dims[i]);
    }
    ght_free(schema->dims);
    ght_free(schema);
    return GHT_OK;
}

GhtErr ght_schema_add_dimension(GhtSchema *schema, GhtDimension *dim)
{
    int i;
    
    assert(schema);
    assert(dim);
    
    if ( ! dim->name ) return GHT_ERROR;
    
    for ( i = 0; i < schema->num_dims; i++ )
    {
        if ( strcmp(dim->name, schema->dims[i]->name) == 0 )
        {
            ght_error("%s: cannot add dimension with a duplicate name", __func__);
            return GHT_ERROR;
        }
    }
    
    if ( schema->num_dims == schema->max_dims )
    {
        schema->max_dims *= 2;
        schema->dims = ght_realloc(schema->dims, schema->max_dims * sizeof(GhtDimension*));
    }
    
    dim->position = schema->num_dims;
    schema->dims[schema->num_dims] = dim;
    schema->num_dims++;
    
    return GHT_OK;
}


static GhtErr ght_dimension_from_xml(xmlNodePtr node, GhtDimension **dimension)
{
    xmlNodePtr child;
    GhtDimension *dim;

    GHT_TRY(ght_dimension_new(dimension));
    dim = *dimension;
    
    for ( child = node->children; child; child = child->next )
    {
        if ( child->type == XML_ELEMENT_NODE )
        {
            
#define TAG_IS(str) (strcmp(child->name, str) == 0)
            
            if ( TAG_IS("name") )
            {
                ght_dimension_set_name(dim, child->children->content);
            }
            else if ( TAG_IS("description") )
            {
                ght_dimension_set_description(dim, child->children->content);
            }
            else if ( TAG_IS("interpretation") )
            {
                GhtType type;
                GHT_TRY(ght_type_from_str(child->children->content, &type));
                dim->type = type;
            }
            else if ( TAG_IS("scale") )
            {
                dim->scale = atof(child->children->content);
            }
            else if ( TAG_IS("offset") )
            {
                dim->offset = atof(child->children->content);
            }
            else
            {
                /* Unhandled <tag> */
            }
        }
    }
    
    return GHT_OK;
}   

static GhtErr ght_schema_from_xml(xmlDocPtr xml_doc, GhtSchema **schema)
{
    static xmlChar *xpath_str = "/pc:PointCloudSchema/pc:dimension";
    xmlNsPtr xml_ns = NULL;
    xmlXPathContextPtr xpath_ctx;
    xmlXPathObjectPtr xpath_obj;
    xmlNodePtr xml_root = NULL;
    xmlNodeSetPtr nodes;
    
    xml_root = xmlDocGetRootElement(xml_doc);

    if ( xml_root->ns )
        xml_ns = xml_root->ns;

    /* Create xpath evaluation context */
    xpath_ctx = xmlXPathNewContext(xml_doc);
    if( ! xpath_ctx )
    {
        ght_warn("unable to create new XPath context to read schema XML");
        return GHT_ERROR;
    }

    /* Register the root namespace if there is one */
    if ( xml_ns )
        xmlXPathRegisterNs(xpath_ctx, "pc", xml_ns->href);

    /* Evaluate xpath expression */
    xpath_obj = xmlXPathEvalExpression(xpath_str, xpath_ctx);
    if( ! xpath_obj )
    {
        xmlXPathFreeContext(xpath_ctx);
        ght_warn("unable to evaluate xpath expression \"%s\" against schema XML", xpath_str);
        return GHT_ERROR;
    }

    /* Iterate on the dimensions we found */
    if ( nodes = xpath_obj->nodesetval )
    {
        int i;
        int ndims = nodes->nodeNr;
        
        GHT_TRY(ght_schema_new(schema));

        for ( i = 0; i < ndims; i++ )
        {
            /* This is a "dimension" */
            if( nodes->nodeTab[i]->type == XML_ELEMENT_NODE )
            {
                GhtDimension *dim;
                GHT_TRY(ght_dimension_from_xml(nodes->nodeTab[i], &dim));
                GHT_TRY(ght_schema_add_dimension(*schema, dim));
            }
        }
    }
    else
    {
        xmlXPathFreeObject(xpath_obj);
        xmlXPathFreeContext(xpath_ctx);
        return GHT_ERROR;
    }
    
    xmlXPathFreeObject(xpath_obj);
    xmlXPathFreeContext(xpath_ctx);
    return GHT_OK;
}

GhtErr ght_schema_from_xml_str(const char *xml_str, GhtSchema **schema) 
{
    const char *xml_ptr = xml_str;
    size_t xml_size;
    xmlDocPtr xml_doc;
    GhtErr result;
    
    /* Roll forward to start of XML string */
    while( (*xml_ptr != '\0') && (*xml_ptr != '<') )
    {
        xml_ptr++;
    }
    
    xml_size = strlen(xml_ptr);
    
    xmlInitParser();
    xml_doc = xmlReadMemory(xml_ptr, xml_size, NULL, NULL, 0);
    if ( ! xml_doc )
    {
        ght_warn("unable to parse schema XML");
        result = GHT_ERROR;
    }
    else
    {
        result = ght_schema_from_xml(xml_doc, schema);
        xmlFreeDoc(xml_doc);
    }
    xmlCleanupParser();
    
    return result;
}

GhtErr ght_schema_to_xml_str(const GhtSchema *schema, char **xml_str, size_t *xml_str_size) 
{
    int i;
    stringbuffer_t *sb = stringbuffer_create_with_size(1024);
    assert(schema);
    assert(xml_str);
    
    stringbuffer_append(sb, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    stringbuffer_append(sb, "<pc:PointCloudSchema xmlns:pc=\"http://pointcloud.org/schemas/PC/1.1\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\n");
    
    for ( i = 0; i < schema->num_dims; i++ )
    {
        GhtDimension *dim = schema->dims[i];
        stringbuffer_append(sb, "<pc:dimension>\n");
        stringbuffer_aprintf(sb, "<pc:position>%d</pc:position>\n", i+1);
        if ( dim->name )
        {
            stringbuffer_aprintf(sb, "<pc:name>%s</pc:name>\n", dim->name);
        }
        if ( dim->description )
        {
            stringbuffer_aprintf(sb, "<pc:description>%s</pc:description>\n", dim->description);
        }
        stringbuffer_aprintf(sb, "<pc:interpretation>%s</pc:interpretation>\n", GhtTypeStrings[dim->type]);
        stringbuffer_aprintf(sb, "<pc:size>%zu</pc:size>\n", GhtTypeSizes[dim->type]);
        if ( dim->scale != 1 )
        {
            stringbuffer_aprintf(sb, "<pc:scale>%g</pc:scale>\n", dim->scale);
        }
        if ( dim->offset != 0 )
        {
            stringbuffer_aprintf(sb, "<pc:offset>%g</pc:offset>\n", dim->offset);
        }
        stringbuffer_append(sb, "<pc:active>true</pc:active>\n");
        stringbuffer_append(sb, "</pc:dimension>\n");
    }
    
    stringbuffer_append(sb, "</pc:PointCloudSchema>");     
    
    *xml_str = stringbuffer_getstringcopy(sb);
    *xml_str_size = stringbuffer_getlength(sb) + 1;
    stringbuffer_destroy(sb);
    return GHT_OK;
}

GhtErr ght_schema_from_xml_file(const char *filename, GhtSchema **schema)
{
    stringbuffer_t *sb;
    GhtErr err;
    FILE *file = NULL;
    static size_t read_size = 1023;
    size_t sz;
    char buf[read_size+1]; /* space for null terminator */

    file = fopen(filename, "r");
    if ( ! file )
    {
        ght_error("%s: failed to open xml schema file %s for reading", __func__, filename);
        return GHT_ERROR;
    }
    
    sb = stringbuffer_create();
    
    while(1)
    {
        sz = fread(buf, read_size, 1, file);
        buf[read_size] = '\0';
        stringbuffer_append(sb, buf);
        if ( sz != read_size )
            break;
    }
    
    err = ght_schema_from_xml_str(stringbuffer_getstring(sb), schema);
    
    stringbuffer_destroy(sb);
    return err;
}


GhtErr ght_schema_to_xml_file(const GhtSchema *schema, const char *filename) 
{
    size_t xml_size, write_size;
    char *xml;
    GhtErr err;
    FILE *file;

    file = fopen(filename, "w");
    if ( ! file )
    {
        ght_error("%s: failed to open xml schema file %s for writing", __func__, filename);
        return GHT_ERROR;
    }
    
    GHT_TRY(ght_schema_to_xml_str(schema, &xml, &xml_size));
    
    write_size = fwrite(xml, 1, xml_size, file);
    if ( write_size != xml_size )
    {
        ght_error("%s: failed to write xml schema file", __func__);
        return GHT_ERROR;
    }
    
    fclose(file);
    
    return GHT_OK;
}


