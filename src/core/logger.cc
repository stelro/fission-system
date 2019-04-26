/* ========================================================================
   $File: logger.cc $
   $Date: Wed Apr 17 13:20:32 2019 $
   $Revision: $
   $Creator: Ro Stelmach $
   $Notice: (C) Copyright 2019 by Ro Orestis Stelmach. All Rights Reserved. $
   ======================================================================== */
#include "core/logger.hh"

namespace fn {

  namespace log {

    void info(const char *format, ...) noexcept {

      va_list args;
      va_start(args, format);
      process_log(LogType::INFO, format, args);
      va_end(args);
    }

    void finfo(const char *format, ...) noexcept {
#ifdef NDEBUG
      return;
#endif
      // Empty, will be used to log to file
    }

    void error(const char *format, ...) noexcept {
#ifdef NDEBUG
      return;
#endif
      va_list args;
      va_start(args, format);
      process_log(LogType::ERROR, format, args);
      va_end(args);
    }

    void warning(const char *format, ...) noexcept {
#ifdef NDEBUG
      return;
#endif
      va_list args;
      va_start(args, format);
      process_log(LogType::WARNING, format, args);
      va_end(args);
    }

    void fatal(const char *format, ...) noexcept {

      va_list args;
      va_start(args, format);
      process_log(LogType::FATAL, format, args);
      va_end(args);
      // Force exit application
      exit(EXIT_FAILURE);
    }

  } // namespace log

} // namespace fn
