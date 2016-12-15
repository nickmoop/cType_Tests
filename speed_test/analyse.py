import os
import glob
import time
from netCDF4 import Dataset
import rejection
import cProfile


def currentFile(file_name, settings):
    #   get data from file
    file_netCDF = Dataset(file_name, 'r', format = 'NETCDF3')
    edmaxlat = float(file_netCDF.edmaxlat)
    edmaxlon = float(file_netCDF.edmaxlon)
    edmaxalt = float(file_netCDF.edmaxalt)
    critfreq = float(file_netCDF.critfreq)
    list_of_elec_dens = file_netCDF.variables['ELEC_dens'][:]
    list_of_height = file_netCDF.variables['MSL_alt'][:]
    list_of_lattitude = file_netCDF.variables['GEO_lat'][:]
    list_of_longitude = file_netCDF.variables['GEO_lon'][:]
    file_netCDF.close()

    #   make all_checks temporary variable
    all_checks, index_of_max, max_value = allRejectionsOfData(settings, list_of_elec_dens, list_of_lattitude, list_of_longitude, list_of_height)

    try:				#MAKE ME! CHANGE? try... except... block
        if isGlobalCalculation(settings):
            lattitude = str(int(edmaxlat))
            longitude = str(int(edmaxlon))
        else:
            lattitude = '0'
            longitude = '0'

        hmf2 = str(round(edmaxalt, 2))
        f0f2 = str(round(critfreq, 2))
        if False in all_checks:
            hmf2_calculated = 'empty'
            f0f2_calculated = 'empty'
        else:
            hmf2_calculated = str(round((list_of_height[index_of_max]), 2))#???
            f0f2_calculated = str(round(((max_value / 100000 / 0.124) ** (1 / 2.0)), 2))

    except TypeError as error:
        print(error)
        print(file_name, edmaxlat, edmaxlon)

    current_file_data_processed = [[longitude, lattitude], [hmf2, hmf2_calculated], [f0f2, f0f2_calculated], all_checks]

    return current_file_data_processed


def allRejectionsOfData(settings, list_of_elec_dens, list_of_lattitude, list_of_longitude, list_of_height):
    #   get rejection parameters from arguments
    max_number_of_negative = float(settings['number_of_negative'])
    max_peak_jump = float(settings['max_peak_jump'])
    lattitude_delta = float(settings['lattitude_delta'])
    longitude_delta = float(settings['longitude_delta'])

    #   check negative in file
    if max_number_of_negative != 0:
        max_number_of_negative_check = rejection.numberOfNegative(list_of_elec_dens, max_number_of_negative)
    else:
        max_number_of_negative_check = True

    #   check geagrafic coordinates delta in file
    if lattitude_delta != 0:
        lattitude_delta_check = rejection.coordinatesDelta(list_of_lattitude, lattitude_delta)
    else:
        lattitude_delta_check = True

    if longitude_delta != 0:
        longitude_delta_check = rejection.coordinatesDelta(list_of_longitude, longitude_delta)
    else:
        longitude_delta_check = True

    #   check inhomogenity in file
    #   MAKE ME!
    if max_peak_jump != 0:
        tmp_list = rejection.maxPeakJump(settings, list_of_elec_dens, list_of_height)
        max_peak_jump_check = tmp_list[0]
        index_of_max = tmp_list[1]
        max_value = tmp_list[2]
    else:
        max_peak_jump_check = True
        i = 0
        max_value = 0
        index_of_max = 0
        for value in list_of_elec_dens:
            if max_value < list_of_elec_dens[i]:
                max_value = list_of_elec_dens[i]
                index_of_max = i
            i += 1

    #   make all_checks temporary variable
    all_checks = [max_number_of_negative_check, lattitude_delta_check, longitude_delta_check, max_peak_jump_check]

    return all_checks, index_of_max, max_value


def testStart(path, settings):
    for file_name in glob.glob(path + '/*.*'):
        if not file_name:
            break

        data = currentFile(file_name, settings)

#        print(data)
#        break


def isGlobalCalculation(settings):
    lattitude = int(settings['coordinate_lattitude'])
    longitude = int(settings['coordinate_longitude'])

    if lattitude == 0 or longitude == 0:
        return True

    else:
        return False

def forCProfile():
    settings = {
        'list_of_year':'2001,2002,2003,2004,2005,2006,2007,2008,2009,2010,2011,2012,2013,2014,2015',
        'list_of_activity':'min',
        'list_of_month':'12',
        'list_of_hour':'10',
        'lattitude_step':'5',
        'longitude_step':'15',
        'minute_step':'60',
        'day_step':'1',
        'month_middle':'22',
        'SA_average_method':'15_0',
        'number_of_negative':'100',
        'max_peak_jump':'20',
        'max_peak_count':'3',
        'lattitude_delta':'10',
        'longitude_delta':'10',
        'height_min':'180',
        'height_max':'500',
        'interval_min_low':'0',
        'interval_min_high':'500',
        'interval_mid_low':'0',
        'interval_mid_high':'0',
        'interval_max_low':'0',
        'interval_max_high':'0',
        'coordinate_lattitude':'0',
        'coordinate_longitude':'0',
        'point_name':'point_name',
        'local_LT':'0',
        'folder_to_read':'/media/nick/Elements/',
        'folder_to_write':'/home/nick/python/F107/__TMP_CProfile',
        'slash':'/',
        'list_of_missions' : 'cosmicnetCDF,gracenetCDF,champnetCDF'
    }
    paths = ['/home/nick/python/dllTests/cosmicnetcdf/2012/2012.123',
        '/home/nick/python/dllTests/cosmicnetcdf/2012/2012.122',
        '/home/nick/python/dllTests/cosmicnetcdf/2012/2012.124'
    ]

    for path in paths:
        testStart(path, settings)

cProfile.run('forCProfile()')

