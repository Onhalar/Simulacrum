#ifndef DEBUG_HEADER
#define DEBUG_HEADER

#ifdef DEBUG_ENABLED
    bool debugMode = true;
#else
    bool debugMode = false;
#endif

bool prettyOutput = true;

#endif // DEBUG_HEADER