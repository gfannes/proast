require("fileutils")

task :default do
    sh "rake -T"
end

file ".extern" do
    puts "creating dir .extern"
    FileUtils.mkdir(".extern")
end
file ".extern/termbox/ok" => ".extern" do
    Dir.chdir(".extern") do
        FileUtils.rm_rf("termbox")
        sh "git clone https://github.com/nsf/termbox"
        Dir.chdir("termbox") do
            sh "touch ok"
        end
    end
end

task :dependencies => ".extern/termbox/ok"

task :version_git_hash_header do
    File.open("app/src/proast/version_git_hash.hpp", "w") do |fo|
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
    mode = :debug
    # mode = :release
    sh "cook -g ninja -T c++.std=17 -T #{mode} proast/app"
    sh "ninja -v"
end

desc "Install proast"
task :install => :build do
    sh "sudo cp proast.app /usr/local/bin/proast"
end

desc "Run unit tests, filter is colon-separated selection filter"
task :ut, [:filter] do |t,args|
    filter = (args[:filter] || "ut").split(":").map{|e|"[#{e}]"}*""

    mode = :debug
    # mode = :release
    sh "cook -g ninja -T c++.std=17 -T #{mode} proast/ut"
    sh "ninja -v"

    cmd = %w[./proast.ut -d yes -a] << filter
    sh(*cmd)
end

desc "Run proast"
task :test => :build do
    sh "./proast.app -r ./ -r #{ENV["HOME"]}/nontech -r #{ENV["gubg"]}"
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
end
