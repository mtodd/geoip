require 'rake'
require 'bundler/gem_tasks'
require 'rake/clean'
require 'rake/testtask'
require 'rdoc/task'
require "rake/extensiontask"

task :default => [:test]

CLOBBER.add 'doc', 'data'

RDoc::Task.new do |rdoc|
  rdoc.main = "README.md" # page to start on
  rdoc.rdoc_files.add ["README.md", "ext/geoip/geoip.c"]
  rdoc.rdoc_dir = 'doc' # rdoc output folder
end

Rake::TestTask.new do |t|
  t.test_files = ['test.rb']
  t.verbose = true
end

spec = Gem::Specification.load "geoip-c.gemspec"

Rake::ExtensionTask.new("geoip", spec) do |ext|
end

directory "data"

file "data/GeoLiteCity.dat" => ["data"] do |f|
  url = "http://geolite.maxmind.com/download/geoip/database/GeoLiteCity.dat.gz"

  sh "curl #{url} -o data/GeoLiteCity.dat.gz"
  sh "gunzip data/GeoLiteCity.dat.gz"
  touch f.name
end

task :database => ["data/GeoLiteCity.dat"] do
  ENV['CITY'] = File.expand_path("data/GeoLiteCity.dat")
end

task :test => [:compile, :database]
