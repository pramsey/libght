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

/** Create an empty dimension */
GhtErr ght_dimension_new(GhtDimension **dimension) 
{
    GhtDimension *dim;
    assert(dimension);

    dim = ght_malloc(sizeof(GhtDimension));
    memset(dim, 0, sizeof(GhtDimension));
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
    ght_free(schema);
    return GHT_OK;
}

GhtErr ght_schema_add_dimension(GhtSchema *schema, GhtDimension *dim)
{
    assert(schema);
    assert(dim);
    
    if ( schema->num_dims == schema->max_dims )
    {
        schema->max_dims *= 2;
        schema->dims = ght_realloc(schema->dims, schema->max_dims * sizeof(GhtDimension*));
    }
    
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
