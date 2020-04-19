require("fileutils")

task :default do
    sh "rake -T"
end

file ".extern" do
    puts "creating dir .extern"
    FileUtils.mkdir(".extern")
end
file ".extern/notcurses/ok" => ".extern" do
    Dir.chdir(".extern") do
        FileUtils.rm_rf("notcurses")
        sh "git clone https://github.com/dankamongmen/notcurses"
        Dir.chdir("notcurses") do
            FileUtils.mkdir("build")
            Dir.chdir("build") do
                sh "cmake .."
                sh "make -j 4"
                sh "sudo make install"
            end
            sh "touch ok"
        end
    end
end

task :dependencies => ".extern/notcurses/ok"

desc "Build proast"
task :build => :dependencies do
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

desc "Remove everything"
task :proper => :clear do
    FileUtils.rm_rf(".extern")
    sh "sudo rm -rf .extern/notcurses"
end
