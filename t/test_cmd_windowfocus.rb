
#!/usr/bin/env ruby
#

require "minitest"
require "./xdo_test_helper"

class XdotoolCommandWindowFocusTests < MiniTest::Test
  include XdoTestHelper

  def test_succeeds_with_valid_window
    if detect_window_manager == :none
      print "Skipping windowfocus tests. No WM present."
      return
    end
    xdotool_ok "windowfocus #{@wid}"
  end # def test_succeeds_with_valid_window

  def test_expected_failures
    xdotool_fail "windowfocus"
    xdotool_fail "windowfocus %1"
    xdotool_fail "windowfocus %@"
    xdotool_fail "windowfocus 2" # test invalid window
  end # def test_fails_without_a_window

  def test_chaining
    xdotool_ok "windowfocus --sync #{@wid}"
    xdotool_ok "getwindowfocus -f windowfocus "
    xdotool_ok "getwindowfocus -f windowfocus %1"
    xdotool_ok "getwindowfocus -f windowfocus %@"
  end # def test_chaining
end # class XdotoolCommandWindowFocusTests

