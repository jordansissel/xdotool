#!/usr/bin/env ruby
#

require "test/unit"
require "xdo_test_helper"

class XdotoolWindowTests < Test::Unit::TestCase
  include XdoTestHelper

  def test_set_name
    name = rand.to_s
    status, lines = xdotool "set_window --name '#{name}' #{@wid}"
    assert_status_ok(status);
    assert_equal(0, lines.length, "set_window should have no output")
    xprop_status, xprop_output = runcmd("xprop -id #{@wid}")
    assert_send([xprop_output, :include?, "WM_NAME(STRING) = \"#{name}\""],
                "xprop should report the WM_NAME as the value we set")
  end

  def test_set_class
    classname = rand.to_s
    _class = rand.to_s

    status, lines = xdotool "set_window --class '#{_class}' #{@wid}"
    assert_status_ok(status);
    assert_equal(0, lines.length, "set_window should have no output")

    status, lines = xdotool "set_window --classname '#{classname}' #{@wid}"
    assert_status_ok(status);
    assert_equal(0, lines.length, "set_window should have no output")

    xprop_status, xprop_output = runcmd("xprop -id #{@wid}")
    assert_send([xprop_output, :include?, 
                "WM_CLASS(STRING) = \"#{classname}\", \"#{_class}\""],
                "xprop should report the WM_CLASS as the value we set")
  end

  def test_set_icon
    icon = rand.to_s
    status, lines = xdotool "set_window --icon '#{icon}' #{@wid}"
    assert_status_ok(status);
    assert_equal(0, lines.length, "set_window should have no output")
    xprop_status, xprop_output = runcmd("xprop -id #{@wid}")
    assert_send([xprop_output, :include?, "WM_ICON_NAME(STRING) = \"#{icon}\""],
                "xprop should report the WM_ICON_NAME as the value we set")
  end

  def test_set_role
    role = rand.to_s
    status, lines = xdotool "set_window --role '#{role}' #{@wid}"
    assert_status_ok(status);
    assert_equal(0, lines.length, "set_window should have no output")
    xprop_status, xprop_output = runcmd("xprop -id #{@wid}")
    assert_send([xprop_output, :include?, "WM_WINDOW_ROLE(STRING) = \"#{role}\""],
                "xprop should report the WM_ROLE as the value we set")
  end
end # class XdotoolWindowTests
