require 'mkmf'

dir_config("geoip")

if have_library('GeoIP', 'GeoIP_record_by_ipnum') and have_header('GeoIPCity.h')
  create_makefile('geoip_city')
end
