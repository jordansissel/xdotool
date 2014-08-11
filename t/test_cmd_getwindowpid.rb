#!/usr/bin/env ruby
#

require "test/unit"
require_relative "xdo_test_helper"

class XdotoolCommandGetWindowPidTests < Test::Unit::TestCase
  include XdoTestHelper

  def test_succeeds_with_valid_window
    xdotool_ok "getwindowpid #{@wid}"
  end # def test_succeeds_with_valid_window

  def test_fails_without_a_window
    xdotool_fail "getwindowpid"
    xdotool_fail "getwindowpid %1"
    xdotool_fail "getwindowpid %@"
  end # def test_fails_without_a_window

  def test_chaining
    xdotool_ok "windowfocus --sync #{@wid}"
    xdotool_ok "getwindowfocus -f getwindowpid"
    xdotool_ok "getwindowfocus -f getwindowpid %1"
    xdotool_ok "getwindowfocus -f getwindowpid %@"
  end # def test_chaining
end # class XdotoolCommandGetWindowPidTests
