/*
 * nf_types.h
 *
 * Created: 4/13/2024 1:20:13 AM
 *  Author: Bradley
 */ 


#ifndef NF_TYPES_H_
#define NF_TYPES_H_

#define GGA_SIZE 37

typedef struct {
	 char utc_time[10];          // UTC Time, e.g., "161229.487"
	 char latitude[9];          // Latitude, e.g., "3723.2475"
	 char * ns_indicator;          // N/S Indicator, 'N' for north or 'S' for south
	 char longitude[10];         // Longitude, e.g., "12158.3416"
	 char * ew_indicator;          // E/W Indicator, 'E' for east or 'W' for west
	 char * position_fix_indicator; // Position Fix Indicator, see Table 1-4
	 char satellites_used[2];    // Satellites Used, range 0 to 12 eg 07
	 char hdop[3];               // HDOP (Horizontal Dilution of Precision), e.g., "1.0"
// 	 char msl_altitude[3];       // MSL (Mean Sea Level) Altitude, e.g., "9.0"
// 	 char altitude_units;        // Units for altitude, 'M' for meters
// 	 char geoid_separation[6];   // Geoid Separation
// 	 char geoid_units;           // Units for geoid separation, 'M' for meters
// 	 char diff_corr_age[3];      // Age of Differential Corrections
// 	 char diff_ref_station_id[5]; // Differential Reference Station ID
//	 char checksum[4];           // Checksum, e.g., "*18"
 }gga_struct_type;


#endif /* NF_TYPES_H_ */