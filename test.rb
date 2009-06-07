require 'test/unit'
require File.dirname(__FILE__) + '/geoip'
require 'rubygems'
# require 'ruby-debug'
# Debugger.start

class Test::Unit::TestCase

  def assert_look_up(db, addr, field, value)
    h = db.look_up(addr)
    assert_equal value, h[field]
    h
  end

end

class GeoIPCityTest < Test::Unit::TestCase
  
  def setup
    ## Change me!
    @dbfile = '/opt/GeoIP/share/GeoIP/GeoLiteCity.dat'
  end

  def test_construction_default
    db = GeoIP::City.new(@dbfile)
    
    assert_raises TypeError do 
      db.look_up(nil) 
    end
    
    h = db.look_up('24.24.24.24')
    #debugger
    assert_kind_of Hash, h
    assert_equal 'Jamaica', h[:city]
    assert_equal 'United States', h[:country_name]
  end

  def test_construction_index
    db = GeoIP::City.new(@dbfile, :index)
    assert_look_up(db, '24.24.24.24', :city, 'Jamaica')
  end

  def test_construction_filesystem
    db = GeoIP::City.new(@dbfile, :filesystem)
    assert_look_up(db, '24.24.24.24', :city, 'Jamaica')
  end

  def test_construction_memory
    db = GeoIP::City.new(@dbfile, :memory)
    assert_look_up(db, '24.24.24.24', :city, 'Jamaica')
  end

  def test_construction_filesystem_check
    db = GeoIP::City.new(@dbfile, :filesystem, true)
    assert_look_up(db, '24.24.24.24', :city, 'Jamaica')
  end

  def test_bad_db_file
    assert_raises Errno::ENOENT do
      GeoIP::City.new('/blah')
    end
  end

end

class GeoIPOrgTest < Test::Unit::TestCase
  
  def setup
    ## Change me!
    @dbfile = '/opt/GeoIP/share/GeoIP/GeoIPOrg.dat'
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
      GeoIP::Organization.new('/blah')
    end
  end

end
