from ctypes import *
from numpy.ctypeslib import ndpointer
import time
import cProfile
import glob



TMP_DLL = cdll.LoadLibrary("/home/nick/python/dllTests/libanalyseCurrentFile.so")


def testStart(path, settings):
    input_settings = [int(settings['number_of_negative']), int(settings['max_peak_jump']),
        int(settings['lattitude_delta']), int(settings['longitude_delta']),
        int(settings['max_peak_count']), int(settings['height_min']),
        int(settings['height_max']), int(settings['coordinate_lattitude']),
        int(settings['coordinate_longitude'])]

    for file_name in glob.glob(path + '/*'):
        if not file_name:
            break

        settings_array = (c_int * len(input_settings))(*input_settings)
        file_name_array = c_char_p(bytes(file_name, 'utf-8'))
        TMP_DLL.startAnalyseFile.restype = ndpointer(dtype=c_double, shape=(10,))

        data = TMP_DLL.startAnalyseFile(settings_array, file_name_array)

        data_to_return = [[str(data[0]), str(data[1])], [str(data[2]), str(data[3])], [str(data[4]), str(data[5])], [str(bool(data[6])), str(bool(data[7])), str(bool(data[8])), str(bool(data[9]))]]

#        print(data_to_return)
#        break


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

