#!/usr/bin/env ruby
#

require "minitest"
require "./xdo_test_helper"

class XdotoolCommandGetWindowPidTests < MiniTest::Test
  include XdoTestHelper

  def assert_is_title(lines)
    assert_equal(1, lines.length,
                 "Expected only one line of output, got #{lines.length}")
    assert_equal(@title, lines[0],
                "Expected title '#{@title}' but got #{lines[0]}")
  end

  def test_succeeds_with_valid_window
    status, lines = xdotool_ok "getwindowname #{@wid}"
    assert_is_title(lines);
  end # def test_succeeds_with_valid_window

  def test_fails_without_a_window
    xdotool_fail "getwindowname"
    xdotool_fail "getwindowname %1"
    xdotool_fail "getwindowname %@"
  end # def test_fails_without_a_window

  def test_chaining
    xdotool_ok "windowfocus --sync #{@wid}"
    status, lines = xdotool_ok "getwindowfocus -f getwindowname"
    assert_is_title(lines);
    status, lines = xdotool_ok "getwindowfocus -f getwindowname %1"
    assert_is_title(lines);
    status, lines = xdotool_ok "getwindowfocus -f getwindowname %@"
    assert_is_title(lines);
  end # def test_chaining
end # class XdotoolCommandGetWindowPidTests
