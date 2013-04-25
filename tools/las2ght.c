/***********************************************************************
* las2ght.c
*
*   convert las file to ght file
*
***********************************************************************/

#include "liblas/capi/liblas.h"
#include "ght_internal.h"
#include <string.h>

#define EXENAME "las2ght"

#ifdef HAVE_GETOPT_H
/* System implementation */
#include <getopt.h>
#else
/* Compatibility implementation */
#include "getopt.h"
#endif

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
    char *lasfile;   /* File to read */
    char *ghtfile;   /* File to write */
    char attrs[NUM_LAS_ATTRIBUTES];  /* Attributes to transfer */
    int num_attrs;
    int validpoints;
} Las2GhtConfig;

typedef struct 
{
    LASReaderH reader;
    LASHeaderH header;
} Las2GhtState;

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
        }
    }
    return;
}

static void
l2g_config_free(Las2GhtConfig *config)
{
    if ( config->lasfile )
        free(config->lasfile);
    if ( config->ghtfile )
        free(config->ghtfile);
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

static GhtSchema *
l2g_schema_new(const Las2GhtState *state, const Las2GhtConfig *config)
{
    int i = 0;
    GhtSchema *schema;
    GhtDimension *dim;
    ght_schema_new(&schema);
    
    /* Add 'X' dimension (position 0) */
    ght_dimension_new(&dim);
    ght_dimension_set_name(dim, "X");
    ght_dimension_set_description(dim, "");
    dim->scale = 1.0;
    dim->offset = 0.0;
    dim->type = GHT_DOUBLE;
    ght_schema_add_dimension(schema, dim);

    /* Add 'Y' dimension (position 1) */
    ght_dimension_new(&dim);
    ght_dimension_set_name(dim, "Y");
    ght_dimension_set_description(dim, "");
    dim->scale = 1.0;
    dim->offset = 0.0;
    dim->type = GHT_DOUBLE;
    ght_schema_add_dimension(schema, dim);

    /* Add 'Z' dimension (position 2) */
    ght_dimension_new(&dim);
    ght_dimension_set_name(dim, "Z");
    ght_dimension_set_description(dim, "");
    dim->scale = LASHeader_GetScaleZ(state->header);
    dim->offset = LASHeader_GetOffsetZ(state->header);
    dim->type = GHT_INT32;
    ght_schema_add_dimension(schema, dim);
    
    /* Add optional attributes (positions 3+) */
    for ( i = 0; i < config->num_attrs; i++ )
    {
        LasDimension ld = LasAttributes[config->attrs[i]];
        ght_dimension_new(&dim);
        ght_dimension_set_name(dim, ld.name);
        ght_dimension_set_description(dim, "");
        dim->scale = ld.scale;
        dim->offset = ld.offset;
        dim->type = ld.type;
        ght_schema_add_dimension(schema, dim);
    }

    return schema;
}


int
main (int argc, char **argv)
{
    Las2GhtConfig config;
    Las2GhtState state;

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
    
    printf ("got args: lasfile=%s ghtfile=%s\n", config.lasfile, config.ghtfile);

    /* Input file exists? */
    if ( ! l2g_fexists(config.lasfile) )
    {
        fprintf(stderr, "%s: LAS file '%s' does not exist\n", EXENAME, config.lasfile);
        return 1;
    }

    /* Can we open the LAS file? */
    state.reader = LASReader_Create(config.lasfile);
    if ( ! state.reader )
    {
        fprintf(stderr, "%s: unable to open LAS file '%s'\n", EXENAME, config.lasfile);
        return 1;
    }

    /* Get the header */
    state.header = LASReader_GetHeader(state.reader);
    if ( ! state.header) 
    {
        fprintf(stderr, "%s: unable to read LAS header in '%s'\n", EXENAME, config.lasfile);
        return 1;
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



    return 0;
}