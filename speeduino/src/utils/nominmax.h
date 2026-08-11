/**
 * @file 
 * @brief Workaround when min() & max() are defined as macros, which generates compile errors in
 * most C++ standard library implementations. 
 * 
 * This file will workaround this. It should probably be included *before* any standard library headers.
 * 
 */
#if defined(max)
#undef max
#endif

#if defined(min)
#undef min
#endif