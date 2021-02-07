UI
==

* Support for colored output with `ui::List`

## ui::List

* Show name in border
* Support for colors

## Windows

* `w[abcmd]`: Switches to a different window
  * `a`: parent
  * `b`: self
  * `c`: child
  * `m`: metadata
  * `d`: details

## Navigation

* Store/load child navigation in `config` folder
* Store/load `current_node_` in `config` folder

## Jump list

* Keep track of jump locations after `jump to bookmark`
* Register jump location on `m\n`
* Use `-` to jump to last jump location, adding current to list
* Use `ctrl-o` to jump down to jump location list
* Show jump location list in BottomLeft

