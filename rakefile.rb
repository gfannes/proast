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

task :version_git_hash_header do
    File.open("tui_app/src/proast/version_git_hash.hpp", "w") do |fo|
        git_hash = `git log -n1 --abbrev=8 --abbrev-commit`.split[1]
        case git_hash
        when NilClass
            puts("Warning: this is not a git repo")
        else
            fo.puts("#define PROAST_GIT_HASH \"#{git_hash}\"")
        end
    end
end

desc "Build proast"
task :build => [:dependencies, :version_git_hash_header] do
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
