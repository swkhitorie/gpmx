#include "rtklib.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <board_config.h>

#define PRGNAME     "rtkrcv"            /* program name */
#define CMDPROMPT   "rtkrcv> "          /* command prompt */
#define MAXARG      10                  /* max number of args in a command */
#define MAXCMD      128                 /* max length of a command */
#define MAXSTR      128                 /* max length of a stream */
#define OPTSDIR     "."                 /* default config directory */
#define OPTSFILE    "stm_teseo.conf"    /* default config file */
#define NAVIFILE    "rtkrcv.nav"        /* navigation save file */
#define STATFILE    "rtkrcv_.stat"    /* solution status file */
#define TRACEFILE   "rtkrcv_.trace"   /* debug trace file */

#define ESC_CLEAR   "\033[H\033[2J"     /* ansi/vt100 escape: erase screen */
#define ESC_RESET   "\033[0m"           /* ansi/vt100: reset attribute */
#define ESC_BOLD    "\033[1m"           /* ansi/vt100: bold */

#define SQRT(x)     ((x)<=0.0||(x)!=(x)?0.0:sqrt(x))

static rtksvr_t *p_svr;

prcopt_t stm_prcopt = {
    .mode    = PMODE_SINGLE, //PMODE_KINEMA,
    .soltype = 0,
    .nf      = 3,
    .navsys  = SYS_GPS | SYS_GAL | SYS_CMP,   /* mode,soltype,nf,navsys */
    15.0 * D2R, {{0, 0}},       /* elmin,snrmask */
    0, 1, 1, 1,                 /* sateph,modear,glomodear,bdsmodear */
    5, 0, 10, 1,                /* maxout,minlock,minfix,armaxiter */
    0, 0, 0, 0,                 /* estion,esttrop,dynamics,tidecorr */
    1, 0, 0, 0, 0,              /* niter,codesmooth,intpref,sbascorr,sbassatsel */
    0, 0,                       /* rovpos,refpos */
    {100.0, 100.0},             /* eratio[] */
    {100.0, 0.003, 0.003, 0.0, 1.0}, /* err[] */
    {30.0, 0.03, 0.3},          /* std[] */
    {1E-4, 1E-3, 1E-4, 1E-1, 1E-2, 0.0}, /* prn[] */
    5E-12,                      /* sclkstab */
    {3.0, 0.9999, 0.25, 0.1, 0.05}, /* thresar */
    0.0, 0.0, 0.05,             /* elmaskar,almaskhold,thresslip */
    30.0, 30.0, 30.0,           /* maxtdif,maxinno,maxgdop */
    {0}, {0}, {0},              /* baseline,ru,rb */
    {"", ""},                   /* anttype */
    {{0}}, {{0}}, {0}           /* antdel,pcv,exsats */
};

const solopt_t solopt_default = {
  SOLF_LLH, TIMES_GPST, 1, 3,    /* posf,times,timef,timeu */
  0, 1, 0, 0, 0, 0, 0,           /* degf,outhead,outopt,outvel,datum,height,geoid */
  0, 0, 0,                       /* solstatic,sstat,trace */
  { 0.0, 0.0 },                  /* nmeaintv */
};

#define prcopt stm_prcopt

static int timetype     = 0;             /* time format (0:gpst,1:utc,2:jst,3:tow) */
static int soltype      = 0;             /* sol format (0:dms,1:deg,2:xyz,3:enu,4:pyl) */
static int solflag      = 2;             /* sol flag (1:std+2:age/ratio/ns) */

static int strtype[] = {
    STR_SERIAL, STR_NONE, STR_NONE, STR_SERIAL, STR_NONE
};

static void *strpath[5] =
{
  "/dev/ttyS3",
  "/dev/ttyS0",
  "/dev/ttyS0",
  "/dev/ttyS2",
  "/dev/ttyS0",
};

static int strfmt[] = {
    STRFMT_RTCM3, STRFMT_RTCM3, STRFMT_RTCM3
};

//static int svrcycle     =10;            /* server cycle (ms) */
static int timeout      = 10000;        /* timeout time (ms) */
static int reconnect    = 10000;        /* reconnect interval (ms) */
static int nmeacycle    = 5000;         /* nmea request cycle (ms) */
static int buffsize     = 4096;         /* input buffer size (bytes) */
static int navmsgsel    = 0;            /* navigation mesaage select */
static int nmeareq      = 0;            /* nmea request type (0:off,1:lat/lon,2:single) */
static double nmeapos[] = {0, 0, 0};    /* nmea position (lat/lon/height) (deg,m) */
static char rcvcmds[3][MAXSTR] = {""};  /* receiver commands files */
static char startcmd[MAXSTR] = "";      /* start command */
static char stopcmd [MAXSTR] = "";      /* stop command */
//static int modflgr[256] ={0};           /* modified flags of receiver options */
//static int modflgs[256] ={0};           /* modified flags of system options */

//static int fswapmargin  =30;            /* file swap margin (s) */
static char sta_name[256] = "";         /* station name */

static filopt_t filopt  = {""};         /* file options */


/* receiver options table ----------------------------------------------------*/
#define TIMOPT  "0:gpst,1:utc,2:jst,3:tow"
#define CONOPT  "0:dms,1:deg,2:xyz,3:enu,4:pyl"
#define FLGOPT  "0:off,1:std+2:age/ratio/ns"
#define ISTOPT  "0:off,1:serial,2:file,3:membuf"
#define OSTOPT  "0:off,1:serial,2:file"
#define FMTOPT  "0:rtcm3"
#define NMEOPT  "0:off,1:latlon,2:single"
#define SOLOPT  "0:llh,1:xyz,2:enu,3:nmea,4:stat"
#define MSGOPT  "0:all,1:rover,2:base,3:corr"

static opt_t rcvopts[] =
{
//    {"console-passwd",  2,  (void *)passwd,              ""     },
//    {"console-timetype",3,  (void *)&timetype,           TIMOPT },
//    {"console-soltype", 3,  (void *)&soltype,            CONOPT },
//    {"console-solflag", 0,  (void *)&solflag,            FLGOPT },

  {"inpstr1-type",    3, (void *) &strtype[0],         ISTOPT },
  {"inpstr2-type",    3, (void *) &strtype[1],         ISTOPT },
  {"inpstr3-type",    3, (void *) &strtype[2],         ISTOPT },
  {"inpstr2-nmeareq", 3, (void *) &nmeareq,            NMEOPT },
  {"inpstr2-nmealat", 1, (void *) &nmeapos[0],         "deg"  },
  {"inpstr2-nmealon", 1, (void *) &nmeapos[1],         "deg"  },
  {"inpstr2-nmeahgt", 1, (void *) &nmeapos[2],         "m"    },
  {"outstr1-type",    3, (void *) &strtype[3],         OSTOPT },
  {"outstr2-type",    3, (void *) &strtype[4],         OSTOPT },
  {"", 0, NULL, ""}
};

/* start rtk server ----------------------------------------------------------*/
static int startsvr()
{
    static sta_t sta[MAXRCV] = {{""}};
    double pos[3], npos[3];
    char s1[3][MAXRCVCMD] = {"", "", ""}, *cmds[] = {NULL, NULL, NULL};
    char s2[3][MAXRCVCMD] = {"", "", ""}, *cmds_periodic[] = {NULL, NULL, NULL};
    char *ropts[] = {"", "", ""};
    char *paths[] =
    {
      strpath[0], strpath[1], strpath[2], strpath[3], strpath[4]
    };
    char errmsg[2048] = "";
    int i, stropt[8] = {0};

    // trace(3, "startsvr:\n");

    if (prcopt.refpos == 4) /* rtcm */
    {
      for (i = 0; i < 3; i++) { prcopt.rb[i] = 0.0; }
    }

    pos[0] = nmeapos[0] * D2R;
    pos[1] = nmeapos[1] * D2R;
    pos[2] = nmeapos[2];
    pos2ecef(pos, npos);

    /* open geoid data file */
    if (p_svr->solopt[0].geoid > 0 && !opengeoid(p_svr->solopt[0].geoid, filopt.geoid))
    {
      // trace(2, "geoid data open error: %s\n", filopt.geoid);
      // printf("geoid data open error: %s\n", filopt.geoid);
      board_printf("geoid data open error: %s\n", filopt.geoid);
    }

    /* set stream options */
    stropt[0] = timeout;
    stropt[1] = reconnect;
    stropt[2] = 1000;
    stropt[3] = buffsize;
    strsetopt(stropt);

    if (strfmt[2] == 8) { strfmt[2] = STRFMT_SP3; }

    /* start rtk server */
    if (!rtksvrstart(p_svr, p_svr->cycle, buffsize, strtype, paths, strfmt, navmsgsel,
                    cmds, cmds_periodic, ropts, nmeacycle, nmeareq, npos, &prcopt,
                    p_svr->solopt, NULL, errmsg))
    {
      board_printf("rtk server start error (%s)\n", errmsg);
      return 0;
    }
    return 1;
}

/* stop rtk server -----------------------------------------------------------*/
static void stopsvr()
{
  char s[3][MAXRCVCMD] = {"", "", ""}, *cmds[] = {NULL, NULL, NULL};
  int i, ret;

  trace(3, "stopsvr:\n");

  if (!p_svr->state) { return; }

  /* stop rtk server */
  rtksvrstop(p_svr, cmds);

  // /* execute stop command */
  // if (*stopcmd && (ret = system(stopcmd)))
  // {
  //   trace(2, "command exec error: %s (%d)\n", stopcmd, ret);
  //   printf("command exec error: %s (%d)\n", stopcmd, ret);
  // }
  // if (p_svr->solopt[0].geoid > 0) { closegeoid(); }

  board_printf("stop rtk server\n");
}


/* print time ----------------------------------------------------------------*/
static void prtime(gtime_t time)
{
    double tow;
    int week;
    char tstr[64] = "";

    if (timetype == 1)
    {
      time2str(gpst2utc(time), tstr, 2);
    }
    else if (timetype == 2)
    {
      time2str(timeadd(gpst2utc(time), 9 * 3600.0), tstr, 2);
    }
    else if (timetype == 3)
    {
      tow = time2gpst(time, &week);
      sprintf(tstr, "  %04d %9.2f", week, tow);
    }
    else time2str(time, tstr, 1);
    board_printf("%s ", tstr);
}


/* print solution ------------------------------------------------------------*/
static void prsolution(const sol_t *sol, const double *rb)
{
    const char *solstr[] = {"------", "FIX", "FLOAT", "SBAS", "DGPS", "SINGLE", "PPP", ""};
    double pos[3] = {0}, Qr[9], Qe[9] = {0}, dms1[3] = {0}, dms2[3] = {0}, bl[3] = {0};
    double enu[3] = {0}, pitch = 0.0, yaw = 0.0, len;
    int i;

    trace(4, "prsolution:\n");

    if (sol->time.time == 0 || !sol->stat) { return; }
    prtime(sol->time);
    board_printf("(%-6s)", solstr[sol->stat]);

    if (norm(sol->rr, 3) > 0.0 && norm(rb, 3) > 0.0)
    {
      for (i = 0; i < 3; i++) { bl[i] = sol->rr[i] - rb[i]; }
    }
    len = norm(bl, 3);
    Qr[0] = sol->qr[0];
    Qr[4] = sol->qr[1];
    Qr[8] = sol->qr[2];
    Qr[1] = Qr[3] = sol->qr[3];
    Qr[5] = Qr[7] = sol->qr[4];
    Qr[2] = Qr[6] = sol->qr[5];

    if (soltype == 0)
    {
        if (norm(sol->rr, 3) > 0.0)
        {
            ecef2pos(sol->rr, pos);
            covenu(pos, Qr, Qe);
            deg2dms(pos[0]*R2D, dms1, 4);
            deg2dms(pos[1]*R2D, dms2, 4);
            if (p_svr->solopt[0].height == 1) { pos[2] -= geoidh(pos); } /* geodetic */
        }
        board_printf(" %s:%2.0f %02.0f %07.4f", pos[0] < 0 ? "S" : "N", fabs(dms1[0]), dms1[1], dms1[2]);
        board_printf(" %s:%3.0f %02.0f %07.4f", pos[1] < 0 ? "W" : "E", fabs(dms2[0]), dms2[1], dms2[2]);
        board_printf(" H:%8.3f", pos[2]);
        if (solflag & 1)
        {
            board_printf(" (N:%6.3f E:%6.3f U:%6.3f)", SQRT(Qe[4]), SQRT(Qe[0]), SQRT(Qe[8]));
        }
    }
    else if (soltype == 1)
    {
        if (norm(sol->rr, 3) > 0.0)
        {
            ecef2pos(sol->rr, pos);
            covenu(pos, Qr, Qe);
            if (p_svr->solopt[0].height == 1) { pos[2] -= geoidh(pos); } /* geodetic */
        }
        board_printf(" %s:%11.8f", pos[0] < 0.0 ? "S" : "N", fabs(pos[0])*R2D);
        board_printf(" %s:%12.8f", pos[1] < 0.0 ? "W" : "E", fabs(pos[1])*R2D);
        board_printf(" H:%8.3f", pos[2]);
        if (solflag & 1)
        {
            board_printf(" (E:%6.3f N:%6.3f U:%6.3fm)", SQRT(Qe[0]), SQRT(Qe[4]), SQRT(Qe[8]));
        }
    }
    else if (soltype == 2)
    {
        board_printf(" X:%12.3f", sol->rr[0]);
        board_printf(" Y:%12.3f", sol->rr[1]);
        board_printf(" Z:%12.3f", sol->rr[2]);
        if (solflag & 1)
        {
            board_printf(" (X:%6.3f Y:%6.3f Z:%6.3f)", SQRT(Qr[0]), SQRT(Qr[4]), SQRT(Qr[8]));
        }
    }
    else if (soltype == 3)
    {
        if (len > 0.0)
        {
            ecef2pos(rb, pos);
            ecef2enu(pos, bl, enu);
            covenu(pos, Qr, Qe);
        }
        board_printf(" E:%12.3f", enu[0]);
        board_printf(" N:%12.3f", enu[1]);
        board_printf(" U:%12.3f", enu[2]);
        if (solflag & 1)
        {
            board_printf(" (E:%6.3f N:%6.3f U:%6.3f)", SQRT(Qe[0]), SQRT(Qe[4]), SQRT(Qe[8]));
        }
    }
    else if (soltype == 4)
    {
        if (len > 0.0)
        {
            ecef2pos(rb, pos);
            ecef2enu(pos, bl, enu);
            covenu(pos, Qr, Qe);
            pitch = asin(enu[2] / len);
            yaw = atan2(enu[0], enu[1]);
            if (yaw < 0.0) { yaw += 2.0 * PI; }
        }
        board_printf(" P:%12.3f", pitch * R2D);
        board_printf(" Y:%12.3f", yaw * R2D);
        board_printf(" L:%12.3f", len);
        if (solflag & 1)
        {
            board_printf(" (E:%6.3f N:%6.3f U:%6.3f)", SQRT(Qe[0]), SQRT(Qe[4]), SQRT(Qe[8]));
        }
    }
    if (solflag & 2)
    {
        board_printf(" A:%4.1f R:%5.1f N:%2d", sol->age, sol->ratio, sol->ns);
    }
    board_printf("\n");
}

/* print status --------------------------------------------------------------*/
static void prstatus()
{
    rtk_t *rtk;
    const char *svrstate[] = {"stop", "run"}, *type[] = {"rover", "base", "corr"};
    const char *sol[] = {"-", "fix", "float", "SBAS", "DGPS", "single", "PPP", ""};
    const char *mode[] =
    {
      "single", "DGPS", "kinematic", "static", "moving-base", "fixed",
      "PPP-kinema", "PPP-static"
    };
    const char *freq[] = {"-", "L1", "L1+L2", "L1+L2+L5", "", "", ""};
    rtcm_t *rtcm[3];
    int i, j, n, thread, cycle, state, rtkstat, nsat0, nsat1, prcout, nave;
    int cputime, nb[3] = {0}, nmsg[3][10] = {{0}};
    char tstr[64], s[1024], *p;
    double runtime, rt[3] = {0}, dop[4] = {0}, rr[3], bl1 = 0.0, bl2 = 0.0;
    double azel[MAXSAT * 2], pos[3], vel[3], *del;

    trace(4, "prstatus:\n");

    rtksvrlock(p_svr);
    rtk = &p_svr->rtk;
    thread = (int)p_svr->thread;
    cycle = p_svr->cycle;
    state = p_svr->state;
    rtkstat = p_svr->rtk.sol.stat;
    nsat0 = p_svr->obs[0][0].n;
    nsat1 = p_svr->obs[1][0].n;
    cputime = p_svr->cputime;
    prcout = p_svr->prcout;
    nave = p_svr->nave;
    for_each_stream_input(i)
    nb[i] = p_svr->nb[i];
    for_each_stream_input(i)
    for (j = 0; j < 10; j++) {
      nmsg[i][j] = p_svr->nmsg[i][j];
    }

    if (p_svr->state) {
      runtime = (double)(tickget() - p_svr->tick) / 1000.0;
      rt[0] = floor(runtime / 3600.0);
      runtime -= rt[0] * 3600.0;
      rt[1] = floor(runtime / 60.0);
      rt[2] = runtime - rt[1] * 60.0;
    }
    for_each_stream_input(i) rtcm[i] = &(p_svr->rtcm[i]);


    for (i = n = 0; i < MAXSAT; i++)
    {
      if (rtk->opt.mode == PMODE_SINGLE && !rtk->ssat[i].vs) { continue; }
      if (rtk->opt.mode != PMODE_SINGLE && !rtk->ssat[i].vsat[0]) { continue; }
      azel[  n * 2] = rtk->ssat[i].azel[0];
      azel[1 + n * 2] = rtk->ssat[i].azel[1];
      n++;
    }
    dops(n, azel, 0.0, dop);

    board_printf("\n%s%-28s: %s%s\n", ESC_BOLD, "Parameter", "Value", ESC_RESET);
    board_printf("%-28s: %s %s\n", "rtklib version", VER_RTKLIB, PATCH_LEVEL);
    board_printf("%-28s: %d\n", "rtk server thread", thread);
    board_printf("%-28s: %s\n", "rtk server state", svrstate[state]);
    board_printf("%-28s: %d\n", "processing cycle (ms)", cycle);
    board_printf("%-28s: %s\n", "positioning mode", mode[rtk->opt.mode]);
    board_printf("%-28s: %s\n", "frequencies", freq[rtk->opt.nf]);
    board_printf("%-28s: %02.0f:%02.0f:%04.1f\n", "accumulated time to run", rt[0], rt[1], rt[2]);
    board_printf("%-28s: %d\n", "cpu time for a cycle (ms)", cputime);
    board_printf("%-28s: %d\n", "missing obs data count", prcout);
    board_printf("%-28s: %d,%d\n", "bytes in input buffer", nb[0], nb[1]);
    for (i = 0; i < 3; i++)
    {
      sprintf(s, "# of input data %s", type[i]);
      board_printf("%-28s: obs(%d),nav(%d),gnav(%d),ion(%d),sbs(%d),pos(%d),dgps(%d),ssr(%d),err(%d)\n",
            s, nmsg[i][0], nmsg[i][1], nmsg[i][6], nmsg[i][2], nmsg[i][3],
            nmsg[i][4], nmsg[i][5], nmsg[i][7], nmsg[i][9]);
    }

    for_each_stream_input(i)
    {
      p = s;
      *p = '\0';
      for (j = 1; j < 300; j++)
      {
        if (rtcm[i]->nmsg3[j] == 0) { continue; }
        p += sprintf(p, "%s%d(%d)", p > s ? "," : "", j + 1000, rtcm[i]->nmsg3[j]);
      }
      if (rtcm[i]->nmsg3[0] > 0)
      {
        sprintf(p, "%sother3(%d)", p > s ? "," : "", rtcm[i]->nmsg3[0]);
      }
      board_printf("%-15s %-9s: %s\n", "# of rtcm messages", type[i], s);
    }

    board_printf("%-28s: %s\n", "solution status", sol[rtkstat]);
    time2str(rtk->sol.time, tstr, 9);
    board_printf("%-28s: %s\n", "time of receiver clock rover", rtk->sol.time.time ? tstr : "-");
    board_printf("%-28s: %.3f,%.3f,%.3f,%.3f\n", "time sys offset (ns)", rtk->sol.dtr[1] * 1e9,
          rtk->sol.dtr[2] * 1e9, rtk->sol.dtr[3] * 1e9, rtk->sol.dtr[4] * 1e9);
    board_printf("%-28s: %.3f\n", "solution interval (s)", rtk->tt);
    board_printf("%-28s: %.3f\n", "age of differential (s)", rtk->sol.age);
    board_printf("%-28s: %.3f\n", "ratio for ar validation", rtk->sol.ratio);
    board_printf("%-28s: %d\n", "# of satellites rover", nsat0);
    board_printf("%-28s: %d\n", "# of satellites base", nsat1);
    board_printf("%-28s: %d\n", "# of valid satellites", rtk->sol.ns);
    board_printf("%-28s: %.1f,%.1f,%.1f,%.1f\n", "GDOP/PDOP/HDOP/VDOP", dop[0], dop[1], dop[2], dop[3]);
    board_printf("%-28s: %d\n", "# of real estimated states", rtk->na);
    board_printf("%-28s: %d\n", "# of all estimated states", rtk->nx);
    board_printf("%-28s: %.3f,%.3f,%.3f\n", "pos xyz single (m) rover",
          rtk->sol.rr[0], rtk->sol.rr[1], rtk->sol.rr[2]);
    if (norm(rtk->sol.rr, 3) > 0.0) { ecef2pos(rtk->sol.rr, pos); }
    else { pos[0] = pos[1] = pos[2] = 0.0; }
      board_printf("%-28s: %.8f,%.8f,%.3f\n", "pos llh single (deg,m) rover",
            pos[0]*R2D, pos[1]*R2D, pos[2]);
    ecef2enu(pos, rtk->sol.rr + 3, vel);
    board_printf("%-28s: %.3f,%.3f,%.3f\n", "vel enu (m/s) rover", vel[0], vel[1], vel[2]);
    board_printf("%-28s: %.3f,%.3f,%.3f\n", "pos xyz float (m) rover",
          rtk->x ? rtk->x[0] : 0, rtk->x ? rtk->x[1] : 0, rtk->x ? rtk->x[2] : 0);
    board_printf("%-28s: %.3f,%.3f,%.3f\n", "pos xyz float std (m) rover",
          rtk->P ? SQRT(rtk->P[0]) : 0, rtk->P ? SQRT(rtk->P[1 + 1 * rtk->nx]) : 0, rtk->P ? SQRT(rtk->P[2 + 2 * rtk->nx]) : 0);
    board_printf("%-28s: %.3f,%.3f,%.3f\n", "pos xyz fixed (m) rover",
          rtk->xa ? rtk->xa[0] : 0, rtk->xa ? rtk->xa[1] : 0, rtk->xa ? rtk->xa[2] : 0);
    board_printf("%-28s: %.3f,%.3f,%.3f\n", "pos xyz fixed std (m) rover",
          rtk->Pa ? SQRT(rtk->Pa[0]) : 0, rtk->Pa ? SQRT(rtk->Pa[1 + 1 * rtk->na]) : 0,
          rtk->Pa ? SQRT(rtk->Pa[2 + 2 * rtk->na]) : 0);
    board_printf("%-28s: %.3f,%.3f,%.3f\n", "pos xyz (m) base",
          rtk->rb[0], rtk->rb[1], rtk->rb[2]);
    if (norm(rtk->rb, 3) > 0.0) { ecef2pos(rtk->rb, pos); }
    else { pos[0] = pos[1] = pos[2] = 0.0; }
      board_printf("%-28s: %.8f,%.8f,%.3f\n", "pos llh (deg,m) base",
            pos[0]*R2D, pos[1]*R2D, pos[2]);
    board_printf("%-28s: %d\n", "# of average single pos base", nave);
    board_printf("%-28s: %s\n", "ant type rover", rtk->opt.pcvr[0].type);
    del = rtk->opt.antdel[0];
    board_printf("%-28s: %.3f %.3f %.3f\n", "ant delta rover", del[0], del[1], del[2]);
    board_printf("%-28s: %s\n", "ant type base", rtk->opt.pcvr[1].type);
    del = rtk->opt.antdel[1];
    board_printf("%-28s: %.3f %.3f %.3f\n", "ant delta base", del[0], del[1], del[2]);
    ecef2enu(pos, rtk->rb + 3, vel);
    board_printf("%-28s: %.3f,%.3f,%.3f\n", "vel enu (m/s) base",
          vel[0], vel[1], vel[2]);
    if (rtk->opt.mode > 0 && rtk->x && norm(rtk->x, 3) > 0.0)
    {
      for (i = 0; i < 3; i++) { rr[i] = rtk->x[i] - rtk->rb[i]; }
      bl1 = norm(rr, 3);
    }
    if (rtk->opt.mode > 0 && rtk->xa && norm(rtk->xa, 3) > 0.0)
    {
      for (i = 0; i < 3; i++) { rr[i] = rtk->xa[i] - rtk->rb[i]; }
      bl2 = norm(rr, 3);
    }
    board_printf("%-28s: %.3f\n", "baseline length float (m)", bl1);
    board_printf("%-28s: %.3f\n", "baseline length fixed (m)", bl2);

    rtksvrunlock(p_svr);
}

/* print satellite -----------------------------------------------------------*/
static void prsatellite(int nf)
{
    rtk_t *rtk;
    double az, el;
    char id[32];
    int i, j, fix, frq[] = {1, 2, 5, 7, 8, 6};

    trace(4, "prsatellite:\n");

    rtksvrlock(p_svr);
    rtk = &p_svr->rtk;

    if (nf <= 0 || nf > NFREQ) { nf = NFREQ; }
    board_printf("\n%s%3s %2s %5s %4s", ESC_BOLD, "SAT", "C1", "Az", "El");
    for (j = 0; j < nf; j++) { board_printf(" L%d", frq[j]); }
    for (j = 0; j < nf; j++) { board_printf("  Fix%d", frq[j]); }
    for (j = 0; j < nf; j++) { board_printf("  P%dRes", frq[j]); }
    for (j = 0; j < nf; j++) { board_printf("   L%dRes", frq[j]); }
    for (j = 0; j < nf; j++) { board_printf("  Sl%d", frq[j]); }
    for (j = 0; j < nf; j++) { board_printf("  Lock%d", frq[j]); }
    for (j = 0; j < nf; j++) { board_printf(" Rj%d", frq[j]); }
    board_printf("%s\n", ESC_RESET);

    for (i = 0; i < MAXSAT; i++)
    {
        if (rtk->ssat[i].azel[1] <= 0.0) { continue; }
        satno2id(i + 1, id);
        board_printf("%3s %2s", id, rtk->ssat[i].vs ? "OK" : "-");
        az = rtk->ssat[i].azel[0] * R2D;
        if (az < 0.0) { az += 360.0; }
        el = rtk->ssat[i].azel[1] * R2D;
        board_printf(" %5.1f %4.1f", az, el);
        for (j = 0; j < nf; j++)
        {
          board_printf(" %2s", rtk->ssat[i].vsat[j] ? "OK" : "-");
        }
        for (j = 0; j < nf; j++)
        {
          fix = rtk->ssat[i].fix[j];
          board_printf(" %5s", fix == 1 ? "FLOAT" : (fix == 2 ? "FIX" : (fix == 3 ? "HOLD" : "-")));
        }
        for (j = 0; j < nf; j++)
        {
          board_printf("%7.3f", rtk->ssat[i].resp[j]);
        }
        for (j = 0; j < nf; j++)
        {
          board_printf("%8.4f", rtk->ssat[i].resc[j]);
        }
        for (j = 0; j < nf; j++)
        {
          board_printf(" %4d", rtk->ssat[i].slipc[j]);
        }
        for (j = 0; j < nf; j++)
        {
          board_printf(" %6d", rtk->ssat[i].lock [j]);
        }
        for (j = 0; j < nf; j++)
        {
          board_printf(" %3d", rtk->ssat[i].rejc [j]);
        }
        board_printf("\n");
    }
    rtksvrunlock(p_svr);
}

/* print observation data ----------------------------------------------------*/
static void probserv(int nf)
{
    obsd_t obs[MAXOBS * 2];
    char tstr[64], id[32];
    int i, j, n = 0, frq[] = {1, 2, 5, 7, 8, 6, 9};

    trace(4, "probserv:\n");

    rtksvrlock(p_svr);
    for (i = 0; i < p_svr->obs[0][0].n && n < MAXOBS * 2; i++)
    {
      obs[n++] = p_svr->obs[0][0].data[i];
    }
    for (i = 0; i < p_svr->obs[1][0].n && n < MAXOBS * 2; i++)
    {
      obs[n++] = p_svr->obs[1][0].data[i];
    }
    rtksvrunlock(p_svr);

    if (nf <= 0 || nf > NFREQ) { nf = NFREQ; }
    board_printf("\n%s%-22s %3s %s", ESC_BOLD, "      TIME(GPST)", "SAT", "R");
    for (i = 0; i < nf; i++) { board_printf("        P%d(m)", frq[i]); }
    for (i = 0; i < nf; i++) { board_printf("       L%d(cyc)", frq[i]); }
    for (i = 0; i < nf; i++) { board_printf("  D%d(Hz)", frq[i]); }
    for (i = 0; i < nf; i++) { board_printf(" S%d", frq[i]); }
    board_printf(" LLI%s\n", ESC_RESET);
    for (i = 0; i < n; i++)
    {
        time2str(obs[i].time, tstr, 2);
        satno2id(obs[i].sat, id);
        board_printf("%s %3s %d", tstr, id, obs[i].rcv);
        for (j = 0; j < nf; j++) { board_printf("%13.3f", obs[i].P[j]); }
        for (j = 0; j < nf; j++) { board_printf("%14.3f", obs[i].L[j]); }
        for (j = 0; j < nf; j++) { board_printf("%8.1f", obs[i].D[j]); }
        for (j = 0; j < nf; j++) { board_printf("%3.0f", obs[i].SNR[j]*SNR_UNIT); }
        for (j = 0; j < nf; j++) { board_printf("%2d", obs[i].LLI[j]); }
        board_printf("\n");
    }
}

/* print navigation data -----------------------------------------------------*/
static void prnavidata()
{
    eph_t *eph[MAXSAT];
#if defined(ENAGLO)
    geph_t *geph[MAXPRNGLO];
#endif
    double ion[8], utc[8];
    gtime_t time;
    char id[32], s1[64], s2[64], s3[64];
    int i, valid, prn;

    trace(4, "prnavidata:\n");

    rtksvrlock(p_svr);
    time = p_svr->rtk.sol.time;
    for (i = 0; i < MAXSAT; i++) { eph[i] = &p_svr->nav.eph[i]; }
#if defined(ENAGLO)
    for (i = 0; i < MAXPRNGLO; i++) { geph[i] = &p_svr->nav.geph[i]; }
#endif
    for (i = 0; i < 8; i++) { ion[i] = p_svr->nav.ion_gps[i]; }
    for (i = 0; i < 8; i++) { utc[i] = p_svr->nav.utc_gps[i]; }


    board_printf("\n%s%3s %3s %3s %3s %3s %3s %3s %19s %19s %19s %3s %3s%s\n",
          ESC_BOLD, "SAT", "S", "IOD", "IOC", "FRQ", "A/A", "SVH", "Toe", "Toc",
          "Ttr/Tof", "L2C", "L2P", ESC_RESET);
    for (i = 0; i < MAXSAT; i++)
    {
        if (!(satsys(i + 1, &prn) & (SYS_GPS | SYS_GAL | SYS_QZS | SYS_CMP)) ||
            eph[i]->sat != i + 1) { continue; }
        valid = eph[i]->toe.time != 0 && !eph[i]->svh &&
                fabs(timediff(time, eph[i]->toe)) <= MAXDTOE;
        satno2id(i + 1, id);
        if (eph[i]->toe.time != 0) { time2str(eph[i]->toe, s1, 0); }
        else strcpy(s1, "-");
        if (eph[i]->toc.time != 0) { time2str(eph[i]->toc, s2, 0); }
        else strcpy(s2, "-");
        if (eph[i]->ttr.time != 0) { time2str(eph[i]->ttr, s3, 0); }
        else strcpy(s3, "-");
        board_printf("%3s %3s %3d %3d %3d %3d %03X %19s %19s %19s %3d %3d\n",
              id, valid ? "OK" : "-", eph[i]->iode, eph[i]->iodc, 0, eph[i]->sva,
              eph[i]->svh, s1, s2, s3, eph[i]->code, eph[i]->flag);
    }
#if defined(ENAGLO)
    for (i = 0; i < MAXSAT; i++)
    {
        if (!(satsys(i + 1, &prn)&SYS_GLO) || geph[prn - 1]->sat != i + 1) { continue; }
        valid = geph[prn - 1]->toe.time != 0 && !geph[prn - 1]->svh &&
                fabs(timediff(time, geph[prn - 1]->toe)) <= MAXDTOE_GLO;
        satno2id(i + 1, id);
        if (geph[prn - 1]->toe.time != 0) { time2str(geph[prn - 1]->toe, s1, 0); }
        else strcpy(s1, "-");
        if (geph[prn - 1]->tof.time != 0) { time2str(geph[prn - 1]->tof, s2, 0); }
        else strcpy(s2, "-");
        board_printf("%3s %3s %3d %3d %3d %3d  %02X %19s %19s %19s %3d %3d\n",
              id, valid ? "OK" : "-", geph[prn - 1]->iode, 0, geph[prn - 1]->frq,
              geph[prn - 1]->age, geph[prn]->svh, s1, "-", s2, 0, 0);
    }
#endif
    board_printf("ION: %9.2E %9.2E %9.2E %9.2E %9.2E %9.2E %9.2E %9.2E\n",
          ion[0], ion[1], ion[2], ion[3], ion[4], ion[5], ion[6], ion[7]);
    board_printf("UTC: %9.2E %9.2E %9.2E %9.2E  LEAPS: %.0f\n", utc[0], utc[1],
          utc[2], utc[3], utc[4]);
    rtksvrunlock(p_svr);
}

/* print error/warning messages ----------------------------------------------*/
static void prerror()
{
    int n;
    trace(4, "prerror:\n");

    rtksvrlock(p_svr);
    if ((n = p_svr->rtk.neb) > 0) {
        p_svr->rtk.errbuf[n] = '\0';
        BOARD_PRINTF("%s\r\n", p_svr->rtk.errbuf);
        p_svr->rtk.neb = 0;
    }
    rtksvrunlock(p_svr);
}

/* print stream --------------------------------------------------------------*/
static void prstream()
{
    const char *ch[] = {
      "input rover", "input base", "input corr", "output sol1", "output sol2",
      "log rover", "log base", "log corr"
    };

    const char *type[] = {
      "-", "serial", "file"
    };
    const char *fmt[] = {"rtcm3", ""};
    const char *sol[] = {"llh", "xyz", "enu", "nmea", "stat", "-"};

    stream_t *stream[MAXSTRRTK];
    int i, *format[MAXSTRRTK];

    trace(4, "prstream:\n");

    rtksvrlock(p_svr);

    for_each_stream(i) stream[i] = &p_svr->stream[i];
    for_each_stream_input(i) format[i] = &p_svr->format[i];
    for_each_stream_output(i) format[i] = &p_svr->solopt[i - 3].posf;

    rtksvrunlock(p_svr);

    BOARD_PRINTF("\n%s%-12s %-8s %-5s %s %10s %7s %10s %7s %-24s %s%s\n", ESC_BOLD,
          "Stream", "Type", "Fmt", "S", "In-byte", "In-bps", "Out-byte", "Out-bps",
          "Path", "Message", ESC_RESET);

    for_each_stream(i)
    {
      printf("%-12s %-8s %-5s %s %10d %7d %10d %7d %-24.24s\n",
            ch[i], type[stream[i]->type], i < 3 ? fmt[*format[i]] : (i < 5 || i == 8 ? sol[*format[i]] : "-"),
            stream[i]->state < 0 ? "E" : (stream[i]->state ? "C" : "-"),
            stream[i]->inb, stream[i]->inr, stream[i]->outb, stream[i]->outr,
            stream[i]->path);
    }
}

int rtklib_main(int argc, char **argv)
{
  int i, start = 0, outstat = 0, trace = 0;
  char *dev = "", file[MAXSTR] = "";

  board_printf("rtklib main enter\r\n");

  p_svr = rtkmalloc(sizeof(rtksvr_t));

  // board_printf("type rtk_t mem: total:%d, member:%d, %d, %d \r\n", 
  //   sizeof(rtk_t), 
  //   sizeof(sol_t), sizeof(ambc_t)* MAXSAT, sizeof(ssat_t)*MAXSAT);

  // board_printf("type nav_t mem: total:%d %d %d\r\n", 
  //   sizeof(nav_t), sizeof(pcv_t)*MAXSAT, sizeof(dgps_t)*MAXSAT);

  // board_printf("type stream_t mem: total:%d \r\n", 
  //   sizeof(stream_t));

  // board_printf("type raw_t mem: total:%d \r\n", 
  //   sizeof(raw_t));

  // board_printf("type obs_t mem: total:%d %d\r\n", 
  //   sizeof(obs_t), sizeof(obsd_t));

  board_printf("type rtcm_t mem: total:%d %d %d tt:%d %d\r\n", 
    sizeof(rtcm_t), sizeof(obsd_t)*MAXOBS, sizeof(eph_t )*MAXSAT*2, sizeof(eph_t )*MAXSAT*4, sizeof(rtksvr_t));

  // board_printf("p_svr: %x %d %d %d\r\n", p_svr, sizeof(prcopt_t), sizeof(sol_t), sizeof(rtksvr_t) - 3*sizeof(rtcm_t));

    memset(p_svr, 0, sizeof(*p_svr));

    for (i = 0; i < CONFIG_RTKLIB_OUTPUT_STREAM; ++i) {
        memcpy(&p_svr->solopt[i], &solopt_default, sizeof(solopt_default));
    }


  /* initialize rtk server and monitor port */
  if (1 != rtksvrinit(p_svr)) {
      board_printf("rtksvrinit failed \r\n");
  }

  board_printf("end rtklib_main malloc \r\n");

  resetsysopts();
  getsysopts(&prcopt, p_svr->solopt, &filopt);

  if (outstat > 0)
  {
    rtkopenstat(STATFILE, outstat);
  }

  /* start rtk server */
  if (start)
  {
    startsvr();
  }

  /* stop rtk server */
  stopsvr();

  if (outstat > 0) { rtkclosestat(); }

  traceclose();
  return 0;
}


#include "gmsh.h"

int rtklib_version(int argc, char **argv)
{
    gsh_kprintf("%s (ver.%s %s)\n", PRGNAME, VER_RTKLIB, PATCH_LEVEL);
    return 0;
}
MSH_FUNCTION_EXPORT_CMD(rtklib_version, rtkversion, rtklib version display);

int rtklib_start(int argc, char **argv)
{
    if (!startsvr()) { 
        gsh_kprintf("rtk server start failed\n");
        return -1; 
    }
    gsh_kprintf("rtk server start\n");
    return 0;
}
MSH_FUNCTION_EXPORT_CMD(rtklib_start, rtkstart, rtklib process start);

int rtklib_stop(int argc, char **argv)
{
    stopsvr();
    gsh_kprintf("rtk server stop\n");
    return 0;
}
MSH_FUNCTION_EXPORT_CMD(rtklib_stop, rtkstop, rtklib process stop);

int rtklib_restart(int argc, char **argv)
{
    stopsvr();

    if (!startsvr()) {
        gsh_kprintf("rtk server start failed\n");
        return -1; 
    }
    gsh_kprintf("rtk server restart\n");
}
MSH_FUNCTION_EXPORT_CMD(rtklib_restart, rtkrestart, rtklib process restart);

int rtklib_solution_print(int argc, char **argv)
{
    int i;

    rtksvrlock(p_svr);
    for (i = 0; i < p_svr->nsol; i++) {
      // prsolution(&p_svr->solbuf[i], p_svr->rtk.rb);
    }
    p_svr->nsol = 0;

    rtksvrunlock(p_svr);

    return 0;
}
MSH_FUNCTION_EXPORT_CMD(rtklib_solution_print, rtksolprint, rtklib solution display);

int rtklib_status(int argc, char **argv)
{
    int i;

    prstatus();

    return 0;
}
MSH_FUNCTION_EXPORT_CMD(rtklib_status, rtkstatus, rtklib status display);

int rtklib_satellite(int argc, char **argv)
{
    int i;

    prsatellite(0);

    return 0;
}
MSH_FUNCTION_EXPORT_CMD(rtklib_satellite, rtksatellite, rtklib satellite display);

int rtklib_observ(int argc, char **argv)
{
    int i;

    probserv(0);

    return 0;
}
MSH_FUNCTION_EXPORT_CMD(rtklib_observ, rtkobserv, rtklib observ display);

int rtklib_navidata(int argc, char **argv)
{
    int i;

    prnavidata();

    return 0;
}
MSH_FUNCTION_EXPORT_CMD(rtklib_navidata, rtknavidata, rtklib navigation display);

int rtklib_solution(int argc, char **argv)
{
    int i;

    rtksvrlock(p_svr);

    for (i = 0; i < p_svr->nsol; i++)
    {
        prsolution(&p_svr->solbuf[i], p_svr->rtk.rb);
    }

    p_svr->nsol = 0;
    rtksvrunlock(p_svr);

    return 0;
}
MSH_FUNCTION_EXPORT_CMD(rtklib_solution, rtksolution, rtklib solution display);
