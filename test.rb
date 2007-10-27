require 'test/unit'
require 'geoip_city'

class GeoIPTest < Test::Unit::TestCase
  
  def setup
    ## Change me!
    @dbfile = '/opt/GeoIP-1.4.2/share/GeoIP/GeoLiteCity.dat'
  end
  
  
  def test_construction
    db = GeoIPCity::Database.new(@dbfile)
    
    assert_raises TypeError do 
      db.look_up(nil) 
    end
    
    h = db.look_up('24.24.24.24')
    assert_kind_of Hash, h
    assert_equal 'Ithaca', h[:city]
    assert_equal 'United States', h[:country_name]
  end
  
end
