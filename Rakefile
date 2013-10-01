require 'rake'
require 'bundler/gem_tasks'
require 'rake/clean'
require 'rake/testtask'
require 'rdoc/task'
require "rake/extensiontask"

task :default => [:compile, :test]

CLOBBER.add 'doc'

RDoc::Task.new do |rdoc|
  rdoc.main = "README.md" # page to start on
  rdoc.rdoc_files.add ["README.md", "ext/geoip/geoip.c"]
  rdoc.rdoc_dir = 'doc/' # rdoc output folder
end

Rake::TestTask.new do |t|
  t.test_files = ['test.rb']
  t.verbose = true
end

spec = Gem::Specification.load "geoip-c.gemspec"

Rake::ExtensionTask.new("geoip", spec) do |ext|
end
