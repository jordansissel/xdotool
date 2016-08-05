#!/usr/bin/env ruby
#

require "minitest"
require "./xdo_test_helper"

class XdotoolChainingTests < MiniTest::Test
  include XdoTestHelper

  def setup
    setup_vars
    setup_ensure_x_is_healthy
    setup_launch_xterm

    @cmds = ["mousedown 1", "mouseup 1", "mousemove 0 0", "mousemove 50 50",
             "click 1", "key \"ctrl+w\"", "mousemove_relative 30 30",
             "windowfocus #{@wid}", "windowmap #{@wid}",
             "windowmove #{@wid} 1 1", "windowraise #{@wid}",
             "windowunmap #{@wid}", "windowsize #{@wid} 500 500" ]
    @cmds_withoutput = ["getmouselocation"]
    @cmds_withoutput << "getwindowfocus -f"

    if (wm_supports?("_NET_ACTIVE_WINDOW"))
      @cmds << "windowactivate #{@wid}"
      @cmds_withoutput << "getwindowfocus"
    else
      puts "Skipping _NET_ACTIVE_WINDOW features (current wm does not support it)"
    end

    if (wm_supports?("_NET_NUMBER_OF_DESKTOPS"))
      @cmds << "set_num_desktops 5"
      @cmds_withoutput << "get_num_desktops"
    else
      puts "Skipping _NET_NUMBER_OF_DESKTOPS features (current wm does not support it)"
    end

    if (wm_supports?("_NET_WM_DESKTOP"))
      @cmds << "set_desktop_for_window #{@wid} 0"
      @cmds_withoutput << "get_desktop_for_window #{@wid}"
    else
      puts "Skipping _NET_WM_DESKTOP features (current wm does not support it)"
    end

    if (wm_supports?("_NET_CURRENT_DESKTOP"))
      @cmds << "set_desktop 0"
      @cmds_withoutput << "get_desktop"
    else
      puts "Skipping _NET_CURRENT_DESKTOP features (current wm does not support it)"
    end
  end

  def test_output_expectations
    @cmds.each do |cmd|
      xdotool "windowmap --sync #{@wid}"
      status, lines = xdotool cmd
      assert_status_ok(status, cmd)
      assert_equal(0, lines.length, "'#{cmd}' should have no output")
    end

    @cmds_withoutput.each do |cmd|
      xdotool "windowmap --sync #{@wid}"
      status, lines = xdotool cmd
      assert_status_ok(status, cmd)
      assert_equal(1, lines.length, "'#{cmd}' should have one line of output")
    end
  end # def test_output_expectations

  def test_chaining_does_not_fail
    @cmds << "search ."
    [@cmds, @cmds_withoutput].flatten.each do |cmd|
      # type command consumes all args. Skip it.
      next if cmd =~ /^type/

      # Sometimes, the window need to be mapped for a command to work.
      # For example, get_desktop_for_window fails on many window managers
      # when the window is unmapped (it has no desktop at that point)
      xdotool "windowmap --sync #{@wid}"
      xdotool "windowmap --sync #{@wid}"
      xdotool_ok "#{cmd} #{cmd}"
    end
  end # def test_multicommands_do_not_fail
end # end 

