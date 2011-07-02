#ifndef _DEBUG_HEADER
#define _DEBUG_HEADER

#ifndef DEBUGMACRO_ERRORSTREAM
 #define DEBUGMACRO_ERRORSTREAM std::cerr
#endif

inline void DebugOutputVariableDumpHelper1(){}
inline void DebugOutputVariableDumpHelper1(const char* str){if(std::strlen(str) != 0){DEBUGMACRO_ERRORSTREAM << str << " = ";}}
inline void DebugOutputVariableDumpHelper3(){}
inline void DebugOutputVariableDumpHelper3(const char* str){if(std::strlen(str) != 0){DEBUGMACRO_ERRORSTREAM << "              " << str << " = ";}}
inline void DebugOutputVariableDumpHelper2(){}
template<class T> inline void DebugOutputVariableDumpHelper2(const T& arg){DEBUGMACRO_ERRORSTREAM << arg << std::endl;}

#define VARDUMP(arg, ...)   (DebugOutputVariableDumpHelper1(#arg),DebugOutputVariableDumpHelper2(arg),VARDUMP_1(__VA_ARGS__))
#define VARDUMP_1(arg, ...) (DebugOutputVariableDumpHelper3(#arg),DebugOutputVariableDumpHelper2(arg),VARDUMP_2(__VA_ARGS__))
#define VARDUMP_2(arg, ...) (DebugOutputVariableDumpHelper3(#arg),DebugOutputVariableDumpHelper2(arg),VARDUMP_3(__VA_ARGS__))
#define VARDUMP_3(arg, ...) (DebugOutputVariableDumpHelper3(#arg),DebugOutputVariableDumpHelper2(arg),VARDUMP_4(__VA_ARGS__))
#define VARDUMP_4(arg, ...) (DebugOutputVariableDumpHelper3(#arg),DebugOutputVariableDumpHelper2(arg),VARDUMP_5(__VA_ARGS__))
#define VARDUMP_5(arg, ...) (DebugOutputVariableDumpHelper3(#arg),DebugOutputVariableDumpHelper2(arg),VARDUMP_6(__VA_ARGS__))
#define VARDUMP_6(arg, ...) (DebugOutputVariableDumpHelper3(#arg),DebugOutputVariableDumpHelper2(arg),VARDUMP_7(__VA_ARGS__))
#define VARDUMP_7(arg, ...) (DebugOutputVariableDumpHelper3(#arg),DebugOutputVariableDumpHelper2(arg),VARDUMP_8(__VA_ARGS__))
#define VARDUMP_8(arg, ...) (DebugOutputVariableDumpHelper3(#arg),DebugOutputVariableDumpHelper2(arg),VARDUMP_9(__VA_ARGS__))
#define VARDUMP_9(arg, ...) (DebugOutputVariableDumpHelper3(#arg),DebugOutputVariableDumpHelper2(arg))
#define DUMP(arg, ...)   (DebugOutputVariableDumpHelper1(#arg),DebugOutputVariableDumpHelper2(arg),DUMP_1(__VA_ARGS__))
#define DUMP_1(arg, ...) (DebugOutputVariableDumpHelper1(#arg),DebugOutputVariableDumpHelper2(arg),DUMP_2(__VA_ARGS__))
#define DUMP_2(arg, ...) (DebugOutputVariableDumpHelper1(#arg),DebugOutputVariableDumpHelper2(arg),DUMP_3(__VA_ARGS__))
#define DUMP_3(arg, ...) (DebugOutputVariableDumpHelper1(#arg),DebugOutputVariableDumpHelper2(arg),DUMP_4(__VA_ARGS__))
#define DUMP_4(arg, ...) (DebugOutputVariableDumpHelper1(#arg),DebugOutputVariableDumpHelper2(arg),DUMP_5(__VA_ARGS__))
#define DUMP_5(arg, ...) (DebugOutputVariableDumpHelper1(#arg),DebugOutputVariableDumpHelper2(arg),DUMP_6(__VA_ARGS__))
#define DUMP_6(arg, ...) (DebugOutputVariableDumpHelper1(#arg),DebugOutputVariableDumpHelper2(arg),DUMP_7(__VA_ARGS__))
#define DUMP_7(arg, ...) (DebugOutputVariableDumpHelper1(#arg),DebugOutputVariableDumpHelper2(arg),DUMP_8(__VA_ARGS__))
#define DUMP_8(arg, ...) (DebugOutputVariableDumpHelper1(#arg),DebugOutputVariableDumpHelper2(arg),DUMP_9(__VA_ARGS__))
#define DUMP_9(arg, ...) (DebugOutputVariableDumpHelper1(#arg),DebugOutputVariableDumpHelper2(arg))

#if defined(NDEBUG)
#define ASSERT_WMD(message, condition, dumpmethod) ;
#else
#define ASSERT_WMD(message, condition, dumpmethod) if(condition) ; else {\
  DEBUGMACRO_ERRORSTREAM << "ASSERTION FAILED!\n" \
  "  File      : " << __FILE__ << "\n" \
  "  Line      : " << __LINE__ << "\n" \
  "  Function  : " << DEBUGMACRO_FUNCTIONNAME << "\n" \
  "  Condition : " << #condition << "\n" \
  "  Message   : " << message << "\n" \
  "  Dump      : "; \
  dumpmethod; DEBUGMACRO_ABORT(); \
  }
#endif

#define ASSERT_WM(message, condition)                    ASSERT_WMD(message, condition, DEBUGMACRO_ERRORSTREAM << std::flush)
#define ASSERT(condition)                        ASSERT_WM("N/A", condition)

#define ASSERT_EQUALS_WMD(message, arg1, arg2, dumpmethod) ASSERT_WMD(message, (arg1) == (arg2), (VARDUMP(arg1, arg2), (dumpmethod)))
#define ASSERT_EQUALS_WM(message, arg1, arg2)              ASSERT_EQUALS_WMD(message, arg1, arg2, DEBUGMACRO_ERRORSTREAM << std::flush)
#define ASSERT_EQUALS(arg1, arg2)                          ASSERT_EQUALS_WM("N/A", arg1, arg2)

#define ASSERT_LT_WMD(message, arg1, arg2, dumpmethod)     ASSERT_WMD(message, (arg1) < (arg2), (VARDUMP(arg1, arg2), (dumpmethod)))
#define ASSERT_LT_WM(message, arg1, arg2)                  ASSERT_LT_WMD(message, arg1, arg2, DEBUGMACRO_ERRORSTREAM << std::flush)
#define ASSERT_LT(arg1, arg2)                              ASSERT_LT_WM("N/A", arg1, arg2)

#define ASSERT_LTE_WMD(message, arg1, arg2, dumpmethod)    ASSERT_WMD(message, (arg1) <= (arg2), (VARDUMP(arg1, arg2), (dumpmethod)))
#define ASSERT_LTE_WM(message, arg1, arg2)                 ASSERT_LTE_WMD(message, arg1, arg2, DEBUGMACRO_ERRORSTREAM << std::flush)
#define ASSERT_LTE(arg1, arg2)                             ASSERT_LTE_WM("N/A", arg1, arg2)

#define ASSERT_GT_WMD(message, arg1, arg2, dumpmethod)     ASSERT_WMD(message, (arg1) > (arg2), (VARDUMP(arg1, arg2), (dumpmethod)))
#define ASSERT_GT_WM(message, arg1, arg2)                  ASSERT_GT_WMD(message, arg1, arg2, DEBUGMACRO_ERRORSTREAM << std::flush)
#define ASSERT_GT(arg1, arg2)                              ASSERT_GT_WM("N/A", arg1, arg2)

#define ASSERT_GTE_WMD(message, arg1, arg2, dumpmethod)    ASSERT_WMD(message, (arg1) >= (arg2), (VARDUMP(arg1, arg2), (dumpmethod)))
#define ASSERT_GTE_WM(message, arg1, arg2)                 ASSERT_GTE_WMD(message, arg1, arg2, DEBUGMACRO_ERRORSTREAM << std::flush)
#define ASSERT_GTE(arg1, arg2)                             ASSERT_GTE_WM("N/A", arg1, arg2)

#define ASSERT_NEVERREACH_WD(dumpmethod)        ASSERT_WMD("Logic error", false, dumpmethod)
#define ASSERT_NEVERREACH( )            ASSERT_NEVERREACH_WD(DEBUGMACRO_ERRORSTREAM << std::flush)

#define ASSERT_NOTIMPLEMENTED_WD(dumpmethod)    ASSERT_WMD("Not implemented", false, dumpmethod)
#define ASSERT_NOTIMPLEMENTED( ) ASSERT_NEVERREACH_WD(DEBUGMACRO_ERRORSTREAM << std::flush)

#endif // #ifndef _DEBUG_HEADER

