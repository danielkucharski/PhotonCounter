//This program demonstrates how to read HTP data files
//Daniel Kucharski, July 2019

#include "pch.h"


///////////////////////////////////////
//standard libraries 

#include <iostream>
#include <limits>

#include <ctime>
#include <windows.h>//for directory listing
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <limits.h>
#include <cstring>
#include <conio.h>

#include <vector>
#include <iostream>

#include <math.h>
#include <errno.h>

#include <sstream>      // std::stringstream
#include <fstream>      // std::ofstream

#include <fcntl.h>//Allows the user to perform a variety of operations on open files.

#include <sys/types.h>

using namespace std;
///////////////////////////////////////


int main(int argc, const char *argv[])
{
	//execute with paramers:
	//- full path to the directory that contains FITS files for processing

	//get frame directory path
	char filename_in[500];
	char filename_out[500];
	sprintf_s(filename_in, "%s", "");
	sprintf_s(filename_out, "%s", "");

	FILE *FFin;//binary
	FILE *FFout;//text

	if (argc > 1)
	{
		//printf("%s\n", argv[0]);
		//printf("%s\n", argv[1]);

		sprintf_s(filename_in, "%s", argv[1]);
		sprintf_s(filename_out, "%s.txt", filename_in);

		fopen_s(&FFin, filename_in, "rb");
		fopen_s(&FFout, filename_out, "w");

		fseek(FFin, 0L, SEEK_END);//to the end
		unsigned long int size_B = ftell(FFin);//B
		fseek(FFin, 0L, SEEK_SET);//to the beginning

		int countB = 0;//bit counter

		unsigned short int version = 0;
		long Epoch_realtime_s = 0;
		long Epoch_realtime_ns = 0;

		const int fields = 10;
		unsigned short int iheader[fields];
		double dheader[fields];
		for (int i = 0; i < fields; i++)
		{
			iheader[i] = 0;
			dheader[i] = 0.;
		}

		double GPGGA_longitude_deg = 0.;
		double GPGGA_latitude_deg = 0.;
		double GPGGA_altitude_m = 0.;
		double GPGGA_SOD = 0.;
		unsigned char GPGGA_ind_quality = 0;
		unsigned char GPGGA_sat_in_view = 0;
		double GPRMC_longitude_deg = 0.;
		double GPRMC_latitude_deg = 0.;
		double GPRMC_MJD = 0.;
		double GPRMC_SOD = 0.;
		unsigned char GPRMC_status = 0;

		bool Read_header = true;
		unsigned short int iWord;
		long int iSample = 0;

		if (size_B > 0)
			while (countB < size_B)
			{
				if (Read_header)
				{
					//read header fields
					fread(&version, sizeof(version), 1, FFin);
					countB += sizeof(version);

					//pass start: UTC
					fread(&Epoch_realtime_s, sizeof(Epoch_realtime_s), 1, FFin); countB += sizeof(Epoch_realtime_s);
					fread(&Epoch_realtime_ns, sizeof(Epoch_realtime_ns), 1, FFin); countB += sizeof(Epoch_realtime_ns);

					for (int i = 0; i < fields; i++)
					{
						fread(&iheader[i], sizeof(iheader[i]), 1, FFin);
						countB += sizeof(iheader[i]);
					}

					//GPS data
					fread(&GPGGA_longitude_deg, sizeof(GPGGA_longitude_deg), 1, FFin); countB += sizeof(GPGGA_longitude_deg);
					fread(&GPGGA_latitude_deg, sizeof(GPGGA_latitude_deg), 1, FFin); countB += sizeof(GPGGA_latitude_deg);
					fread(&GPGGA_altitude_m, sizeof(GPGGA_altitude_m), 1, FFin); countB += sizeof(GPGGA_altitude_m);
					fread(&GPGGA_SOD, sizeof(GPGGA_SOD), 1, FFin); countB += sizeof(GPGGA_SOD);
					fread(&GPGGA_ind_quality, sizeof(GPGGA_ind_quality), 1, FFin); countB += sizeof(GPGGA_ind_quality);
					fread(&GPGGA_sat_in_view, sizeof(GPGGA_sat_in_view), 1, FFin); countB += sizeof(GPGGA_sat_in_view);
					fread(&GPRMC_longitude_deg, sizeof(GPRMC_longitude_deg), 1, FFin); countB += sizeof(GPRMC_longitude_deg);
					fread(&GPRMC_latitude_deg, sizeof(GPRMC_latitude_deg), 1, FFin); countB += sizeof(GPRMC_latitude_deg);
					fread(&GPRMC_MJD, sizeof(GPRMC_MJD), 1, FFin); countB += sizeof(GPRMC_MJD);
					fread(&GPRMC_SOD, sizeof(GPRMC_SOD), 1, FFin); countB += sizeof(GPRMC_SOD);
					fread(&GPRMC_status, sizeof(GPRMC_status), 1, FFin); countB += sizeof(GPRMC_status);

					for (int i = 0; i < fields; i++)
					{
						fread(&dheader[i], sizeof(dheader[i]), 1, FFin);
						countB += sizeof(dheader[i]);
					}

					Read_header = false;


					//store data
					double dMJD = 40587. + (double(Epoch_realtime_s) + double(Epoch_realtime_ns) / 1.e9) / 86400.0;
					int iMJD = int(dMJD);
					double sod = double(Epoch_realtime_s % 86400) + double(Epoch_realtime_ns) / 1.e9;

					fprintf(FFout, "UTC_time\n", "");
					fprintf(FFout, "MJD\t%d\n", iMJD);
					fprintf(FFout, "SOD\t%.6f\n", sod);
					//fprintf(FFout, "GPS_data\n", "");
					//fprintf(FFout, "Longitude_W_deg\t%.6f\n", GPGGA_longitude_deg);
					//fprintf(FFout, "Latitude_N_deg\t%.6f\n", GPGGA_latitude_deg);
					//fprintf(FFout, "Altitude_m\t%.3f\n", GPGGA_altitude_m);
					fprintf(FFout, "Data_sampling_interval_us\t20\n", "");

				}
				//done with header fields

				//Read data records
				fread(&iWord, sizeof(unsigned short int), 1, FFin); countB += sizeof(unsigned short int);

				//is this record valid? 0=no, 1=yes
				bool flag_valid = iWord & 0x8000;//16th bit HEX mask

				//pps line status, 0=low, 1=high
				bool flag_pps = iWord & 0x4000;//15th bit HEX mask

				//12-bit photon count number
				int count = iWord & 0xFFF;//0xFFF: 0000111111111111

				if (flag_valid)
					fprintf(FFout, "%d\t%d\n", iSample, count);

				iSample++;
			}

		fclose(FFin);
		fclose(FFout);
	}


	return(0);
}

