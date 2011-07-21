#
# Tcl code which creates the UI portion of the keyboard
#
.menu.devices.m add command -label "Keyboard ..." -state normal -command kbdcfg

proc kbdcfg {} {
	toplevel .dialog
	wm title .dialog "Keyboard Configuration"
	frame .dialog.entry -relief sunken -bd 1
	label .dialog.entry.text -text "Polling Delay:"
	entry .dialog.entry.delay
	.dialog.entry.delay insert 0 [key_delay]
	bind .dialog.entry.delay <Return> { kbdok }
	pack .dialog.entry 
	pack .dialog.entry.text .dialog.entry.delay -side left
	.dialog.entry.delay select to end
	frame .dialog.close
	pack .dialog.close -side bottom
	frame .dialog.close.default -relief sunken -bd 1
	pack .dialog.close.default -side left -expand 1 -padx 3m -pady 2m
	button .dialog.close.ok -text OK -command kbdok
	pack .dialog.close.ok -in .dialog.close.default -side left 
	button .dialog.close.cancel -text Cancel -command kbdclose
	pack .dialog.close.cancel -side left
	focus .dialog.entry.delay
}

proc kbdok {} {
	key_delay [.dialog.entry.delay get]
	kbdclose
}

proc kbdclose {} {
	destroy .dialog
}

global hosttype
if { $hosttype == "i486" } { 
  key_delay 25 
} elseif { $hosttype == "i586" } {
  key_delay 50
} elseif { $hosttype == "i686" } { 
  key_delay 275 
}
# Currently the keyboard is just an invisible window possessing the input 
# focus but this approach allows, for example, a visible keyboard without
# requiring modification to the keyboard module.
#
label .kbd
bind .kbd <KeyPress> { key_down %N }
bind .kbd <KeyRelease> { key_up %N }
pack .kbd
focus .kbd
