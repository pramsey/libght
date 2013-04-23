/***********************************************************************
* las2ght.c
*
*   convert las file to ght file
*
***********************************************************************/

#include "liblas/capi/liblas.h"
#include "ght.h"
#include <string.h>

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
    int something;
} Las2GhtState;

static void
usage()
{
    printf("las2ght, version %d.%d\n\n", GHT_VERSION_MAJOR, GHT_VERSION_MINOR);
    printf("usage: las2ght --lasfile <lasfile> --ghtfile <ghtfile>\n\n");
}

static void
las2ght_config_free(Las2GhtConfig *config)
{
    if ( config->lasfile )
        free(config->lasfile);
    if ( config->ghtfile )
        free(config->ghtfile);
}

static Las2GhtConfig *
getopts(int argc, char **argv)
{
    int ch = 0;
    Las2GhtConfig *config;

    /* options descriptor */
    static struct option longopts[] = 
    {
        { "lasfile", required_argument, NULL, 'l' },
        { "ghtfile", required_argument, NULL, 'g' },
        { NULL, 0, NULL, 0 }
    };

    config = malloc(sizeof(Las2GhtConfig));
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
            default:
            {
                las2ght_config_free(config);
                return NULL;
            }
        }
    }
    
    if ( ! (config->lasfile && config->ghtfile) )
    {
        las2ght_config_free(config);
        config = NULL;
    }
    return config;
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
        return 1;
    }

    /* Parse command line options and set configuration */
    config = getopts(argc, argv);
    if ( ! config )
    {
        usage();
        return 1;
    }

    printf ("got args: lasfile=%s ghtfile=%s\n", config->lasfile, config->ghtfile);

    

    return 0;
}