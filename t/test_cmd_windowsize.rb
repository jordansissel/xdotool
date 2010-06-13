
#!/usr/bin/env ruby
#

require "test/unit"
require "xdo_test_helper"

class XdotoolCommandWindowSizeTests < Test::Unit::TestCase
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
    xdotool_ok "windowfocus --sync #{@wid}"
    xdotool_ok "getwindowfocus windowsize 20 20"
    xdotool_ok "getwindowfocus windowsize %1 20 20"
    xdotool_ok "getwindowfocus windowsize %@ 20 20"
  end # def test_chaining
end # class XdotoolCommandWindowsizeTests

