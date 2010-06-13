
#!/usr/bin/env ruby
#

require "test/unit"
require "xdo_test_helper"

class XdotoolCommandWindowFocusTests < Test::Unit::TestCase
  include XdoTestHelper

  def test_succeeds_with_valid_window
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
    xdotool_ok "getwindowfocus windowfocus "
    xdotool_ok "getwindowfocus windowfocus %1"
    xdotool_ok "getwindowfocus windowfocus %@"
  end # def test_chaining
end # class XdotoolCommandWindowFocusTests

