; polymode for generator.lua
; unfortunately polymode inside polymode doesn't work, and I have no idea what
; am I doing. patches are welcome.

(require 'polymode)

(defcustom pm-inner/lua-inline
  (pm-hbtchunkmode "lua-inline"
                   :mode 'lua-mode
                   :head-reg "/\\*\\$=?"
                   :tail-reg "\\*/"
                                        ;:font-lock-narrow t
                   )
  "Lua code chunk /*$ */"
  :group 'innermodes
  :type 'object)

(defcustom pm-inner/lua-line
  (pm-hbtchunkmode "lua-line"
                   :mode 'lua-mode
                   :head-reg "//\\$=?"
                   :tail-reg "\n"
                                        ;:font-lock-narrow t
                   )
  "Lua code chunk /*$ */"
  :group 'innermodes
  :type 'object)

(defcustom pm-poly/c++-lua
  (pm-polymode-multi "c++-lua"
                     :hostmode 'pm-host/C++
                     :innermodes '(pm-inner/lua-inline pm-inner/lua-line))
  "C++ + Lua"
  :group 'polymodes
  :type 'object)

(define-polymode poly-c++-lua-mode pm-poly/c++-lua)

(defcustom pm-host/lua
  (pm-bchunkmode "lua" :mode 'lua-mode)
  "Lua host chunkmode"
  :group 'hostmodes
  :type 'object)

(defcustom pm-inner/c++-inner
  (pm-hbtchunkmode "c++-inner"
                   :mode 'poly-c++-lua-mode
                   :head-reg "\\[=*\\["
                   :tail-reg "\\]=*\\]")
  "Lua + C++ + Lua"
  :group 'innermodes
  :type 'object)

(defcustom pm-poly/lua-c++-lua
  (pm-polymode-one "lua-c++-lua"
                   :hostmode 'pm-host/lua
                   :innermode 'pm-inner/c++-inner)
  "Lua + C++ + Lua"
  :group 'polymodes
  :type 'object)

(define-polymode poly-lua-c++-lua-mode pm-poly/lua-c++-lua)
