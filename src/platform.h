#ifndef CTR_PLATFORM_H
#define CTR_PLATFORM_H

#if defined(__ANDROID__) || defined(__TERMUX__)
    #define CTR_ANDROID 1
    #define CTR_PLATFORM_ADI "Android (Termux)"
#elif defined(_WIN32) || defined(_WIN64)
    #define CTR_WINDOWS 1
    #define CTR_PLATFORM_ADI "Windows"
#elif defined(__APPLE__)
    #define CTR_MACOS 1
    #define CTR_PLATFORM_ADI "macOS"
#elif defined(__linux__)
    #define CTR_LINUX 1
    #define CTR_PLATFORM_ADI "Linux"
#else
    #define CTR_BILINMEYEN 1
    #define CTR_PLATFORM_ADI "Bilinmeyen"
#endif

#define CTR_SURUM "0.1.0"

#ifdef CTR_WINDOWS
    #define CTR_YOL_AYRAC '\\'
    #define CTR_SATIR_SONU "\r\n"
#else
    #define CTR_YOL_AYRAC '/'
    #define CTR_SATIR_SONU "\n"
#endif

void ctr_platform_yazdir(void);

#endif