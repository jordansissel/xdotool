#!/usr/bin/env ruby
#

require "minitest"
require "./xdo_test_helper"

class XdotoolCommandWindowMinimizeTests < MiniTest::Test
  include XdoTestHelper

  def test_succeeds_with_valid_window
    if wm_supports?("_NET_WM_STATE") 
      puts "Skipping minimize tests (current wm does not support it _NET_WM_STATE)"
      return
    end
    xdotool_ok "windowminimize #{@wid}"
  end # def test_succeeds_with_valid_window

  def test_expected_failures
    if wm_supports?("_NET_WM_STATE") 
      puts "Skipping minimize tests (current wm does not support it _NET_WM_STATE)"
      return
    end
    xdotool_fail "windowminimize"
    xdotool_fail "windowminimize %1"
    xdotool_fail "windowminimize %@"
    xdotool_fail "windowminimize 2" # test invalid window
  end # def test_fails_without_a_window

  def test_chaining
    if wm_supports?("_NET_WM_STATE") 
      puts "Skipping minimize tests (current wm does not support it _NET_WM_STATE)"
      return
    end
    xdotool_ok "windowfocus --sync #{@wid}"
    xdotool_ok "getwindowfocus -f windowminimize "
    xdotool_ok "getwindowfocus -f windowminimize %1"
    xdotool_ok "getwindowfocus -f windowminimize %@"
    xdotool_ok "getwindowfocus -f windowminimize --sync %@"
  end # def test_chaining
end # class XdotoolCommandWindowMinimizeTests
