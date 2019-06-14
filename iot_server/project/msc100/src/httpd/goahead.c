/*
    goahead.c -- Main program for GoAhead

    Usage: goahead [options] [documents] [IP][:port] ...
        Options:
        --auth authFile        # User and role configuration
        --background           # Run as a Linux daemon
        --home directory       # Change to directory to run
        --log logFile:level    # Log to file file at verbosity level
        --route routeFile      # Route configuration file
        --verbose              # Same as --log stdout:2
        --version              # Output version information

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes ***********************************/

#include "goahead.h"
#include "goahead_apps.h"
#include "gbk_utf8.h"

/********************************* Defines ************************************/

static int finished = 0;

/********************************* Forwards ***********************************/

static void initPlatform(void);
static void logHeader(void);
static void usage(void);
static int  get_local_ip_addr(char * net_dev, char *ip_buf, int len);
#if ME_UNIX_LIKE
static void sigHandler(int signo);
#endif


int goahead_init(void)
{
    int argc;
    char *argv[5];
    char    *argp, *home, *documents, *endpoints, *endpoint, *route, *auth, *tok, *lspec;
    int     argind;
    // ./mkit_msc -v ./web/ 10.101.70.32:8081

    argc  = 4;
    argv[0] = "./mkit_msc";
    argv[1] = "-v";
    argv[2] = "./web";
    argv[3] = "127.0.0.1:8081";

    char ip_buf[32] = {0};
    char argv_buf[32] = {0};
    if (0 == get_local_ip_addr(NET_DEV, ip_buf, sizeof(ip_buf)))
    {
        snprintf(argv_buf, sizeof(argv_buf), "%s:%d", ip_buf,  HTTP_PORT);
         argv[3] = argv_buf;
    }
    else
    {
        error("Cannot get host address for host %s: errno %d", NET_DEV, errno);
        return -1;
    }

    route = "route.txt";
    auth  = "auth.txt";

    for (argind = 1; argind < argc; argind++) {
        argp = argv[argind];
        if (*argp != '-') {
            break;

        } else if (smatch(argp, "--auth") || smatch(argp, "-a")) {
            if (argind >= argc) usage();
            auth = argv[++argind];

#if ME_UNIX_LIKE && !MACOSX
        } else if (smatch(argp, "--background") || smatch(argp, "-b")) {
            websSetBackground(1);
#endif

        } else if (smatch(argp, "--debugger") || smatch(argp, "-d") || smatch(argp, "-D")) {
            websSetDebug(1);

        } else if (smatch(argp, "--home")) {
            if (argind >= argc) usage();
            home = argv[++argind];
            if (chdir(home) < 0) {
                error("Cannot change directory to %s", home);
                exit(-1);
            }
        } else if (smatch(argp, "--log") || smatch(argp, "-l")) {
            if (argind >= argc) usage();
            logSetPath(argv[++argind]);

        } else if (smatch(argp, "--verbose") || smatch(argp, "-v")) {
            logSetPath("stdout:2");

        } else if (smatch(argp, "--route") || smatch(argp, "-r")) {
            route = argv[++argind];

        } else if (smatch(argp, "--version") || smatch(argp, "-V")) {
            printf("%s\n", ME_VERSION);
            exit(0);

        } else if (*argp == '-' && isdigit((uchar) argp[1])) {
            lspec = sfmt("stdout:%s", &argp[1]);
            logSetPath(lspec);
            wfree(lspec);

        } else {
            usage();
        }
    }
    documents = ME_GOAHEAD_DOCUMENTS;
    if (argc > argind) {
        documents = argv[argind++];
    }
    initPlatform();
    if (websOpen(documents, route) < 0) {
        error("Cannot initialize server. Exiting.");
        return -1;
    }
#if ME_GOAHEAD_AUTH
    if (websLoad(auth) < 0) {
        error("Cannot load %s", auth);
        return -1;
    }
#endif
    logHeader();
    if (argind < argc) {
        while (argind < argc) {
            endpoint = argv[argind++];
            if (websListen(endpoint) < 0) {
                return -1;
            }
        }
    } else {
        endpoints = sclone(ME_GOAHEAD_LISTEN);
        for (endpoint = stok(endpoints, ", \t", &tok); endpoint; endpoint = stok(NULL, ", \t,", &tok)) {
#if !ME_COM_SSL
            if (strstr(endpoint, "https")) continue;
#endif
            if (websListen(endpoint) < 0) {
                wfree(endpoints);
                return -1;
            }
        }
        wfree(endpoints);
    }

    gohead_apps_init();

#if ME_ROM && KEEP
    /*
        If not using a route/auth config files, then manually create the routes like this:
        If custom matching is required, use websSetRouteMatch. If authentication is required, use websSetRouteAuth.
     */
    websAddRoute("/", "file", 0);
#endif

#ifdef GOAHEAD_INIT
    /*
        Define your init function in main.me goahead.init, or
        configure with DFLAGS=GOAHEAD_INIT=myInitFunction
     */
    {
        extern int GOAHEAD_INIT();

        if (GOAHEAD_INIT() < 0) {
            exit(1);
        }
    }
#endif

#if ME_UNIX_LIKE && !MACOSX
    /*
        Service events till terminated
     */
    if (websGetBackground()) {
        if (daemon(0, 0) < 0) {
            error("Cannot run as daemon");
            return -1;
        }
    }
#endif


#if 0
    websServiceEvents(&finished);
    logmsg(1, "Instructed to exit");
    websClose();
#if WINDOWS
    windowsClose();
#endif
#endif

    return 0;
}

void goahead_eventloop(void)
{
    websServiceEvents(&finished);
    logmsg(1, "Instructed to exit");
    websClose();
}

static void logHeader(void)
{
    char    home[ME_GOAHEAD_LIMIT_STRING];

    getcwd(home, sizeof(home));
    logmsg(2, "Configuration for %s", ME_TITLE);
    logmsg(2, "---------------------------------------------");
    logmsg(2, "Version:            %s", ME_VERSION);
    logmsg(2, "BuildType:          %s", ME_DEBUG ? "Debug" : "Release");
    logmsg(2, "CPU:                %s", ME_CPU);
    logmsg(2, "OS:                 %s", ME_OS);
    logmsg(2, "Host:               %s", websGetServer());
    logmsg(2, "Directory:          %s", home);
    logmsg(2, "Documents:          %s", websGetDocuments());
    logmsg(2, "Configure:          %s", ME_CONFIG_CMD);
    logmsg(2, "---------------------------------------------");
}

static int get_local_ip_addr(char * net_dev, char *ip_buf, int len)
{
    int sockfd;
    struct sockaddr_in sin;
    struct ifreq ifr;
    char   *ipaddr;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1)
    {
        return -1;
    }
    strncpy(ifr.ifr_name, net_dev, IFNAMSIZ);    // 网卡多的得修改下
    ifr.ifr_name[IFNAMSIZ - 1] = 0;
    if (ioctl(sockfd, SIOCGIFADDR, &ifr) < 0)
    {
        close(sockfd);
        return -1;
    }
    memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
    ipaddr=inet_ntoa(sin.sin_addr);
    close(sockfd);
    snprintf(ip_buf, len, "%s", ipaddr);

    return 0;
}

static void usage(void) {
    fprintf(stderr, "\n%s Usage:\n\n"
        "  %s [options] [documents] [[IPaddress][:port] ...]\n\n"
        "  Options:\n"
#if ME_GOAHEAD_AUTH
        "    --auth authFile        # User and role configuration\n"
#endif
#if ME_UNIX_LIKE && !MACOSX
        "    --background           # Run as a Unix daemon\n"
#endif
        "    --debugger             # Run in debug mode\n"
        "    --home directory       # Change to directory to run\n"
        "    --log logFile:level    # Log to file file at verbosity level\n"
        "    --route routeFile      # Route configuration file\n"
        "    --verbose              # Same as --log stdout:2\n"
        "    --version              # Output version information\n\n",
        ME_TITLE, ME_NAME);
    exit(-1);
}


static void initPlatform(void)
{
#if ME_UNIX_LIKE
    signal(SIGTERM, sigHandler);
    #ifdef SIGPIPE
        signal(SIGPIPE, SIG_IGN);
    #endif
#elif ME_WIN_LIKE
    _fmode=_O_BINARY;
#endif
}

#if ME_UNIX_LIKE
static void sigHandler(int signo)
{
    finished = 1;
}
#endif
