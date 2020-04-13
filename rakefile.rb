require("fileutils")

task :default do
    sh "rake -T"
end

desc "Build proast"
task :build do
    sh "cook -g ninja -T c++.std=17 proast"
    sh "ninja -v"
end

desc "Run proast"
task :test => :build do
    sh "./proast"
end

desc "Clear cached binaries"
task :clear do
    FileUtils.rm_rf(".cook")
    FileUtils.rm_f("proast")
    FileUtils.rm_f("build.ninja")
end
