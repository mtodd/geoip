Gem::Specification.new do |s|
  s.author            = 'Matt Todd'
  s.email             = 'mtodd@highgroove.com'
  
  s.name    = 'geoip'
  s.version = "0.5.0"
  
  s.summary     = "A Binding to the GeoIP C library"
  s.description = 'Generic GeoIP lookup tool. Based on the geoip_city RubyGem by Ryan Dahl'
  
  s.authors  = ['Ryan Dahl', 'Matt Todd', 'Charles Brian Quinn']
  s.email    = 'mtodd@highgroove.com'
  s.homepage          = "http://github.com/mtodd/geoip"
  
  s.files             = ["Rakefile", "extconf.rb", "test.rb", "geoip.c", "README.md"]
  s.test_files        = 'test.rb'
  s.extensions        = 'extconf.rb'
  s.require_path      = '.'
end
