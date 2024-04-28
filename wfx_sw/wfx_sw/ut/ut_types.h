/**
 * @file ut_types.h
 * @brief Header file containing typedefs for common types.
 *
 * This file provides typedefs for common types to improve code readability and portability.
 * @version 1.0
 * @copyright (C) 2024 Bradley Johnson and Abele Atresso
 */


#ifndef UT_TYPES_H_
#define UT_TYPES_H_

/**
 * @brief 8-bit unsigned integer type.
 */
typedef unsigned char  uint8_t;

/**
 * @brief 8-bit signed integer type.
 */
typedef signed char    int8_t;

/**
 * @brief 16-bit unsigned integer type.
 */
typedef unsigned int   uint16_t;

/**
 * @brief Boolean type (true/false).
 */
typedef unsigned char boolean_t;

/**
 * @brief Definition of false as boolean value 0.
 */
#define false ((boolean_t)0)

/**
 * @brief Definition of true as boolean value 1.
 */
#define true ((boolean_t)1)

#endif /* UT_TYPES_H_ */