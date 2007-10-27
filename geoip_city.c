/* ry dahl <ry@tinyclouds.org> May 21, 2007 */
/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */
#include <ruby.h>
#include <GeoIP.h>
#include <GeoIPCity.h>

static VALUE cDB;
 
/* The argument is the filename of the GeoIPCity.dat file */
VALUE rb_geoip_new(VALUE self, VALUE filename) {
  GeoIP *gi;
  VALUE database = Qnil;

  if(gi = GeoIP_open(STR2CSTR(filename), GEOIP_MEMORY_CACHE)) {
    database = Data_Wrap_Struct(cDB, 0, GeoIP_delete, gi);
    rb_obj_call_init(database, 0, 0);
  }
  return database; 
}

/* helper  */
void rb_hash_sset(VALUE hash, const char *str, VALUE v) {
  rb_hash_aset(hash, ID2SYM(rb_intern(str)), v);
}

VALUE rb_record_to_hash (GeoIPRecord *record) 
{
  VALUE hash = rb_hash_new();

  if(record->country_code)
    rb_hash_sset(hash, "country_code", rb_str_new2(record->country_code));
  if(record->country_code3)
    rb_hash_sset(hash, "country_code3", rb_str_new2(record->country_code3));
  if(record->country_name)
    rb_hash_sset(hash, "country_name", rb_str_new2(record->country_name));
  if(record->region)
    rb_hash_sset(hash, "region", rb_str_new2(record->region));
  if(record->city)
    rb_hash_sset(hash, "city", rb_str_new2(record->city));
  if(record->postal_code)
    rb_hash_sset(hash, "postal_code", rb_str_new2(record->postal_code));
  if(record->latitude)
    rb_hash_sset(hash, "latitude", rb_float_new((double)record->latitude));
  if(record->longitude)
    rb_hash_sset(hash, "longitude", rb_float_new((double)record->longitude));
  if(record->dma_code)
    rb_hash_sset(hash, "dma_code", INT2NUM(record->dma_code));
  if(record->area_code)
    rb_hash_sset(hash, "area_code", INT2NUM(record->area_code));
  
  return hash;
}

/* Pass this function an IP address as a string, it will return a hash
 * containing all the information that the database knows about the IP
 *    db.look_up('24.24.24.24') 
 *    => {:city=>"Ithaca", :latitude=>42.4277992248535, 
 *        :country_code=>"US", :longitude=>-76.4981994628906, 
 *        :country_code3=>"USA", :dma_code=>555, 
 *        :country_name=>"United States", :area_code=>607, 
 *        :region=>"NY"} 
 */ 
VALUE rb_geoip_look_up(VALUE self, VALUE addr) {
  GeoIP *gi;
  GeoIPRecord *record = NULL;
  VALUE hash = Qnil; 
  
  Check_Type(addr, T_STRING);
  Data_Get_Struct(self, GeoIP, gi);
  if(record = GeoIP_record_by_addr(gi, STR2CSTR(addr))) {
    hash =  rb_record_to_hash(record); 
    GeoIPRecord_delete(record);
  }
  return hash;
}

void Init_geoip_city ()
{
  VALUE mGeoIP = rb_define_module("GeoIPCity");

  cDB = rb_define_class_under(mGeoIP, "Database", rb_cObject);
  rb_define_singleton_method(cDB, "new", rb_geoip_new, 1);
  rb_define_method(cDB, "look_up", rb_geoip_look_up, 1);
}
