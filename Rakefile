# -*- ruby -*-

java    = RUBY_PLATFORM =~ /java/

require 'rubygems'
require 'hoe'
unless java
  gem 'rake-compiler', '>= 0.4.1'
  require "rake/extensiontask"
end

HOE = Hoe.spec 'phuby' do
  developer('Aaron Patterson', 'aaronp@rubyforge.org')
  self.readme_file   = 'README.rdoc'
  self.history_file  = 'CHANGELOG.rdoc'
  self.extra_rdoc_files  = FileList['*.rdoc']

  self.spec_extras = { :extensions => ["ext/phuby/extconf.rb"] }
end

unless java
  RET = Rake::ExtensionTask.new("phuby", HOE.spec) do |ext|
    ext.lib_dir = File.join('lib', 'phuby')
  end
end

Rake::Task[:test].prerequisites << :compile

namespace :java do

  JRUBY_HOME = Config::CONFIG['prefix']
  JAR_PATH = File.join('lib','phuby','phuby.jar')
  EXTERNAL_JAVA_LIBRARIES = %w{quercus resin-util}.map {|x| File.join('lib', x+".jar")}

  task :spec => ["java:build_external"] do
    File.open("#{HOE.name}.gemspec", "w") do |f|
      HOE.spec.platform = 'java'
      HOE.spec.extensions = []
      HOE.spec.files += [JAR_PATH] + EXTERNAL_JAVA_LIBRARIES
      f.write(HOE.spec.to_ruby)
    end
  end

  task :clean_jar do
    File.delete JAR_PATH if File.exists? JAR_PATH
  end

  task :clean_classes do
    FileList['ext/java/phuby/*.class'].each do |f|
      File.delete f
    end
  end

  task :clean_all => ['java:clean_jar', 'java:clean_classes']

  desc "Build a gem targetted for JRuby"
  task :gem => ["java:spec"] do
    system "gem build phuby.gemspec"
    FileUtils.mkdir_p "pkg"
    FileUtils.mv Dir.glob("phuby*-java.gem"), "pkg"
  end

  task :build_external do
    Dir.chdir('ext/java') do
      LIB_DIR = '../../lib'
      JRUBY_JAR = "#{JRUBY_HOME}/lib/jruby.jar"
      CLASSPATH = "#{JRUBY_JAR}:#{EXTERNAL_JAVA_LIBRARIES.join(':')}"
      sh "javac -cp #{CLASSPATH} phuby/*.java"
      sh "jar cf #{File.join('..', '..', JAR_PATH)} phuby/*.class"
    end
  end

  task :build => ["java:clean_jar", "java:build_external", "java:clean_classes"]
end
# vim: syntax=ruby
