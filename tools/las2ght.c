/***********************************************************************
* las2ght.c
*
*   convert las file to ght file
*
***********************************************************************/

#include "liblas/capi/liblas.h"
#include "ght.h"

typedef struct 
{
    char *lasfile;
    char *ghtfile;
} Las2GhtConfig;

typedef struct 
{
    int something;
} Las2GhtState;

static void
usage()
{
    printf("las2ght, version %d.%d\n\n", GHT_VERSION_MAJOR, GHT_VERSION_MINOR);
    printf("usage: las2ght --lasfile <lasfile> --ghtfile <ghtfile>\n\n");
}

int
main (int argc, char **argv)
{
    Las2GhtConfig *config;
    Las2GhtState *state;

    /* If no options are specified, display usage */
    if (argc <= 1)
    {
        usage();
        exit(0);
    }

    /* Parse command line options and set configuration */
    config = malloc(sizeof(Las2GhtConfig));
    memset(config, 0, sizeof(Las2GhtConfig));

}