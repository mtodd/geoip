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
static VALUE mGeoIP_ISP;
static VALUE mGeoIP_Domain;
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
    return hash;
  } else {
    return Qnil;
  }
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
  VALUE hash = rb_hash_new();
  char * name = NULL;

  Check_Type(addr, T_STRING);
  Data_Get_Struct(self, GeoIP, gi);
  if(name = GeoIP_name_by_addr(gi, STR2CSTR(addr))) {
    rb_hash_sset(hash, "isp", rb_str_new2(name));
    free(name);
    return hash;
  } else {
    return Qnil;
  }
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
  VALUE hash = rb_hash_new();
  char * name = NULL;

  Check_Type(addr, T_STRING);
  Data_Get_Struct(self, GeoIP, gi);
  if(name = GeoIP_name_by_addr(gi, STR2CSTR(addr))) {
    rb_hash_sset(hash, "domain", rb_str_new2(name));
    free(name);
    return hash;
  } else {
    return Qnil;
  }
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
  return UINT2NUM((unsigned int)_GeoIP_addr_to_num(STR2CSTR(addr)));
}

/* This returns a pointer to a character array that represents the IP string of
 * the IP Number given it.
 * 
 * This was originally a slightly patched version of the function found in the
 * GeoIP library called _GeoIP_num_to_addr with the same declaration but
 * instead a slight variation has been used here.
 * 
 * The function definition this replaces follows:
 *   char *ret_str;
 *   char *cur_str;
 *   int octet[4];
 *   int num_chars_written, i;
 *   
 *   ret_str = malloc(sizeof(char) * 16);
 *   memset(ret_str, '\0', sizeof(char) * 16); // ensure we null-terminate the string
 *   cur_str = ret_str;
 *   
 *   for (i = 0; i<4; i++) {
 *     octet[3 - i] = ipnum % 256;
 *     ipnum >>= 8;
 *   }
 *   
 *   for (i = 0; i<4; i++) {
 *     num_chars_written = sprintf(cur_str, "%d", octet[i]);
 *     cur_str += num_chars_written;
 *     
 *     if (i < 3) {
 *       cur_str[0] = '.';
 *       cur_str++;
 *     }
 *   }
 *   
 *   return ret_str;
 * 
 * I make no assertions about speed or efficiency here. There is no error
 * handling (in case of malloc failing), also.
 * 
 * However, this works on 64bit platforms, where the previous implementation
 * did not.
 */
char *_patched_GeoIP_num_to_addr(GeoIP* gi, unsigned long ipnum) {
  char *ip;
  int i, octet[4];
  
  ip = malloc(sizeof(char) * 16);
  memset(ip, '\0', sizeof(char) * 16);
  
  for(i = 0; i < 4; i++)
  {
    octet[i] = (ipnum >> (i*8)) & 0xFF;
  }
  
  sprintf(ip, "%i.%i.%i.%i", octet[3], octet[2], octet[1], octet[0]);
  
  return ip;
}

VALUE rb_geoip_num_to_addr(VALUE self, VALUE num) {
  VALUE num_type = TYPE(num);
  switch(num_type) {
    case T_FIXNUM: break;
    case T_BIGNUM: break;
    default: rb_raise(rb_eTypeError, "wrong argument type %s (expected Fixnum or Bignum)", rb_obj_classname(num));
  }
  // return rb_str_new2((char*)_GeoIP_num_to_addr(NULL, (unsigned long)NUM2ULONG(num)));
  return rb_str_new2((char*)_patched_GeoIP_num_to_addr(NULL, (unsigned long)NUM2ULONG(num)));
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
  
  mGeoIP_ISP = rb_define_class_under(mGeoIP, "ISP", rb_cObject);
  rb_define_singleton_method( mGeoIP_ISP, "new",     rb_geoip_isp_new,     -1);
  rb_define_method(           mGeoIP_ISP, "look_up", rb_geoip_isp_look_up, 1);

  mGeoIP_Domain = rb_define_class_under(mGeoIP, "Domain", rb_cObject);
  rb_define_singleton_method( mGeoIP_Domain, "new",     rb_geoip_domain_new,     -1);
  rb_define_method(           mGeoIP_Domain, "look_up", rb_geoip_domain_look_up, 1);

  rb_define_singleton_method(mGeoIP, "addr_to_num", rb_geoip_addr_to_num, 1);
  rb_define_singleton_method(mGeoIP, "num_to_addr", rb_geoip_num_to_addr, 1);
}
