/* ry dahl <ry@tinyclouds.org> May 21, 2007 */
/* Matt Todd <mtodd@highgroove.com> June 4, 2009 */
/* This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details. */
#include <ruby.h>
#include <GeoIP.h>
#include <GeoIPCity.h>

static VALUE mGeoIP;
static VALUE mGeoIP_City;
static VALUE mGeoIP_Organization;
static VALUE rb_geoip_memory;
static VALUE rb_geoip_filesystem;
static VALUE rb_geoip_index;

/* helpers */
void rb_hash_sset(VALUE hash, const char *str, VALUE v) {
  rb_hash_aset(hash, ID2SYM(rb_intern(str)), v);
}

int check_load_option(VALUE load_option) {
  if(load_option == rb_geoip_memory) {
    return GEOIP_MEMORY_CACHE;
  } else if(load_option == rb_geoip_filesystem) {
    return GEOIP_STANDARD;
  } else if(load_option == rb_geoip_index) {
    return GEOIP_INDEX_CACHE;
  } else {
    rb_raise(rb_eTypeError, "the second option must be :memory, :filesystem, or :index");
    return Qnil;
  }
}

/* Generic initializer */
static VALUE rb_geoip_database_new(VALUE mGeoIP_Database_Class, int argc, VALUE *argv, VALUE self)
{
  GeoIP *gi;
  VALUE database = Qnil;
  VALUE filename, load_option = Qnil, check_cache = Qnil;
  int flag;

  rb_scan_args(argc, argv, "12", &filename, &load_option, &check_cache);
  if(NIL_P(load_option))
    load_option = rb_geoip_memory;
  if(NIL_P(check_cache))
    check_cache = Qfalse;
  Check_Type(load_option, T_SYMBOL);

  if(flag = check_load_option(load_option)) flag;

  if(RTEST(check_cache)) flag |= GEOIP_CHECK_CACHE;

  if(gi = GeoIP_open(STR2CSTR(filename), flag)) {
    database = Data_Wrap_Struct(mGeoIP_Database_Class, 0, GeoIP_delete, gi);
    rb_obj_call_init(database, 0, 0);
  } else { 
    rb_sys_fail("Problem opening database");
  }
  return database;
}

/* GeoIP::City ***************************************************************/

VALUE rb_city_record_to_hash(GeoIPRecord *record) 
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

/* The first argument is the filename of the GeoIPCity.dat file 
 * load_option = :standard, :index, or :memory. default :memory
 * check_cache = true or false. default false
 * 
 * filesystem: read database from filesystem, uses least memory.
 * 
 * index: the most frequently accessed index portion of the database,
 *   resulting in faster lookups than :filesystem, but less memory usage than
 *   :memory.
 * 
 * memory: load database into memory, faster performance but uses more
 *   memory.
 */
static VALUE rb_geoip_city_new(int argc, VALUE *argv, VALUE self)
{
  return rb_geoip_database_new(mGeoIP_City, argc, argv, self);
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
VALUE rb_geoip_city_look_up(VALUE self, VALUE addr) {
  GeoIP *gi;
  GeoIPRecord *record = NULL;
  VALUE hash = Qnil; 
  
  Check_Type(addr, T_STRING);
  Data_Get_Struct(self, GeoIP, gi);
  if(record = GeoIP_record_by_addr(gi, STR2CSTR(addr))) {
    hash =  rb_city_record_to_hash(record);
    GeoIPRecord_delete(record);
  }
  return hash;
}

/* GeoIP::Organization *******************************************************/

/* GeoIP::Organization.new('/path/to/GeoIPOrg.dat', load_option)
 * load_option can be:
 * * :memory - load the data into memory, fastest (default)
 * * :filesystem - look up from filesystem, least memory intensive
 * * :index - stores in memory most recent queries
 * 
 */
static VALUE rb_geoip_org_new(int argc, VALUE *argv, VALUE self)
{
  return rb_geoip_database_new(mGeoIP_Organization, argc, argv, self);
}

/* Pass this function an IP address as a string, it will return a hash
 * containing all the information that the database knows about the IP:
 *    db.look_up('24.24.24.24') 
 *    => {:name => "Road Runner"}
 */ 
VALUE rb_geoip_org_look_up(VALUE self, VALUE addr) {
  GeoIP *gi;
  VALUE hash = rb_hash_new();
  char * name = NULL;
  
  Check_Type(addr, T_STRING);
  Data_Get_Struct(self, GeoIP, gi);
  if(name = GeoIP_name_by_addr(gi, STR2CSTR(addr))) {
    rb_hash_sset(hash, "name", rb_str_new2(name));
    free(name);
  }
  return hash;
}

/* GeoIP *********************************************************************/

/* Returns the numeric form of an IP address.
 * 
 * For example:
 * 24.24.24.24 => 404232216
 * 
 * This is used in order to be able to perform searches in CSV versions of the
 * data files or in SQL records if the data has been put there.
 */
VALUE rb_geoip_addr_to_num(VALUE self, VALUE addr) {
  Check_Type(addr, T_STRING);
  return UINT2NUM((unsigned long)_GeoIP_addr_to_num(STR2CSTR(addr)));
}

void Init_geoip()
{
  mGeoIP = rb_define_module("GeoIP");

  rb_geoip_memory = ID2SYM(rb_intern("memory")); 
  rb_geoip_filesystem = ID2SYM(rb_intern("filesystem")); 
  rb_geoip_index = ID2SYM(rb_intern("index")); 

  mGeoIP_City = rb_define_class_under(mGeoIP, "City", rb_cObject);
  rb_define_singleton_method(mGeoIP_City, "new",      rb_geoip_city_new,      -1);
  rb_define_method(mGeoIP_City,           "look_up",  rb_geoip_city_look_up,  1);
  
  mGeoIP_Organization = rb_define_class_under(mGeoIP, "Organization", rb_cObject);
  rb_define_singleton_method( mGeoIP_Organization, "new",     rb_geoip_org_new,     -1);
  rb_define_method(           mGeoIP_Organization, "look_up", rb_geoip_org_look_up, 1);
  
  rb_define_singleton_method(mGeoIP, "addr_to_num", rb_geoip_addr_to_num, 1);
}
