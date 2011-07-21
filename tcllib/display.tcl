proc display_init {w h bw bh d} {

  # create the canvas representing the display surface
  #
  set ww [expr $w*$bw+2]
  set hh [expr $h*$bh+2]
  canvas .screen.display -relief flat -width $ww -height $hh -background black
  pack .screen.display

  # create bitmaps for the character-set
  #
  global chars
  for {set i 0} {$i<256} {incr i} {
    set chars($i) \
      [image create bitmap -background black -foreground white -file $d/$i.bmp]
  }

  # create one cell for each screen position
  #
  global cells
  set k 0
  set n 0
  for {set j 2} {$j < $hh} {set j [expr $j+$bh]} {
    for {set i 2} {$i < $ww} {set i [expr $i+$bw]} {
      if {$k == 256} { set k 0 }
      set cells($n) \
        [.screen.display create image $i $j -anchor nw -image $chars($k)]
      set k [expr $k+1]
      set n [expr $n+1]
    }
  }
}

proc display_fini {} {
  destroy .screen.display
  destroy .screen
  for {set i 0} {$i<256} {incr i} {
    [image delete bitmap $chars($i)]
  }
}

proc display_at {c ch} {
  global chars
  global cells
  .screen.display itemconfigure $cells($c) -image $chars($ch)
}

toplevel .screen
wm title .screen Screen
