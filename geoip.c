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
#include "iconv.h"

static VALUE mGeoIP;
static VALUE mGeoIP_City;
static VALUE mGeoIP_Country;
static VALUE mGeoIP_Organization;
static VALUE mGeoIP_ISP;
static VALUE mGeoIP_NetSpeed;
static VALUE mGeoIP_Domain;
static VALUE rb_geoip_memory;
static VALUE rb_geoip_filesystem;
static VALUE rb_geoip_index;

#ifndef HAVE_GEOIP_ADDR_TO_NUM
  // Support geoip <= 1.4.6
  #define GeoIP_addr_to_num _GeoIP_addr_to_num
#endif

#ifndef HAVE_GEOIP_NUM_TO_ADDR
  // Support geoip <= 1.4.6

  // Fixes a bug with 64bit architectures where this delcaration doesn't exist
  // in the GeoIP library causing segmentation faults.
  char *_GeoIP_num_to_addr(GeoIP* gi, unsigned long ipnum);
  #define GeoIP_num_to_addr(ipnum) _GeoIP_num_to_addr(NULL, ipnum)
#endif

/* helpers */
void rb_hash_sset(VALUE hash, const char *str, VALUE v) {
  rb_hash_aset(hash, ID2SYM(rb_intern(str)), v);
}

/*  pulled from http://blog.inventic.eu/?p=238 and
    https://github.com/Vagabond/erlang-iconv/blob/master/c_src/iconv_drv.c */
static VALUE encode_to_utf8_and_return_rb_str(char *value) {
  char dst[BUFSIZ];
  size_t srclen = strlen(value);
  size_t dstlen = srclen * 2;

  char * pIn = value;
  char * pOut = ( char*)dst;

  iconv_t cd = iconv_open("UTF-8","ISO-8859-1");
  iconv(cd, &pIn, &srclen, &pOut, &dstlen);
  iconv_close(cd);

  *(pOut++) = 0; /* ensure we null terminate */

  return rb_str_new2(dst);
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

  if(gi = GeoIP_open(StringValuePtr(filename), flag)) {
    database = Data_Wrap_Struct(mGeoIP_Database_Class, 0, GeoIP_delete, gi);
    rb_obj_call_init(database, 0, 0);
  } else {
    rb_sys_fail("Problem opening database");
  }
  return database;
}

/* Generic, single-value look up method */
static VALUE generic_single_value_lookup_response(char *key, char *value)
{
  VALUE result = rb_hash_new();
  if(value) {
    rb_hash_sset(result, key, encode_to_utf8_and_return_rb_str(value));
    return result;
  } else {
    return Qnil;
  }
}

/* GeoIP::City ***************************************************************/

VALUE rb_city_record_to_hash(GeoIPRecord *record)
{
  VALUE hash = rb_hash_new();

  if(record->country_code)
    rb_hash_sset(hash, "country_code", encode_to_utf8_and_return_rb_str(record->country_code));
  if(record->country_code3)
    rb_hash_sset(hash, "country_code3", encode_to_utf8_and_return_rb_str(record->country_code3));
  if(record->country_name)
    rb_hash_sset(hash, "country_name", encode_to_utf8_and_return_rb_str(record->country_name));
  if(record->region) {
    rb_hash_sset(hash, "region", encode_to_utf8_and_return_rb_str(record->region));
    rb_hash_sset(hash, "region_name", encode_to_utf8_and_return_rb_str(GeoIP_region_name_by_code(record->country_code, record->region)));
  }
  if(record->city)
    rb_hash_sset(hash, "city", encode_to_utf8_and_return_rb_str(record->city));
  if(record->postal_code)
    rb_hash_sset(hash, "postal_code", encode_to_utf8_and_return_rb_str(record->postal_code));
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
  if(record = GeoIP_record_by_addr(gi, StringValuePtr(addr))) {
    hash =  rb_city_record_to_hash(record);
    GeoIPRecord_delete(record);
  }
  return hash;
}

/* GeoIP::Country ************************************************************/

/* GeoIP::Country.new('/path/to/GeoIPCountry.dat')
 * load_option is not required for this database because it is ignored.
 */
static VALUE rb_geoip_country_new(int argc, VALUE *argv, VALUE self)
{
  return rb_geoip_database_new(mGeoIP_Country, argc, argv, self);
}

/* Pass this function an IP address as a string, it will return a hash
 * containing all the information that the database knows about the IP
 *    db.look_up('24.24.24.24')
 *    => {:country_code=>"US",
 *        :country_code3=>"USA",
 *        :country_name=>"United States"}
 */
VALUE rb_geoip_country_look_up(VALUE self, VALUE addr) {
  GeoIP *gi;
  VALUE hash = Qnil;
  int country_id;
  
  Check_Type(addr, T_STRING);
  Data_Get_Struct(self, GeoIP, gi);
  country_id = GeoIP_id_by_addr(gi, StringValuePtr(addr));
  if(country_id < 1) return Qnil;
  
  hash = rb_hash_new();
  rb_hash_sset(hash, "country_code", rb_str_new2(GeoIP_country_code[country_id]));
  rb_hash_sset(hash, "country_code3", rb_str_new2(GeoIP_country_code3[country_id]));
  rb_hash_sset(hash, "country_name", rb_str_new2(GeoIP_country_name[country_id]));
  
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
  Check_Type(addr, T_STRING);
  Data_Get_Struct(self, GeoIP, gi);
  return generic_single_value_lookup_response("name", GeoIP_name_by_addr(gi, StringValuePtr(addr)));
}

/* GeoIP::ISP *******************************************************/

/* GeoIP::ISP.new('/path/to/GeoIPISP.dat', load_option)
 * load_option can be:
 * * :memory - load the data into memory, fastest (default)
 * * :filesystem - look up from filesystem, least memory intensive
 * * :index - stores in memory most recent queries
 *
 */
static VALUE rb_geoip_isp_new(int argc, VALUE *argv, VALUE self)
{
  return rb_geoip_database_new(mGeoIP_ISP, argc, argv, self);
}

/* Pass this function an IP address as a string, it will return a hash
 * containing all the information that the database knows about the IP:
 *    db.look_up('24.24.24.24')
 *    => {:isp => "Road Runner"}
 */
VALUE rb_geoip_isp_look_up(VALUE self, VALUE addr) {
  GeoIP *gi;
  Check_Type(addr, T_STRING);
  Data_Get_Struct(self, GeoIP, gi);
  return generic_single_value_lookup_response("isp", GeoIP_name_by_addr(gi, StringValuePtr(addr)));
}

/* GeoIP::NetSpeed *******************************************************/

/* GeoIP::NetSpeed.new('/path/to/GeoIPNetSpeed.dat', load_option)
 * load_option can be:
 * * :memory - load the data into memory, fastest (default)
 * * :filesystem - look up from filesystem, least memory intensive
 * * :index - stores in memory most recent queries
 *
 */
static VALUE rb_geoip_netspeed_new(int argc, VALUE *argv, VALUE self)
{
  return rb_geoip_database_new(mGeoIP_NetSpeed, argc, argv, self);
}

/* Pass this function an IP address as a string, it will return a hash
 * containing all the information that the database knows about the IP:
 *    db.look_up('24.24.24.24')
 *    => {:netspeed => "Cable/DSL"}
 */
VALUE rb_geoip_netspeed_look_up(VALUE self, VALUE addr) {
  GeoIP *gi;
  Check_Type(addr, T_STRING);
  Data_Get_Struct(self, GeoIP, gi);
  return generic_single_value_lookup_response("netspeed", GeoIP_name_by_addr(gi, StringValuePtr(addr)));
}

/* GeoIP::Domain *******************************************************/

/* GeoIP::Domain.new('/path/to/GeoIPDomain.dat', load_option)
 * load_option can be:
 * * :memory - load the data into memory, fastest (default)
 * * :filesystem - look up from filesystem, least memory intensive
 * * :index - stores in memory most recent queries
 *
 */
static VALUE rb_geoip_domain_new(int argc, VALUE *argv, VALUE self)
{
  return rb_geoip_database_new(mGeoIP_Domain, argc, argv, self);
}

/* Pass this function an IP address as a string, it will return a hash
 * containing all the information that the database knows about the IP:
 *    db.look_up('24.24.24.24')
 *    => {:domain => "rr.com"}
 */
VALUE rb_geoip_domain_look_up(VALUE self, VALUE addr) {
  GeoIP *gi;
  Check_Type(addr, T_STRING);
  Data_Get_Struct(self, GeoIP, gi);
  return generic_single_value_lookup_response("domain", GeoIP_name_by_addr(gi, StringValuePtr(addr)));
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
  return UINT2NUM((unsigned int)GeoIP_addr_to_num(StringValuePtr(addr)));
}


VALUE rb_geoip_num_to_addr(VALUE self, VALUE num) {
  VALUE num_type = TYPE(num);
  switch(num_type) {
    case T_FIXNUM: break;
    case T_BIGNUM: break;
    default: rb_raise(rb_eTypeError, "wrong argument type %s (expected Fixnum or Bignum)", rb_obj_classname(num));
  }
  return rb_str_new2((char*)GeoIP_num_to_addr((unsigned long)NUM2ULONG(num)));
}

void Init_geoip()
{
  mGeoIP = rb_define_module("GeoIP");

  rb_geoip_memory = ID2SYM(rb_intern("memory"));
  rb_geoip_filesystem = ID2SYM(rb_intern("filesystem"));
  rb_geoip_index = ID2SYM(rb_intern("index"));

  mGeoIP_City = rb_define_class_under(mGeoIP, "City", rb_cObject);
  rb_define_singleton_method(mGeoIP_City, "new",      rb_geoip_city_new,      -1);
  rb_define_method(          mGeoIP_City, "look_up",  rb_geoip_city_look_up,  1);

  mGeoIP_Country = rb_define_class_under(mGeoIP, "Country", rb_cObject);
  rb_define_singleton_method(mGeoIP_Country, "new",      rb_geoip_country_new,      -1);
  rb_define_method(          mGeoIP_Country, "look_up",  rb_geoip_country_look_up,  1);

  mGeoIP_Organization = rb_define_class_under(mGeoIP, "Organization", rb_cObject);
  rb_define_singleton_method(mGeoIP_Organization, "new",     rb_geoip_org_new,     -1);
  rb_define_method(          mGeoIP_Organization, "look_up", rb_geoip_org_look_up, 1);

  mGeoIP_ISP = rb_define_class_under(mGeoIP, "ISP", rb_cObject);
  rb_define_singleton_method(mGeoIP_ISP, "new",     rb_geoip_isp_new,     -1);
  rb_define_method(          mGeoIP_ISP, "look_up", rb_geoip_isp_look_up, 1);

  mGeoIP_NetSpeed = rb_define_class_under(mGeoIP, "NetSpeed", rb_cObject);
  rb_define_singleton_method(mGeoIP_NetSpeed, "new",     rb_geoip_netspeed_new,     -1);
  rb_define_method(          mGeoIP_NetSpeed, "look_up", rb_geoip_netspeed_look_up, 1);

  mGeoIP_Domain = rb_define_class_under(mGeoIP, "Domain", rb_cObject);
  rb_define_singleton_method(mGeoIP_Domain, "new",     rb_geoip_domain_new,     -1);
  rb_define_method(          mGeoIP_Domain, "look_up", rb_geoip_domain_look_up, 1);

  rb_define_singleton_method(mGeoIP, "addr_to_num", rb_geoip_addr_to_num, 1);
  rb_define_singleton_method(mGeoIP, "num_to_addr", rb_geoip_num_to_addr, 1);
}
