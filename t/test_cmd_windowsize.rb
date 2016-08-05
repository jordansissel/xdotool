
#!/usr/bin/env ruby
#

require "minitest"
require "./xdo_test_helper"

class XdotoolCommandWindowSizeTests < MiniTest::Test
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

