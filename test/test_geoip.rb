# encoding: utf-8

require 'rubygems'
gem 'minitest'
require 'minitest/autorun'
require 'geoip'

CITY_DB = ENV.fetch("CITY", '/usr/local/GeoIP/share/GeoIP/GeoLiteCity.dat')
ORG_DB  = ENV.fetch("ORG",  '/usr/local/GeoIP/share/GeoIP/GeoIPOrg.dat')

class Minitest::Test
  def assert_look_up(db, addr, field, value)
    h = db.look_up(addr)
    assert_equal value, h[field]
    h
  end
end

class GeoIPTest < Minitest::Test

  def setup
    @ip = "24.24.24.24"
    @ipnum = 16777216*24 + 65536*24 + 256*24 + 24

    @large_ip = "245.245.245.245"
    @large_ipnum = 16777216*245 + 65536*245 + 256*245 + 245
  end

  # addr_to_num

  def test_addr_to_num_converts_an_ip_to_an_ipnum
    assert_equal @ipnum, GeoIP.addr_to_num(@ip)
  end

  def test_addr_to_num_converts_large_ips_to_an_ipnum_correctly
    assert_equal @large_ipnum, GeoIP.addr_to_num(@large_ip)
  end

  def test_addr_to_num_expects_an_ip_string
    assert_raises TypeError do
      GeoIP.addr_to_num(nil)
    end
  end

  def test_addr_to_num_returns_zero_for_an_illformed_ip_string
    assert_equal 0, GeoIP.addr_to_num("foo.bar")
  end

  # num_to_addr

  def test_num_to_addr_converts_an_ipnum_to_an_ip
    assert_equal @ip, GeoIP.num_to_addr(@ipnum)
  end

  def test_num_to_addr_converts_large_ipnums_to_an_ip_correctly
    assert_equal @large_ip, GeoIP.num_to_addr(@large_ipnum)
  end

  def test_num_to_addr_expects_a_numeric_ip
    assert_raises TypeError do
      GeoIP.num_to_addr(nil)
    end
    assert_raises TypeError do
      GeoIP.num_to_addr("foo.bar")
    end
  end

end

class GeoIPCityTest < Minitest::Test

  def setup
    ## Change me!
    @dbfile = CITY_DB
  end

  def test_construction_default
    db = GeoIP::City.new(@dbfile)

    assert_raises TypeError do
      db.look_up(nil)
    end

    h = db.look_up('24.24.24.24')
    #debugger
    assert_kind_of Hash, h
    assert_equal 'Deer Park', h[:city]
    assert_equal 'United States', h[:country_name]
  end

  def test_construction_index
    db = GeoIP::City.new(@dbfile, :index)
    assert_look_up(db, '24.24.24.24', :city, 'Deer Park')
  end

  def test_construction_filesystem
    db = GeoIP::City.new(@dbfile, :filesystem)
    assert_look_up(db, '24.24.24.24', :city, 'Deer Park')
  end

  def test_construction_memory
    db = GeoIP::City.new(@dbfile, :memory)
    assert_look_up(db, '24.24.24.24', :city, 'Deer Park')
  end

  def test_construction_filesystem_check
    db = GeoIP::City.new(@dbfile, :filesystem, true)
    assert_look_up(db, '24.24.24.24', :city, 'Deer Park')
  end

  def test_bad_db_file
    assert_raises Errno::ENOENT do
      GeoIP::City.new('/supposed-to-fail')
    end
  end

  def test_character_encoding_converted_to_utf8_first
    db = GeoIP::City.new(@dbfile, :filesystem, true)
    assert_look_up(db, '201.85.50.148', :city, "Jundiaí")
  end

  def test_empty_region_name_does_not_crash
    db = GeoIP::City.new(@dbfile, :filesystem, true)
    assert_look_up(db, '119.236.232.169', :region, "00")
    assert_look_up(db, '119.236.232.169', :region_name, nil)
  end

end

class GeoIPOrgTest < Minitest::Test

  def setup
    ## Change me!
    @dbfile = ORG_DB
  end

  def test_construction_default
    db = GeoIP::Organization.new(@dbfile)

    assert_raises TypeError do
      db.look_up(nil)
    end

    h = db.look_up('24.24.24.24')
    assert_kind_of Hash, h
    assert_equal 'Road Runner', h[:name]
  end

  def test_construction_index
    db = GeoIP::Organization.new(@dbfile, :index)
    assert_look_up(db, '24.24.24.24', :name, 'Road Runner')
  end

  def test_construction_filesystem
    db = GeoIP::Organization.new(@dbfile, :filesystem)
    assert_look_up(db, '24.24.24.24', :name, 'Road Runner')
  end

  def test_construction_memory
    db = GeoIP::Organization.new(@dbfile, :memory)
    assert_look_up(db, '24.24.24.24', :name, 'Road Runner')
  end

  def test_construction_filesystem_check
    db = GeoIP::Organization.new(@dbfile, :filesystem, true)
    assert_look_up(db, '24.24.24.24', :name, 'Road Runner')
  end

  def test_bad_db_file
    assert_raises Errno::ENOENT do
      GeoIP::Organization.new('/supposed-to-fail')
    end
  end

end
