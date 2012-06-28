#ifndef INC_APPINFO_H_
#define INC_APPINFO_H_

#define XSTR(x) STR(x)
#define STR(x) #x

#define APP_NAME "Stream Compositor"
#define APP_VER_MAJOR 1
#define APP_VER_MINOR 1
#define APP_AURTHOR "Xiaoyi Shi <ashi009@gmail.com>"

#define APP_TITLE APP_NAME " " XSTR(APP_VER_MAJOR) "." \
    XSTR(APP_VER_MINOR) " by " APP_AURTHOR

#endif /* INC_APPINFO_H_ */

