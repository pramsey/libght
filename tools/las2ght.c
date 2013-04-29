/***********************************************************************
* las2ght.c
*
*   convert las file to ght file
*
***********************************************************************/

#include "liblas/capi/liblas.h"
#include "proj_api.h"
#include "ght_internal.h"
#include <string.h>

#define EXENAME "las2ght"
#define MAXPOINTS 2000000
#define STRSIZE 1024
#define LOG_NUM_POINTS 100000

#ifdef HAVE_GETOPT_H
/* System implementation */
#include <getopt.h>
#else
/* Compatibility implementation */
#include "getopt.h"
#endif

/* "username-fileno-hash.ght" */
static char *ght_file_template = "%s-%d-%s.ght";
static char *xml_file_template = "%s-%d-%s.ght.xml";

typedef enum
{
    LL_INTENSITY = 0,
    LL_RETURN_NUMBER,
    LL_NUMBER_OF_RETURNS,
    LL_SCAN_DIRECTION,
    LL_FLIGHT_LINE_EDGE,
    LL_CLASSIFICATION,
    LL_SCAN_ANGLE,
    LL_POINT_SOURCE_ID,
    LL_RED,
    LL_GREEN,
    LL_BLUE
} LasAttribute;

typedef struct  
{
    const char *name;
    const char *description;
    GhtType type;
    double scale;
    double offset;
    LasAttribute attr;
    char flag;
} LasDimension;

static LasDimension LasAttributes[] = 
{
    { "Intensity", "", GHT_UINT16, 1.0, 0.0, LL_INTENSITY, 'i' },
    { "ReturnNumber", "", GHT_UINT16, 1.0, 0.0, LL_RETURN_NUMBER, 'r' },
    { "NumberOfReturns", "", GHT_UINT16, 1.0, 0.0, LL_NUMBER_OF_RETURNS, 'n' },
    { "ScanDirection", "", GHT_UINT16, 1.0, 0.0, LL_SCAN_DIRECTION, 'd' },
    { "FlightLineEdge", "", GHT_UINT16, 1.0, 0.0, LL_FLIGHT_LINE_EDGE, 'e' },
    { "Classification", "", GHT_UINT8, 1.0, 0.0, LL_CLASSIFICATION, 'c' },
    { "ScanAngle", "", GHT_INT8, 1.0, 0.0, LL_SCAN_ANGLE, 'a' },
    { "PointSourceId", "", GHT_UINT16, 1.0, 0.0, LL_POINT_SOURCE_ID, 'p' },
    { "Red", "", GHT_UINT16, 1.0, 0.0, LL_RED, 'R' },
    { "Green", "", GHT_UINT16, 1.0, 0.0, LL_GREEN, 'G' },
    { "Blue", "", GHT_UINT16, 1.0, 0.0, LL_BLUE, 'B' },
    { NULL, NULL, 0, 0, 0 }
};

#define NUM_LAS_ATTRIBUTES 11

typedef struct 
{
    char *lasfile;    /* File to read */
    char *ghtfile;    /* File to write */
    char attrs[NUM_LAS_ATTRIBUTES];  /* Attributes to transfer */
    int num_attrs;    /* How many attributes are we transferring? */
    int validpoints;  /* Should we only convert valid points? */
    int resolution;   /* How many digits of the GeoHash to build? */
    int maxpoints;    /* How many points to save in each GHT file? */
} Las2GhtConfig;

typedef struct 
{
    LASReaderH reader;
    LASHeaderH header;
    int fileno;
    projPJ pj_input;
    projPJ pj_output;
    GhtSchema *schema;
} Las2GhtState;

static void
l2g_config_printf(const Las2GhtConfig *config)
{
    ght_info("Las2GhtConfig (%p)", config);
    ght_info("      lasfile: %s", config->lasfile);
    ght_info("      ghtfile: %s", config->ghtfile);
    ght_info("    num_attrs: %d", config->num_attrs);
    ght_info("  validpoints: %d", config->validpoints);
    ght_info("   resolution: %d", config->resolution);
}

static void
l2g_usage()
{
    printf("%s, version %d.%d\n\n", EXENAME, GHT_VERSION_MAJOR, GHT_VERSION_MINOR);
    printf("Usage: %s [options]\n\n", EXENAME);
    printf("Options:\n");
    printf("  --lasfile FILENAME            Read file as input.\n");
    printf("  --ghtfile FILENAME            Write file as output.\n");
    printf("  --validpoints                 Only convert valid points.\n");
    printf("  --attrs [irndecapRGB]         Convert selected attributes.\n");
    printf("                                X,Y,Z are always converted.\n");
    printf("      i - intensity\n");
    printf("      r - number of this return\n");
    printf("      n - number of returns for given pulse\n");
    printf("      d - direction of scan flag\n");
    printf("      e - edge of flight line\n");
    printf("      c - classification number\n");
    printf("      a - scan angle\n");
    printf("      p - point source ID\n");
    printf("      R - red channel of RGB color\n");
    printf("      G - green channel of RGB color\n");
    printf("      B - blue channel of RGB color\n");
    printf("\n");
}

static void
l2g_config_attrs(Las2GhtConfig *config, const char *attr_str)
{
    int len;
    int i = 0, j = 0, k = 0;
    char current_attr;
    
    assert(config);
    config->num_attrs = 0;

    /* Noop on null */
    if ( ! attr_str ) return;
    
    /* Go through the string one at a time */
    j = 0;
    while(attr_str[j])
    {
        /* Go through the possible attributes one at a time */
        i = 0;
        while( LasAttributes[i].name )
        {
            /* Found an attribute flag! */
            if ( LasAttributes[i].flag == attr_str[j] )
            {
                config->attrs[config->num_attrs++] = LasAttributes[i].attr;
            }
            i++;
        }
        j++;
    }
    return;
}

static void
l2g_config_free(Las2GhtConfig *config)
{
    if ( config->lasfile )
    {
        free(config->lasfile);
        config->lasfile = NULL;
    }
    if ( config->ghtfile )
    {
        free(config->ghtfile);
        config->ghtfile = NULL;
    }
}

l2g_state_free(Las2GhtState *state)
{
    if ( state->header )
    {
        LASHeader_Destroy(state->header);
        state->header = NULL;
    }
    if ( state->reader )
    {
        LASReader_Destroy(state->reader);
        state->reader = NULL;
    }
    if ( state->pj_input )
    {
        pj_free(state->pj_input);
        state->pj_input = NULL;
    }
    if ( state->pj_output )
    {
        pj_free(state->pj_output);
        state->pj_output = NULL;
    }
    if ( state->schema )
    {
        ght_schema_free(state->schema);
        state->schema = NULL;
    }       
}

static int
l2g_fexists(const char *filename)
{
    FILE *fd;
    if ( ! (fd = fopen(filename, "r")) )
        return 0;
    fclose(fd);
    return 1;
}

static int
l2g_writable(const char *filename)
{
    FILE *fd;
    if ( ! (fd = fopen(filename, "w")) )
        return 0;
    fclose(fd);
    remove(filename);
    return 1;
}


static int
l2g_getopts(int argc, char **argv, Las2GhtConfig *config)
{
    int ch = 0;

    /* options descriptor */
    static struct option longopts[] = 
    {
        { "lasfile", required_argument, NULL, 'l' },
        { "ghtfile", required_argument, NULL, 'g' },
        { "attrs", required_argument, NULL, 'a' },
        { "validpoints", no_argument, NULL, 'p' },
        { NULL, 0, NULL, 0 }
    };

    memset(config, 0, sizeof(Las2GhtConfig));

    while ( (ch = getopt_long(argc, argv, "g:l:a:p", longopts, NULL)) != -1)
    {
        switch (ch) 
        {
            case 'l':
            {
                config->lasfile = strdup(optarg);
                break;
            }
            case 'g':
            {
                config->ghtfile = strdup(optarg);
                break;
            }
            case 'a':
            {
                l2g_config_attrs(config, optarg);
                break;
            }
            case 'p':
            {
                config->validpoints = 1;
                break;
            }
            default:
            {
                l2g_config_free(config);
                return 0;
            }
        }
    }
    
    if ( ! (config->lasfile && config->ghtfile) )
    {
        l2g_config_free(config);
        return 0;
    }
    return 1;
}

static GhtErr
l2g_build_schema(const Las2GhtConfig *config, Las2GhtState *state)
{
    int i = 0;
    GhtSchema *schema;
    GhtDimension *dim;
    GHT_TRY(ght_schema_new(&schema));
    
    /* Add 'X' dimension (position 0) */
    GHT_TRY(ght_dimension_new(&dim));
    GHT_TRY(ght_dimension_set_name(dim, "X"));
    GHT_TRY(ght_dimension_set_description(dim, ""));
    dim->scale = 1.0;
    dim->offset = 0.0;
    dim->type = GHT_DOUBLE;
    GHT_TRY(ght_schema_add_dimension(schema, dim));

    /* Add 'Y' dimension (position 1) */
    GHT_TRY(ght_dimension_new(&dim));
    GHT_TRY(ght_dimension_set_name(dim, "Y"));
    GHT_TRY(ght_dimension_set_description(dim, ""));
    dim->scale = 1.0;
    dim->offset = 0.0;
    dim->type = GHT_DOUBLE;
    GHT_TRY(ght_schema_add_dimension(schema, dim));

    /* Add 'Z' dimension (position 2) */
    GHT_TRY(ght_dimension_new(&dim));
    GHT_TRY(ght_dimension_set_name(dim, "Z"));
    GHT_TRY(ght_dimension_set_description(dim, ""));
    dim->scale = LASHeader_GetScaleZ(state->header);
    dim->offset = LASHeader_GetOffsetZ(state->header);
    dim->type = GHT_INT32;
    GHT_TRY(ght_schema_add_dimension(schema, dim));
    
    /* Add optional attributes (positions 3+) */
    for ( i = 0; i < config->num_attrs; i++ )
    {
        LasDimension ld = LasAttributes[config->attrs[i]];
        GHT_TRY(ght_dimension_new(&dim));
        GHT_TRY(ght_dimension_set_name(dim, ld.name));
        GHT_TRY(ght_dimension_set_description(dim, ""));
        dim->scale = ld.scale;
        dim->offset = ld.offset;
        dim->type = ld.type;
        GHT_TRY(ght_schema_add_dimension(schema, dim));
    }

    state->schema = schema;
    return GHT_OK;
}

    // LAS_DLL LASPointH LASReader_GetNextPoint(const LASReaderH hReader);
    // LAS_DLL LASSRSH LASHeader_GetSRS(const LASHeaderH hHeader);
    // LAS_DLL double LASPoint_GetX(const LASPointH hPoint);
    // LAS_DLL double LASPoint_GetY(const LASPointH hPoint);
    // LAS_DLL double LASPoint_GetZ(const LASPointH hPoint);
    // LAS_DLL unsigned short LASPoint_GetIntensity(const LASPointH hPoint);
    // LAS_DLL unsigned short LASPoint_GetReturnNumber(const LASPointH hPoint);
    // LAS_DLL unsigned short LASPoint_GetNumberOfReturns(const LASPointH hPoint);
    // LAS_DLL unsigned short LASPoint_GetScanDirection(const LASPointH hPoint);
    // LAS_DLL unsigned short LASPoint_GetFlightLineEdge(const LASPointH hPoint);
    // LAS_DLL unsigned char LASPoint_GetScanFlags(const LASPointH hPoint);
    // LAS_DLL unsigned char LASPoint_GetClassification(const LASPointH hPoint);
    // LAS_DLL double LASPoint_GetTime(const LASPointH hPoint);
    // LAS_DLL char LASPoint_GetScanAngleRank(const LASPointH hPoint);
    // LAS_DLL unsigned short LASPoint_GetPointSourceId(LASPointH hPoint);
    // LAS_DLL LASColorH LASPoint_GetColor(const LASPointH hPoint);
    // LAS_DLL unsigned short LASColor_GetRed(const LASColorH hColor);
    // LAS_DLL unsigned short LASColor_GetGreen(const LASColorH hColor);
    // LAS_DLL unsigned short LASColor_GetBlue(const LASColorH hColor);
    // LAS_DLL LASSRSH LASHeader_GetSRS(const LASHeaderH hHeader);
    // LAS_DLL char* LASSRS_GetProj4(LASSRSH hSRS);    
    // enum DataMemberFlag
    //  {
    //      eReturnNumber = 1,
    //      eNumberOfReturns = 2,
    //      eScanDirection = 4,
    //      eFlightLineEdge = 8,
    //      eClassification = 16,
    //      eScanAngleRank = 32,
    //      eTime = 64
    //  };
    // LAS_DLL int LASPoint_Validate(LASPointH hPoint);
    // LAS_DLL int LASPoint_IsValid(LASPointH hPoint);
    // LAS_DLL double LASHeader_GetScaleX(const LASHeaderH hHeader);
    // LAS_DLL double LASHeader_GetScaleY(const LASHeaderH hHeader);
    // LAS_DLL double LASHeader_GetScaleZ(const LASHeaderH hHeader);
    // LAS_DLL double LASHeader_GetOffsetX(const LASHeaderH hHeader);
    // LAS_DLL double LASHeader_GetOffsetY(const LASHeaderH hHeader);
    // LAS_DLL double LASHeader_GetOffsetZ(const LASHeaderH hHeader);
    // LAS_DLL double LASHeader_GetMinX(const LASHeaderH hHeader);

static double
l2g_attribute_value(const LASPointH laspoint, LasAttribute lasdim)
{
    double val = 0.0;
    switch ( lasdim )
    {
        case LL_INTENSITY:
            val = LASPoint_GetIntensity(laspoint);
            break;
        case LL_RETURN_NUMBER:
            val = LASPoint_GetReturnNumber(laspoint);
            break;
        case LL_NUMBER_OF_RETURNS:
            val = LASPoint_GetNumberOfReturns(laspoint);
            break;
        case LL_SCAN_DIRECTION:
            val = LASPoint_GetScanDirection(laspoint);
            break;
        case LL_FLIGHT_LINE_EDGE:
            val = LASPoint_GetFlightLineEdge(laspoint);
            break;
        case LL_CLASSIFICATION:
            val = LASPoint_GetClassification(laspoint);
            break;
        case LL_SCAN_ANGLE:
            val = LASPoint_GetScanAngleRank(laspoint);
            break;
        case LL_POINT_SOURCE_ID:
            val = LASPoint_GetPointSourceId(laspoint);
            break;
        case LL_RED:
            val = LASColor_GetRed(LASPoint_GetColor(laspoint));
            break;
        case LL_GREEN:
            val = LASColor_GetRed(LASPoint_GetColor(laspoint));
            break;
        case LL_BLUE:
            val = LASColor_GetRed(LASPoint_GetColor(laspoint));
            break;
    }
    return val;
}

static void
l2g_coordinate_to_rad(GhtCoordinate *coord)
{
    coord->x *=  M_PI/180.0;
    coord->y *=  M_PI/180.0;
}

static void
l2g_coordinate_to_dec(GhtCoordinate *coord)
{
    coord->x *=  180.0/M_PI;
    coord->y *=  180.0/M_PI;
}

static GhtErr
l2g_coordinate_reproject(const Las2GhtState *state, GhtCoordinate *coord)
{

    int *pj_errno_ref;
    GhtCoordinate origcoord;

    /* Make a copy of the input point so we can report the original should an error occur */
    origcoord = *coord;

    if (pj_is_latlong(state->pj_input)) l2g_coordinate_to_rad(coord);

    /* Perform the transform */
    pj_transform(state->pj_input, state->pj_output, 1, 0, &(coord->x), &(coord->y), NULL);

    /* For NAD grid-shift errors, display an error message with an additional hint */
    pj_errno_ref = pj_get_errno_ref();

    if (*pj_errno_ref != 0)
    {
        if (*pj_errno_ref == -38)
        {
            ght_warn("No no grid shift files were found, or point out of range.");
        }
        ght_error("%s: could not project point (%g %g): %s (%d)", 
                  __func__, 
                  origcoord.x, origcoord.y,
                  pj_strerrno(*pj_errno_ref), *pj_errno_ref
                  );
        return GHT_ERROR;
    }

    if (pj_is_latlong(state->pj_output)) l2g_coordinate_to_dec(coord);
    return GHT_OK;
}

static GhtErr
l2g_build_node(const Las2GhtConfig *config, const Las2GhtState *state, LASPointH laspoint, GhtNode **node)
{
    int i;
    double z;
    GhtDimension *ghtdim;
    GhtAttribute *attribute;
    GhtCoordinate coord;
    GhtErr err;
    
    assert(config);
    assert(state->schema);
    
    /* Skip invalid points, if so configured */
    if ( config->validpoints && ! LASPoint_IsValid(laspoint) )
        return NULL;

    coord.x = LASPoint_GetX(laspoint);
    coord.y = LASPoint_GetY(laspoint);
    
    if ( l2g_coordinate_reproject(state, &coord) != GHT_OK )
        return GHT_ERROR;
    
    if ( ght_node_new_from_coordinate(&coord, config->resolution, node) != GHT_OK )
        return GHT_ERROR;

    /* We know that 'Z' is always dimension 2 */
    z = LASPoint_GetZ(laspoint);
    ghtdim = state->schema->dims[2];
    
    if ( ght_attribute_new_from_double(ghtdim, z, &attribute) != GHT_OK )
        return GHT_ERROR;

    if ( ght_node_add_attribute(*node, attribute) != GHT_OK )
        return GHT_ERROR;
    
    for ( i = 0; i < config->num_attrs; i++ )
    {
        LasAttribute lasattr = config->attrs[i];
        double val = l2g_attribute_value(laspoint, lasattr);

        /* Magic number 3: X,Y,Z are first three dimensions */
        ghtdim = state->schema->dims[3 + i];
        
        if ( ght_attribute_new_from_double(ghtdim, val, &attribute) != GHT_OK )
            return GHT_ERROR;

        if ( ght_node_add_attribute(*node, attribute) != GHT_OK )
            return GHT_ERROR;
    }

    return GHT_OK;
}

static int
l2g_build_tree(const Las2GhtConfig *config, Las2GhtState *state, GhtTree **tree)
{
    int num_points = 0;
    int i;
    LASPointH laspoint;
    GhtNode *node;
    GhtErr err;

    ght_info("starting a new tree");
    ght_tree_new(state->schema, tree);

    while( (laspoint = LASReader_GetNextPoint(state->reader)) && num_points <= config->maxpoints )
    {
        err = l2g_build_node(config, state, laspoint, &node);
        // LASPoint_Destroy(laspoint); /* Don't need this, it's not allocated on the heap? */
        err = ght_tree_insert_node(*tree, node);
        num_points++;
        if ( ! (num_points % LOG_NUM_POINTS) )
            ght_info("inserted point %d into the tree...", num_points);
    }

    return num_points;
}

static void
l2g_ght_file(const Las2GhtConfig *config, Las2GhtState *state, GhtHash *hash, char *str)
{
    char *ptr;
    char basename[STRSIZE];
    strncpy(basename, config->ghtfile, STRSIZE);
    /* Null out ".ght" extension if it already exists, */
    /* because the filename template already includes it. */
    ptr = strcasestr(basename, ".ght");
    if ( ptr )
        *ptr = 0;

    snprintf(str, STRSIZE, ght_file_template, basename, state->fileno, hash);
    return;
}

static void
l2g_xml_file(const Las2GhtConfig *config, Las2GhtState *state, GhtHash *hash, char *str)
{
    char *ptr;
    char basename[STRSIZE];
    strncpy(basename, config->ghtfile, STRSIZE);
    /* Null out ".ght" extension from basename if it already exists, */
    /* because the filename template already includes it. */
    ptr = strcasestr(basename, ".ght");
    if ( ptr )
        *ptr = 0;

    snprintf(str, STRSIZE, xml_file_template, basename, state->fileno, hash);
    return;
}

static GhtErr
l2g_save_tree(const Las2GhtConfig *config, Las2GhtState *state, const GhtTree *tree)
{
    char ght_filename[STRSIZE];
    char xml_filename[STRSIZE];
    GhtWriter *writer;
    GhtHash *hash = NULL;

    assert(config);
    assert(state);
    assert(tree);
    assert(tree->schema);

    ght_tree_get_hash(tree, &hash);

    l2g_ght_file(config, state, hash, ght_filename);
    l2g_xml_file(config, state, hash, xml_filename);

    ght_info("writing tree to file %s", ght_filename);

    if ( ! l2g_writable(ght_filename) )
    {
        ght_error("unable to write to '%s'", ght_filename);
        return GHT_ERROR;
    }
    if ( ! l2g_writable(xml_filename) )
    {
        ght_error("unable to write to '%s'", xml_filename);
        return GHT_ERROR;
    }

    GHT_TRY(ght_schema_to_xml_file(tree->schema, xml_filename));
    GHT_TRY(ght_writer_new_file(ght_filename, &writer));
    GHT_TRY(ght_tree_write(tree, writer));
    GHT_TRY(ght_writer_free(writer));
    
    /* Increment file counter */
    state->fileno++;

    return GHT_OK;
}

static projPJ
l2g_proj_from_string(const char *str1)
{
    int t;
    char *params[1024];  /* one for each parameter */
    char *loc;
    char *str;
    size_t slen;
    projPJ result;

    if ( ! str1 || ! strlen(str1) ) return NULL;

    str = strdup(str1);
    
    params[0] = str; /* 1st param, we'll null terminate at the " " soon */

    loc = str;
    t = 1;
    while  ((loc != NULL) && (*loc != 0) )
    {
        loc = strchr(loc, ' ');
        if (loc != NULL)
        {
            *loc = 0; /* null terminate */
            loc++; /* next char */
            params[t] = loc;
            t++; /*next param */
        }
    }

    if (!(result=pj_init(t, params)))
    {
        free(str);
        return NULL;
    }
    free(str);
    return result;
}
    
static GhtErr
l2g_read_projection(const Las2GhtConfig *config, Las2GhtState *state)
{
    LASSRSH lassrs;    
    char *proj4_input = NULL;
    static char *proj4_output = "+proj=longlat +datum=WGS84 +no_defs";

    assert(state);
    assert(state->header);
    assert(config);


    lassrs = LASHeader_GetSRS(state->header);
    if ( ! lassrs )
    {   
        ght_error("%s: unable to read SRS from LAS header", __func__);
        return GHT_ERROR;
    }

    proj4_input = LASSRS_GetProj4(lassrs);
    if ( ! proj4_input )
    {   
        ght_error("%s: unable to read proj4 string from LAS SRS", __func__);
        return GHT_ERROR;
    }
    
    ght_info("Got LAS file projection information '%s'", proj4_input);

    state->pj_input = l2g_proj_from_string(proj4_input);
    LASString_Free(proj4_input);
    
    if ( ! state->pj_input )
    {
        ght_error("%s: unable to parse proj4 string '%s'", __func__, proj4_input);
        return GHT_ERROR;
    }

    state->pj_output = l2g_proj_from_string(proj4_output);
    if ( ! state->pj_output )
    {
        ght_error("%s: unable to parse proj4 string '%s'", __func__, proj4_input);
        return GHT_ERROR;
    }
    
    return GHT_OK;
}

int
main (int argc, char **argv)
{
    Las2GhtConfig config;
    Las2GhtState state;
    GhtTree *tree;
    int num_points;
    
    /* We can't do anything if we don't have GDAL/GeoTIFF support in libLAS */
    if ( ! (LAS_IsGDALEnabled() && LAS_IsLibGeoTIFFEnabled()) )
    {
        ght_error("%s: requires LibLAS built with GDAL and GeoTIFF support", EXENAME);
        return 1;
    }
    
    /* Ensure state is clean */
    memset(&state, 0, sizeof(Las2GhtState));

    /* Set up to use the GHT system memory management / logging */
    ght_init();

    /* If no options are specified, display l2g_usage */
    if (argc <= 1)
    {
        l2g_usage();
        return 1;
    }

    /* Parse command line options and set configuration */
    if ( ! l2g_getopts(argc, argv, &config) )
    {
        l2g_usage();
        return 1;
    }
     
    /* Hard code resolution for now */
    config.resolution = GHT_MAX_HASH_LENGTH;
    config.maxpoints = 2000000;
    
    /* Temporary info printout */
    l2g_config_printf(&config);

    /* Input file exists? */
    if ( ! l2g_fexists(config.lasfile) )
    {
        ght_error("%s: LAS file '%s' does not exist\n", EXENAME, config.lasfile);
        return 1;
    }
    
    /* Output file is writeable? */
    if ( ! l2g_writable(config.ghtfile) )
    {
        ght_error("%s: GHT file '%s' is not writable\n", EXENAME, config.ghtfile);
        return 1;
    }

    /* Can we open the LAS file? */
    state.reader = LASReader_Create(config.lasfile);
    if ( ! state.reader )
    {
        ght_error("%s: unable to open LAS file '%s'\n", EXENAME, config.lasfile);
        return 1;
    }
    
    ght_info("Opened LAS file '%s' for reading", config.lasfile);
    
    /* Get the header */
    state.header = LASReader_GetHeader(state.reader);
    if ( ! state.header) 
    {
        l2g_state_free(&state);
        ght_error("%s: unable to read LAS header in '%s'\n", EXENAME, config.lasfile);
        return 1;
    }
    
    /* Schema is needed to create nodes/attributes */
    if ( GHT_OK != l2g_build_schema(&config, &state) )
    {
        l2g_state_free(&state);
        ght_error("%s: unable to build schema!", EXENAME);
        return 1;
    }
    
    /* Project info is needed to get points into lat/lon space */
    if ( GHT_OK != l2g_read_projection(&config, &state) )
    {
        l2g_state_free(&state);
        ght_error("%s: unable to build projection information", EXENAME);
        return 1;
    }

    // char *xmlstr;
    // size_t xmlsize;
    // ght_schema_to_xml_str(schema, &xmlstr, &xmlsize);
    // printf("\n%s\n\n", xmlstr);


    /* Break the problem into chunks. We might get a really really */
    /* big LAS file, and we don't want to blow out memory, so we need to */
    /* do this a few million records at a file */
    do 
    {
        num_points = l2g_build_tree(&config, &state, &tree);
        if ( num_points )
        {
            GhtErr err;
            ght_tree_compact_attributes(tree);
            err = l2g_save_tree(&config, &state, tree);
            ght_tree_free(tree);
            if ( err != GHT_OK )
                return 1;
        }
    } 
    while ( num_points > 0 );

    l2g_state_free(&state);
    l2g_config_free(&config);

    ght_info("conversion complete");

    return 0;
}