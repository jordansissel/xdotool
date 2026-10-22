
#!/usr/bin/env ruby
#

require "minitest"
require "./xdo_test_helper"

class XdotoolCommandWindowSizeTests < Minitest::Test
  include XdoTestHelper

  def test_succeeds_with_valid_window
    xdotool_ok "windowsize #{@wid} 20 20"
  end # def test_succeeds_with_valid_window

  def test_expected_failures
    xdotool_fail "windowsize"
    xdotool_fail "windowsize %1"
    xdotool_fail "windowsize %@"
    xdotool_fail "windowsize 1"
    xdotool_fail "windowsize 1 1"
  end # def test_fails_without_a_window

  def test_percent_fractional
    status, lines = xdotool_ok "getdisplaygeometry"
    screen_w, screen_h = lines.first.split.map(&:to_i)

    xdotool_ok "windowsize #{@wid} 50.5% 50.5%"

    status, lines = xdotool_ok "getwindowgeometry --shell #{@wid}"
    geometry = {}
    lines.each do |line|
      key, value = line.split("=", 2)
      geometry[key] = value.to_i
    end

    assert_equal((screen_w * 50.5 / 100).round, geometry["WIDTH"])
    assert_equal((screen_h * 50.5 / 100).round, geometry["HEIGHT"])
  end # def test_percent_fractional

  def test_chaining
    #if detect_window_manager == :none
      #print "Skipping windowfocus tests. No WM present."
      #return
    #end
    xdotool_ok "windowfocus --sync #{@wid}"
    xdotool_ok "getwindowfocus -f windowsize 20 20"
    xdotool_ok "getwindowfocus -f windowsize %1 20 20"
    xdotool_ok "getwindowfocus -f windowsize %@ 20 20"
  end # def test_chaining
end # class XdotoolCommandWindowsizeTests

