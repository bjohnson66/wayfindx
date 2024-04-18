/**
 * @file ut_types.h
 * @brief Header file containing typedefs for common types.
 *
 * This file provides typedefs for common types to improve code readability and portability.
 */


#ifndef UT_TYPES_H_
#define UT_TYPES_H_

typedef unsigned char  uint8_t;  ///< 8-bit unsigned integer
typedef signed char    int8_t;   ///< 8-bit signed integer
typedef unsigned int   uint16_t; ///< 16-bit unsigned integer

typedef unsigned char boolean_t;  /**< Boolean type (true/false). */

#define false 0  /**< Definition of false as integer value 0. */
#define true 1   /**< Definition of true as integer value 1. */

#define STAT_MODE 0
#define NAV_MODE 1

#endif /* UT_TYPES_H_ */