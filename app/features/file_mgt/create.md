# Create

* `c[sabie]/?$`: Create new file or folder inside/next/above/below current file or folder:
  * `s`: Start
  * `a`: Above
  * `b`, Return: Below
  * `i`: In
  * `e`: End
  * `/$`: Create folder iso file
    * When `/` is found in the middle of the name, it should be escaped
  * When creating in a file, replace `file.ext` with `file/index.ext`
  * Creating next to a file/directory creates in the parent
* Support for using names containing '/'
  * Will escaping work?
* Use last extension by default
  * Save this is `home_dir`
