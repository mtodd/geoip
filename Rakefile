require 'rake'
require 'bundler/gem_tasks'
require 'rake/clean'
require 'rake/testtask'
require 'rdoc/task'

task :default => [:compile, :test]

CLEAN.add "geoip.{o,bundle,so,obj,pdb,lib,def,exp}"
CLOBBER.add ['Makefile', 'mkmf.log','doc']

Rake::RDocTask.new do |rdoc|
  rdoc.rdoc_files.add ['README.md', 'geoip.c']
  rdoc.main = "README.md" # page to start on
  rdoc.rdoc_dir = 'doc/' # rdoc output folder
end

Rake::TestTask.new do |t|
  t.test_files = ['test.rb']
  t.verbose = true
end

desc 'compile the extension'
task(:compile => 'Makefile') { sh 'make' }
file('Makefile' => "geoip.c") { ruby 'extconf.rb' }
