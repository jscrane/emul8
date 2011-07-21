#
# Tcl code which adds a configuration menu to the machine harness
#
.menu.devices.m add command -label "Tape ..." -state normal -command tapecfg

proc tapecfg {} {
  toplevel .dialog
  wm title .dialog "Tape I/O"
  frame .dialog.entry -relief sunken -bd 1
  label .dialog.entry.text -text "File Name:"
  entry .dialog.entry.file
  .dialog.entry.file insert 0 [tape filename]
  bind .dialog.entry.file <Return> { tape_ok }
  pack .dialog.entry 
  pack .dialog.entry.text .dialog.entry.file -side left
  .dialog.entry.file select to end
  frame .dialog.buttons
  pack .dialog.buttons -side bottom
  frame .dialog.buttons.default -relief sunken -bd 1
  pack .dialog.buttons.default -side left -expand 1 -padx 3m -pady 2m
  button .dialog.buttons.ok -text OK -command tape_ok
  pack .dialog.buttons.ok -in .dialog.buttons.default -side left 
  button .dialog.buttons.rewind -text Rewind -state [tape rewind] -command tape_rewind
  pack .dialog.buttons.rewind -side left
  button .dialog.buttons.cancel -text Cancel -command tape_close
  pack .dialog.buttons.cancel -side left
  focus .dialog.entry.file
}

proc tape_ok {} {
  set f [.dialog.entry.file get]
  tape filename $f
  tape_close
}

proc tape_rewind {} {
  tape rewind
  tape_close
}

proc tape_close {} {
  destroy .dialog
}
