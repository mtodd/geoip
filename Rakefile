require 'rake'
require 'rake/clean'
require 'rake/testtask'
require 'rake/rdoctask'
require 'rake/gempackagetask'

task :default => [:compile, :test]

CLEAN.add "geoip_city.{o,bundle,so,obj,pdb,lib,def,exp}"
CLOBBER.add ['Makefile', 'mkmf.log','doc']

Rake::RDocTask.new do |rdoc|
  rdoc.rdoc_files.add ['README', 'geoip_city.c']
  rdoc.main = "README" # page to start on
  rdoc.rdoc_dir = 'doc/' # rdoc output folder
end

Rake::TestTask.new do |t|
  t.test_files = 'test.rb'
  t.verbose = true
end

spec = Gem::Specification.new do |s|
  s.name              = 'geoip_city'
  s.author            = 'ry dahl'
  s.email             = 'ry@tinyclouds.org'
  s.version           = "0.2.0"
  s.summary           = "A Binding to the GeoIP C library"
  s.homepage          = "http://geoip_city.rubyforge.org"
  s.files             = FileList['Rakefile', '*.rb', '*.c', 'README*']
  s.test_files        = 'test.rb'
  s.extensions        = 'extconf.rb'
  s.require_path      = '.'
end

Rake::GemPackageTask.new(spec) do |p|
  p.need_tar = true
  p.gem_spec = spec
end

desc 'compile the extension'
task(:compile => 'Makefile') { sh 'make' }
file('Makefile' => "geoip_city.c") { ruby 'extconf.rb' }

task(:webpage) do
  sh 'scp -r doc/* rydahl@rubyforge.org:/var/www/gforge-projects/geoip-city/'
end
