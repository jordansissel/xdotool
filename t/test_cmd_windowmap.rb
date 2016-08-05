
#!/usr/bin/env ruby
#

require "minitest"
require "./xdo_test_helper"

class XdotoolCommandWindowMapTests < MiniTest::Test
  include XdoTestHelper

  def test_succeeds_with_valid_window
    xdotool_ok "windowmap #{@wid}"
  end # def test_succeeds_with_valid_window

  def test_expected_failures
    xdotool_fail "windowmap"
    xdotool_fail "windowmap %1"
    xdotool_fail "windowmap %@"
    xdotool_fail "windowmap 2" # test invalid window
  end # def test_fails_without_a_window

  def test_chaining
    xdotool_ok "windowfocus --sync #{@wid}"
    xdotool_ok "getwindowfocus -f windowmap "
    xdotool_ok "getwindowfocus -f windowmap %1"
    xdotool_ok "getwindowfocus -f windowmap %@"
  end # def test_chaining
end # class XdotoolCommandWindowMapTests
