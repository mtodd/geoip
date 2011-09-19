require 'mkmf'

dir_config("geoip")

unless have_func('iconv_open', 'iconv.h') or have_library('iconv', 'iconv_open', 'iconv.h')
  abort "you must have iconv library installed!"
end

if have_library('GeoIP', 'GeoIP_record_by_ipnum') and have_header('GeoIPCity.h')
  # Defines HAVE_GEOIP_ADDR_TO_NUM
  have_func('GeoIP_addr_to_num', 'GeoIP.h')

  # Defines HAVE_GEOIP_NUM_TO_ADDR
  have_func('GeoIP_num_to_addr', 'GeoIP.h')

  create_makefile('geoip')
else
  abort("you must have geoip c library installed!")
end
