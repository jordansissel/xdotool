
#!/usr/bin/env ruby
#

require "minitest"
require "./xdo_test_helper"

class XdotoolCommandWindowMoveTests < MiniTest::Test
  include XdoTestHelper

  def test_succeeds_with_valid_window
    xdotool_ok "windowmove #{@wid} 20 20"
  end # def test_succeeds_with_valid_window

  def test_expected_failures
    xdotool_fail "windowmove"
    xdotool_fail "windowmove %1"
    xdotool_fail "windowmove %@"
    xdotool_fail "windowmove 1"
    xdotool_fail "windowmove 1 1 1" # test invalid window
  end # def test_fails_without_a_window

  def test_chaining
    xdotool_ok "windowfocus --sync #{@wid}"
    xdotool_ok "getwindowfocus -f windowmove 20 20"
    xdotool_ok "getwindowfocus -f windowmove %1 20 20"
    xdotool_ok "getwindowfocus -f windowmove %@ 20 20"
  end # def test_chaining
end # class XdotoolCommandWindowMoveTests

