# Creating file or folder

* `c.*/?$`: Create new file or folder under current file or folder:
  * `/$`: Create folder iso file
    * When `/` is found in the middle of the name, it should be escaped
  * Creating next to a file/directory creates in the parent
* Support for using names containing `/`
  * Will escaping work?
* Use last extension by default
  * Save this is `home_dir`
