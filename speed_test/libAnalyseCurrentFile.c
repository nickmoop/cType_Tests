#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netcdf.h>
#include <math.h>

#define ERR(e) {printf("Error: %s\n", nc_strerror(e));}
#define LEN(array) (sizeof(array) / sizeof(array[0]))
#define True 1
#define False 0
#define EMPTY -99999            //  MAYBE CHANGE??
#define SIZE_I 4
#define SIZE_J 180              //   WHY SO???
#define SIZE SIZE_I * SIZE_J



double * startAnalyseFile(int input_settings[], char *input_file_name);
int numberOfNegative(double array[], int size, int max_number_of_negative);
int coordinatesDelta(double array_of_coordinates[], int size, int coordinates_max_delta);
void setMaxValueAndIndex(double *max_value, int *index, double array[], int size);
void deleteElementFromArrayByIndex(double *array, int size, int index);
void getSettings(int settings[], int input_settings[]);
double * maxPeakJump(int settings[], double elec_dens[], int size, double height[]);
int isGlobalCalculation(double latitude, double longitude);
void getDataFromFile(char *file_name, double *edmaxlat, double *edmaxlon, double *edmaxalt, double *critfreq, double elec_dens[], double latitude[], double longitude[], double height[]);
void allRejectionsOfData(int settings[], double elec_dens[], double latitude[], double longitude[], double height[], int size, int *negative_flag, int *latitude_flag, int *longitude_flag, int *peak_jump_flag, int *index_of_max, double *max_value);



double * startAnalyseFile(int input_settings[], char *input_file_name) {

    double edmaxlat, edmaxlon, edmaxalt, critfreq;
    double elec_dens[SIZE], latitude[SIZE], longitude[SIZE], height[SIZE];
    static double r[10];
    int settings[9];

    getSettings(settings, input_settings);
    getDataFromFile(input_file_name, &edmaxlat, &edmaxlon, &edmaxalt, &critfreq, elec_dens, latitude, longitude, height);

    int size = LEN(elec_dens);
    int negative_flag;
    int latitude_flag;
    int longitude_flag;
    int peak_jump_flag;
    int index;
    double max_value;

    allRejectionsOfData(settings, elec_dens, latitude, longitude, height, size, &negative_flag, &latitude_flag, &longitude_flag, &peak_jump_flag, &index, &max_value);

    double TMP_latitude, TMP_longitude;
    double hmf2, f0f2, hmf2_calculated, f0f2_calculated;
    int latitude_local = settings[7];
    int longitude_local = settings[8];

    if (isGlobalCalculation(latitude_local, longitude_local) == True) {
        TMP_latitude = edmaxlat;
        TMP_longitude = edmaxlon;
    } else {
        TMP_latitude = 0;
        TMP_longitude = 0;
    }

    hmf2 = edmaxalt;
    f0f2 = critfreq;
    if ((negative_flag == False) || (latitude_flag == False) || (longitude_flag == False) || (peak_jump_flag == False)) {
        hmf2_calculated = EMPTY;
        f0f2_calculated = EMPTY;
    } else {
        hmf2_calculated = height[index];
        f0f2_calculated = sqrt(max_value / 12400);
    }

    r[0] = TMP_longitude;
    r[1] = TMP_latitude;
    r[2] = hmf2;
    r[3] = hmf2_calculated;
    r[4] = f0f2;
    r[5] = f0f2_calculated;
    r[6] = negative_flag;
    r[7] = latitude_flag;
    r[8] = longitude_flag;
    r[9] = peak_jump_flag;

    return r;
}


void allRejectionsOfData(int settings[], double elec_dens[], double latitude[], double longitude[], double height[], int size, int *negative_flag, int *latitude_flag, int *longitude_flag, int *peak_jump_flag, int *index_of_max, double *max_value) {

    double negative = settings[0];
    double peak_jump = settings[1];
    double latitude_delta = settings[2];
    double longitude_delta = settings[3];
    double peak_count = settings[4];
    int height_min = settings[5];
    int height_max = settings[6];
    int TMP_settings[4] = {peak_jump, peak_count, height_min, height_max};

    if (negative != 0) {
        *negative_flag = numberOfNegative(elec_dens, size, negative);
    } else {
        *negative_flag = True;
    }

    if (latitude_delta != 0) {
        *latitude_flag = coordinatesDelta(latitude, size, latitude_delta);
    } else {
        *latitude_flag = True;
    }

    if (longitude_delta != 0) {
        *longitude_flag = coordinatesDelta(longitude, size, longitude_delta);
    } else {
        *longitude_flag = True;
    }

    if (peak_jump != 0) {
        double *p;
        p = maxPeakJump(TMP_settings, elec_dens, size, height);
        *peak_jump_flag = *(p);
        *index_of_max = *(p + 1);
        *max_value = *(p + 2);
    } else {
        *peak_jump_flag = True;
        *max_value = EMPTY;
        *index_of_max = 0;

        for (int i = 0; i < size; i++) {
            if (*max_value < elec_dens[i]) {
                *max_value = elec_dens[i];
                *index_of_max = i;
            }
        }
    }
}


double * maxPeakJump(int settings[], double elec_dens[], int size, double height[]) {

    int max_peak_jump = settings[0];
    int max_peak_count = settings[1];
    int height_min = settings[2];
    int height_max = settings[3];
    int max_peak_jump_flag = True;
    int peak_count = 0;
    int index_of_max = 0;
    int check_flag = True;
    static double r[3];
    double max_value;

    while (max_peak_jump_flag) {

        setMaxValueAndIndex(&max_value, &index_of_max, elec_dens, size);

        if ((height[index_of_max] < height_min) || (height[index_of_max] > height_max)) {
            deleteElementFromArrayByIndex(height, size, index_of_max);
            deleteElementFromArrayByIndex(elec_dens, size, index_of_max);
            continue;
        }

        double average_value = max_value;

        if (((index_of_max + 1) != size) && (index_of_max != 0)) {

            if ((elec_dens[index_of_max + 1] != EMPTY) && (elec_dens[index_of_max - 1] != EMPTY)) {
                average_value = (elec_dens[index_of_max + 1] + elec_dens[index_of_max - 1]) / 2;

            }
        }

        if (max_value == 0) {
            check_flag = False;
            break;
        }

        double percentage = (max_value - average_value) / max_value * 100;

        if (percentage >= max_peak_jump) {
            peak_count++;
            deleteElementFromArrayByIndex(height, size, index_of_max);
            deleteElementFromArrayByIndex(elec_dens, size, index_of_max);
        } else {
            max_peak_jump_flag = False;
        }

        if (peak_count == max_peak_count) {
            check_flag = False;
            max_peak_jump_flag = False;
        }
    }

    r[0] = check_flag;
    r[1] = index_of_max;
    r[2] = max_value;

    return r;
}


int numberOfNegative(double array[], int size, int max_number_of_negative) {

    int negative_count = 0;
	int check_flag = True;

	for (int i = 0; i < size; i++) {
		if (array[i] < 0)
			negative_count++;
    }

	if (negative_count >= max_number_of_negative)
		check_flag = False;

	return check_flag;
}


int coordinatesDelta(double array_of_coordinates[], int size, int coordinates_max_delta) {

    int check_flag = True;
    double min_coordinate = 200;
    double max_coordinate = -200;
    double delta;

    for (int i = 0; i < size; i++) {

        if (array_of_coordinates[i] > max_coordinate)
            max_coordinate = array_of_coordinates[i];

        if (array_of_coordinates[i] < min_coordinate)
            min_coordinate = array_of_coordinates[i];
    }

    delta = fabs(max_coordinate - min_coordinate);
    if (delta >= coordinates_max_delta)
        check_flag = False;

    return check_flag;
}


void setMaxValueAndIndex(double *max_value, int *index, double array[], int size) {

    double current_max_value = -1000;
    int current_index = -1;

    for (int i = 0; i < size; i++) {
        if (array[i] > current_max_value) {
            current_max_value = array[i];
            current_index = i;
        }
    }

    *max_value = current_max_value;
    *index = current_index;
}


void deleteElementFromArrayByIndex(double array[], int size, int index) {

    for (int i = index; i < (size - 1); i++) {
        array[i] = array[i + 1];
    }

    array[size - 1] = EMPTY;
}


void getDataFromFile(char *file_name, double *edmaxlat, double *edmaxlon, double *edmaxalt, double *critfreq, double elec_dens[], double latitude[], double longitude[], double height[]) {

    int nc_file_id;
    int retval;
    int MSL_alt_id, ELEC_dens_id, GEO_lat_id ,GEO_lon_id;
    double TMP_data_0[SIZE_I][SIZE_J], TMP_data_1[SIZE_I][SIZE_J], TMP_data_2[SIZE_I][SIZE_J], TMP_data_3[SIZE_I][SIZE_J];

    if ((retval = nc_open(file_name, NC_NOWRITE, &nc_file_id)))
        ERR(retval);

    if ((retval = nc_get_att_double(nc_file_id, NC_GLOBAL, "edmaxlat", edmaxlat)))
        ERR(retval);
    if ((retval = nc_get_att_double(nc_file_id, NC_GLOBAL, "edmaxlon", edmaxlon)))
        ERR(retval);
    if ((retval = nc_get_att_double(nc_file_id, NC_GLOBAL, "edmaxalt", edmaxalt)))
        ERR(retval);
    if ((retval = nc_get_att_double(nc_file_id, NC_GLOBAL, "critfreq", critfreq)))
        ERR(retval);

    if ((retval = nc_inq_varid(nc_file_id, "MSL_alt", &MSL_alt_id)))
        ERR(retval);
    if ((retval = nc_inq_varid(nc_file_id, "ELEC_dens", &ELEC_dens_id)))
        ERR(retval);
    if ((retval = nc_inq_varid(nc_file_id, "GEO_lat", &GEO_lat_id)))
        ERR(retval);
    if ((retval = nc_inq_varid(nc_file_id, "GEO_lon", &GEO_lon_id)))
        ERR(retval);

    if ((retval = nc_get_var_double(nc_file_id, MSL_alt_id, &TMP_data_0[0][0])))
        ERR(retval);
    if ((retval = nc_get_var_double(nc_file_id, ELEC_dens_id, &TMP_data_1[0][0])))
        ERR(retval);
    if ((retval = nc_get_var_double(nc_file_id, GEO_lat_id, &TMP_data_2[0][0])))
        ERR(retval);
    if ((retval = nc_get_var_double(nc_file_id, GEO_lon_id, &TMP_data_3[0][0])))
        ERR(retval);

    for (int i = 0; i < SIZE_I; i++) {
        for (int j = 0; j < SIZE_J; j++) {

            elec_dens[i*SIZE_J + j] = TMP_data_1[i][j];
            latitude[i*SIZE_J + j] = TMP_data_2[i][j];
            longitude[i*SIZE_J + j] = TMP_data_3[i][j];
            height[i*SIZE_J + j] = TMP_data_0[i][j];

        }
    }

    if ((retval = nc_close(nc_file_id)))
        ERR(retval);
}


void getSettings(int settings[], int input_settings[]) {

    for (int i = 0; i < 9; i++) {
        settings[i] = input_settings[i];
    }
}


int isGlobalCalculation(double latitude, double longitude) {

    if ((latitude == 0) || (longitude == 0))
        return True;

    return False;
}

