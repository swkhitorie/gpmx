#ifndef __MW_LOG_CONF_H__
#define __MW_LOG_CONF_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MW_LOG_ENABLED
/* Map your own trace mechanism or to map UTIL_ADV_TRACE see examples from CubeFw, i.e.:
                             do{ {UTIL_ADV_TRACE_COND_FSend(VL, T_REG_OFF, TS, __VA_ARGS__);} }while(0) */
#define MW_LOG(TS,VL, ...)
#else
#define MW_LOG(TS,VL, ...)
#endif

#ifdef __cplusplus
}
#endif

#endif
