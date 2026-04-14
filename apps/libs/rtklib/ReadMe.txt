
recommand:
    -DENAGLO,                                disable default
    -DDCONFIG_RTKLIB_DATUMTRANS              disable default
    -DCONFIG_RTKLIB_SBAS_CORR_ENABLE         disable default
    -DCONFIG_RTKLIB_SSR_ENABLE               disable default
    -DCONFIG_RTKLIB_PRECISE_EPH_CLK_ENABLE   disable default
    -DCONFIG_RTKLIB_POST_PROCESS_ENABLE      disable default
    -DCONFIG_RTKLIB_PPP_ENABLE               disable default
    -DCONFIG_RTKLIB_PRECISE_EPH_CLK_ENABLE   disable default
    -DCONFIG_RTKLIB_RTCM2_ENABLE             disable default
    -DCONFIG_RTKLIB_RTCM3_OUTPUT_ENABLE      disable default

datum.c       (not used)
ephemeris.c 
geoid.c       (use embedded model)
gis.c         (not used)
ionex.c
lambda.c
options.c
pntpos.c
postpos.c     (not used)
ppp_ar.c      (not used)
ppp.c         (not used)
preceph.c     (not used)
rtcm.c        (use rtcm3 part)
rtcm3.c
rtcm3e.c      (not used)
rtkcmn.c
rtkpos.c
rtksvr.c
sbas.c        (not used)
solution.c
stream.c                    (user implement)
tides.c
tle.c         (not used)
