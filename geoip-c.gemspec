Gem::Specification.new do |s|
  s.name         = 'geoip-c'
  s.version      = "0.9.0"
  s.license      = 'WTFPL'

  s.authors      = ['Ryan Dahl', 'Matt Todd', 'Charles Brian Quinn', 'Michael Sheakoski', 'Silvio Quadri', 'Andy Lindeman']
  s.email        = ['andy@andylindeman.com', 'mtodd@highgroove.com']

  s.summary      = "A Binding to the GeoIP C library"
  s.description  = 'Generic GeoIP lookup tool. Based on the geoip_city RubyGem by Ryan Dahl'
  s.homepage     = "http://github.com/mtodd/geoip"

  s.files        = `git ls-files`.split("\n")
  s.test_files   = ['test.rb']
  s.extensions   = ['extconf.rb']
  s.require_path = '.'

  s.add_development_dependency 'minitest', '~>5.0'
  s.add_development_dependency 'rake', '~>10.0'
end
