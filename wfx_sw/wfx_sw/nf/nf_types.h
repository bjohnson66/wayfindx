/*
 * nf_types.h
 * @brief Header file containing common types and definitions for the navigation fetch (NV) CSC.
 *
 * This file provides definitions for common data types and sizes associated with the NV CSC.
 */ 


#ifndef NF_TYPES_H_
#define NF_TYPES_H_

#define GGA_INDICATOR_SIZE 1 /**< Size of the N/S and E/W indicators in the GGA message */

#define LLA_LONG_BUFFER_SIZE 10 /**< Size of the buffer for storing longitude in LLA format */
#define LLA_LAT_BUFFER_SIZE 11 /**< Size of the buffer for storing latitude in LLA format */
#define LLA_ALT_BUFFER_SIZE 6 /**< Size of the buffer for storing altitude in LLA format */

// Size definitions for components of the GGA message
#define GGA_UTC_BUFFER_SIZE 9 /**< Size of the buffer for storing UTC time in the GGA message */
#define GGA_LAT_BUFFER_SIZE 10 /**< Size of the buffer for storing latitude in the GGA message */
#define GGA_LONG_BUFFER_SIZE 11 /**< Size of the buffer for storing longitude in the GGA message */
#define GGA_SV_USD_BUFFER_SIZE 2 /**< Size of the buffer for storing satellites used in the GGA message */
#define GGA_HDOP_BUFFER_SIZE 3 /**< Size of the buffer for storing HDOP in the GGA message */
#define GGA_NUM_INDICATORS_ACTIVE 3 /**< Number of active indicators in the GGA message */
#define GGA_NUM_ITEMS_ACTIVE 8 /**< Number of active items in the GGA message */
#define GGA_ALTITUDE_BUFFER_SIZE 7 /**< Size of the buffer for storing altitude in the GGA message */
#define VTG_SPEED_BUFER_SIZE 7 /**< Size of the buffer for storing speed in the VTG message */

#define GGA_SIZE (GGA_UTC_BUFFER_SIZE + GGA_LAT_BUFFER_SIZE + GGA_LONG_BUFFER_SIZE + GGA_SV_USD_BUFFER_SIZE + GGA_HDOP_BUFFER_SIZE + GGA_NUM_INDICATORS_ACTIVE + GGA_NUM_ITEMS_ACTIVE) /**< Total size of the GGA message */

extern char utc_time[GGA_UTC_BUFFER_SIZE]; /**< UTC Time, e.g., "161229.487" */
extern char latitude[GGA_LAT_BUFFER_SIZE]; /**< Latitude, e.g., "3723.24756" */
extern char ns_indicator[GGA_INDICATOR_SIZE]; /**< N/S Indicator, 'N' for north or 'S' for south */
extern char longitude[GGA_LONG_BUFFER_SIZE]; /**< Longitude, e.g., "12158.34166" */
extern char ew_indicator[GGA_INDICATOR_SIZE]; /**< E/W Indicator, 'E' for east or 'W' for west */
extern char position_fix_indicator[GGA_INDICATOR_SIZE]; /**< Position Fix Indicator, see Table 1-4 */
extern char satellites_used[GGA_SV_USD_BUFFER_SIZE]; /**< Satellites Used, range 0 to 12 eg 07 */
extern char hdop[GGA_HDOP_BUFFER_SIZE]; /**< HDOP (Horizontal Dilution of Precision), e.g., "1.0" */
extern char msl_altitude[GGA_ALTITUDE_BUFFER_SIZE]; /**< Altitude above sea level in meters one decimal of precision. */

extern float latitudeLLA_float; /**< Latitude in degrees */
extern float longitudeLLA_float; /**< Longitude in degrees */
extern float altitudeLLA_float; /**< Altitude in meters */

extern char latitudeLLA_str[LLA_LAT_BUFFER_SIZE]; /**< Latitude in degrees */
extern char longitudeLLA_str[LLA_LONG_BUFFER_SIZE]; /**< Longitude in degrees */
extern char altitudeLLA_str[LLA_ALT_BUFFER_SIZE]; /**< Altitude in meters */

extern char speed[VTG_SPEED_BUFER_SIZE]; /**< Speed in km/hr */

#endif /* NF_TYPES_H_ */