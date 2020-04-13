task :build do
    sh "cook -g ninja -T c++.std=17 proast"
    sh "ninja -v"
end

task :test => :build do
    sh "./proast"
end
