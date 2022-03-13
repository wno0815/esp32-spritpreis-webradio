
#define DEBUGGING

#if defined(DEBUGGING)
#define DEB_B(...)    Serial.begin(__VA_ARGS__)
#define DEB_H()       Serial.println(F("----------------------------------------------------------------"));
#define DEB_P(...)    Serial.print(__VA_ARGS__)
#define TRC_P(...)  { Serial.printf("%s:%d - %s\n", __FILE__, __LINE__,  __PRETTY_FUNCTION__); Serial.print(__VA_ARGS__) }
#define DEB_PL(...)   Serial.println(__VA_ARGS__)
#define TRC_PL(...) { Serial.printf("%s:%d - %s\n", __FILE__, __LINE__,  __PRETTY_FUNCTION__); Serial.println(__VA_ARGS__) }
#define DEB_PF(...)   Serial.printf(__VA_ARGS__)
#define TRC_PF(...) { Serial.printf("%s:%d - %s\n", __FILE__, __LINE__,  __PRETTY_FUNCTION__); Serial.printf(__VA_ARGS__) }
#define TRACE()       Serial.printf("%s:%d - %s\n", __FILE__, __LINE__,  __PRETTY_FUNCTION__)
#else
#define DEB_B(...) 
#define DEB_H() 
#define DEB_P(...) 
#define TRC_P(...) 
#define DEB_PL(...) 
#define TRC_PL(...) 
#define DEB_PF(...) 
#define TRC_PF(...) 
#define TRACE() 
#endif
