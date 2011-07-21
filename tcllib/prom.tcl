.menu.devices.m add command -label "PROM ..." -state normal -command promcfg

proc promcfg {} {
  toplevel .dialog
  wm title .dialog "Create ROM image"
  frame .dialog.entry -relief sunken -bd 1
  frame .dialog.entry.top
  frame .dialog.entry.bottom
  pack .dialog.entry.top .dialog.entry.bottom -side top
  label .dialog.entry.top.lbl -text "File Name:"
  entry .dialog.entry.top.file
  .dialog.entry.top.file insert 0 [prom]
  bind .dialog.entry.top.file <Return> { prom_ok }
  pack .dialog.entry 
  pack .dialog.entry.top.lbl .dialog.entry.top.file -side left
  label .dialog.entry.bottom.from_l -text "Page Range:"
  entry .dialog.entry.bottom.from -width 2
  label .dialog.entry.bottom.to_l -text ","
  entry .dialog.entry.bottom.to -width 2
  # disable all printing characters for the entries
  #
  for {set i 32} {$i < 127} {incr i} {
    if {$i==60} {
      set ch <less>
    } elseif {$i==32} {
      set ch <space>
    } else {
      set ch [format %c $i]
    }
    bind .dialog.entry.bottom.from $ch { break }
    bind .dialog.entry.bottom.to $ch { break }
  }
  # re-enable only 0-9a-fA-F for the entries
  #
  set hex "0123456789abcdefABCDEF"
  set len [string length $hex]
  for {set i 0} {$i < $len} {incr i} {
    set ch [string index $hex $i]
    bind .dialog.entry.bottom.from $ch { 
      enter_alpha %A %W .dialog.entry.bottom.to
      if {2==[string length [%W get]]} {
        focus .dialog.entry.bottom.to
      }
      break
    }
    bind .dialog.entry.bottom.to $ch {
      enter_alpha %A %W .dialog.entry.bottom.from
      break
    }
  }
  pack .dialog.entry.bottom.from_l .dialog.entry.bottom.from \
    .dialog.entry.bottom.to_l .dialog.entry.bottom.to -side left
  .dialog.entry.top.file select to end
  frame .dialog.buttons
  pack .dialog.buttons -side bottom
  frame .dialog.buttons.default -relief sunken -bd 1
  pack .dialog.buttons.default -side left -expand 1 -padx 3m -pady 2m
  button .dialog.buttons.ok -text OK -command prom_ok -state disabled
  pack .dialog.buttons.ok -in .dialog.buttons.default -side left 
  button .dialog.buttons.cancel -text Cancel -command prom_close
  pack .dialog.buttons.cancel -side left
  focus .dialog.entry.top.file
}

proc enter_alpha {ch this other} {
  set len [string length [$this get]]
  if {$len < 2} {
    tkEntryInsert $this $ch
    incr len
  }
  set olen [string length [$other get]]
  if {$len==2 && $olen==2} {
    .dialog.buttons.ok configure -state normal
  } else {
    .dialog.buttons.ok configure -state disabled
  }
}

proc prom_ok {} {
  prom [.dialog.entry.top.file get] \
    [.dialog.entry.bottom.from get] [.dialog.entry.bottom.to get]
  prom_close
}

proc prom_close {} {
  destroy .dialog
}
