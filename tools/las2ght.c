/***********************************************************************
* las2ght.c
*
*   convert las file to ght file
*
***********************************************************************/

#include "liblas/capi/liblas.h"
#include "ght.h"
#include <string.h>

#define EXENAME "las2ght"

#ifdef HAVE_GETOPT_H
/* System implementation */
#include <getopt.h>
#else
/* Compatibility implementation */
#include "getopt.h"
#endif

typedef struct 
{
    char *lasfile;
    char *ghtfile;
} Las2GhtConfig;

typedef struct 
{
    LASReaderH reader;
    LASHeaderH header;
} Las2GhtState;

static void
usage()
{
    printf("%s, version %d.%d\n\n", EXENAME, GHT_VERSION_MAJOR, GHT_VERSION_MINOR);
    printf("usage: %s --lasfile <lasfile> --ghtfile <ghtfile>\n\n", EXENAME);
}

static void
las2ght_config_free(Las2GhtConfig *config)
{
    if ( config->lasfile )
        free(config->lasfile);
    if ( config->ghtfile )
        free(config->ghtfile);
}

static int
fexists(const char *filename)
{
    FILE *fd;
    if ( ! (fd = fopen(filename, "r")) )
        return 0;
    fclose(fd);
    return 1;
}

static int
getopts(int argc, char **argv, Las2GhtConfig *config)
{
    int ch = 0;

    /* options descriptor */
    static struct option longopts[] = 
    {
        { "lasfile", required_argument, NULL, 'l' },
        { "ghtfile", required_argument, NULL, 'g' },
        { "attrs", optional_argument, NULL, 'a' },
        { NULL, 0, NULL, 0 }
    };

    memset(config, 0, sizeof(Las2GhtConfig));

    while ( (ch = getopt_long(argc, argv, "g:l:", longopts, NULL)) != -1)
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
                // a - scan angle
                // i - intensity
                // n - number of returns for given pulse
                // r - number of this return
                // c - classification number
                // C - classification name
                // u - user data
                // p - point source ID
                // e - edge of flight line
                // d - direction of scan flag
                // R - red channel of RGB color
                // G - green channel of RGB color
                // B - blue channel of RGB color
                // M - vertex index number
                break;
            }
            default:
            {
                las2ght_config_free(config);
                return 0;
            }
        }
    }
    
    if ( ! (config->lasfile && config->ghtfile) )
    {
        las2ght_config_free(config);
        return 0;
    }
    return 1;
}

int
main (int argc, char **argv)
{
    Las2GhtConfig config;
    Las2GhtState state;

    /* If no options are specified, display usage */
    if (argc <= 1)
    {
        usage();
        return 1;
    }

    /* Parse command line options and set configuration */
    if ( ! getopts(argc, argv, &config) )
    {
        usage();
        return 1;
    }
    
    printf ("got args: lasfile=%s ghtfile=%s\n", config.lasfile, config.ghtfile);

    /* Input file exists? */
    if ( ! fexists(config.lasfile) )
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