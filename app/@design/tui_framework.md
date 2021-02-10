Text-base UI framework
======================

## Candidates

### [notcurses](https://nick-black.com/dankwiki/index.php/Notcurses)

Install via `yay notcurses`.
Clone frome [github](https://github.com/dankamongmen/notcurses) and building works.

* Pro
  * Seems the most advanced
  * Has [nice documentation](https://nick-black.com/htp-notcurses.pdf)
* Con
  * No windows support
  * First line drops sometimes

### [ncurses](https://invisible-island.net/ncurses/announce.html)

### [final cut](https://github.com/gansm/finalcut)

Install via `yay final cut` and link with `-lfinal`

* Con
  * Seems to add windows border around everything

### [termbox](https://github.com/nsf/termbox)

* Pro
  * Simple API

### [ftxui](https://github.com/ArthurSonzogni/FTXUI)

* Pro
  * Supports utf8
  * Supports widgets
  * Code is readable, developer is responsive
  * No dependencies, modern C++
* Con
  * Does not emit Ctrl
