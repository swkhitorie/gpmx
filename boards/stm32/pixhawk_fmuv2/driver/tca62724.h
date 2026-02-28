#ifndef RGB_TCA_62724_H_
#define RGB_TCA_62724_H_


#include <stdint.h>

#define RGB_COLOR_RED     (0x00)
#define RGB_COLOR_GREEN   (0x01)
#define RGB_COLOR_BLUE    (0x02)
#define RGB_COLOR_AMBER   (0x03)
#define RGB_COLOR_YELLOW  (0x04)
#define RGB_COLOR_PURPLE  (0x05)
#define RGB_COLOR_CYAN    (0x06)
#define RGB_COLOR_WHITE   (0x07)
#define RGB_COLOR_NONE    (0xff)

#ifdef __cplusplus
extern "C" {
#endif

int rgb_tca62724_init(int busn);

int rgb_tca62724_set(uint8_t color, uint8_t brightness);


#ifdef __cplusplus
}
#endif

#endif

