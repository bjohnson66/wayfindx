/*
 * nf_types.h
 *
 * Created: 4/13/2024 1:20:13 AM
 *  Author: Bradley
 */ 


#ifndef NF_TYPES_H_
#define NF_TYPES_H_

#define GGA_INDICATOR_SIZE 1

//37 + 8 commas
#define GGA_UTC_BUFFER_SIZE 9
#define GGA_LAT_BUFFER_SIZE 10
#define GGA_LONG_BUFFER_SIZE 11
#define GGA_SV_USD_BUFFER_SIZE 2
#define GGA_HDOP_BUFFER_SIZE 3
#define GGA_NUM_INDICATORS_ACTIVE 3
#define GGA_NUM_ITEMS_ACTIVE 8

#define GGA_SIZE (GGA_UTC_BUFFER_SIZE + GGA_LAT_BUFFER_SIZE + GGA_LONG_BUFFER_SIZE + GGA_SV_USD_BUFFER_SIZE + GGA_HDOP_BUFFER_SIZE + GGA_NUM_INDICATORS_ACTIVE + GGA_NUM_ITEMS_ACTIVE)

extern char utc_time[GGA_UTC_BUFFER_SIZE];				// UTC Time, e.g., "161229.487"
extern char latitude[GGA_LAT_BUFFER_SIZE];				// Latitude, e.g., "3723.2475"
extern char longitude[GGA_LONG_BUFFER_SIZE];				// Longitude, e.g., "12158.3416"
extern char ew_indicator[GGA_INDICATOR_SIZE];				// E/W Indicator, 'E' for east or 'W' for west
extern char position_fix_indicator[GGA_INDICATOR_SIZE];	// Position Fix Indicator, see Table 1-4
extern char satellites_used[GGA_SV_USD_BUFFER_SIZE];		// Satellites Used, range 0 to 12 eg 07
extern char hdop[GGA_HDOP_BUFFER_SIZE];					// HDOP (Horizontal Dilution of Precision), e.g., "1.0"
extern char ns_indicator[GGA_INDICATOR_SIZE];				// N/S Indicator, 'N' for north or 'S' for south


#endif /* NF_TYPES_H_ */