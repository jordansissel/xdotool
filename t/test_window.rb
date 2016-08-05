#!/usr/bin/env ruby

require "minitest"
require "./xdo_test_helper"

class XdotoolWindowTests < MiniTest::Test
  include XdoTestHelper

  def setup
    setup_vars
    setup_ensure_x_is_healthy
    setup_launch_xterm
    @requirewm = false
  end # def setup

  def test_set_name
    return if @requirewm && detect_window_manager == :none
    name = rand.to_s
    status, lines = xdotool "set_window --name '#{name}' #{@wid}"
    assert_status_ok(status);
    assert_equal(0, lines.length, "set_window should have no output")
    xprop_status, xprop_output = runcmd("xprop -id #{@wid}")
    assert_send([xprop_output, :include?, "WM_NAME(STRING) = \"#{name}\""],
                "xprop should report the WM_NAME as the value we set")
  end

  def test_set_class
    return if @requirewm && detect_window_manager == :none
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
    return if @requirewm && detect_window_manager == :none
    icon = rand.to_s
    status, lines = xdotool "set_window --icon '#{icon}' #{@wid}"
    assert_status_ok(status);
    assert_equal(0, lines.length, "set_window should have no output")
    xprop_status, xprop_output = runcmd("xprop -id #{@wid}")
    assert_send([xprop_output, :include?, "WM_ICON_NAME(STRING) = \"#{icon}\""],
                "xprop should report the WM_ICON_NAME as the value we set")
  end

  def test_set_role
    return if @requirewm && detect_window_manager == :none
    role = rand.to_s
    status, lines = xdotool "set_window --role '#{role}' #{@wid}"
    assert_status_ok(status);
    assert_equal(0, lines.length, "set_window should have no output")
    xprop_status, xprop_output = runcmd("xprop -id #{@wid}")
    assert_send([xprop_output, :include?, "WM_WINDOW_ROLE(STRING) = \"#{role}\""],
                "xprop should report the WM_ROLE as the value we set")
  end
end # class XdotoolWindowTests

class XdotoolWindowChainTests < XdotoolWindowTests
  def setup
    setup_vars
    setup_ensure_x_is_healthy
    setup_launch_xterm
    @requirewm = true
  end
end

class XdotoolWindowChainDefaultTests < XdotoolWindowChainTests
  def xdotool(args)
    args.gsub!(@wid.to_s, "");
    args = "getwindowfocus -f #{args}"
    if $DEBUG
      puts "Running: #{@xdotool} #{args}"
    end
    return runcmd("#{@xdotool} #{args}")
  end # def xdotool
end # class XdotoolWindowChainTests

class XdotoolWindowChain1Tests < XdotoolWindowChainTests
  def xdotool(args)
    args.gsub!(@wid.to_s, "%1");
    args = "getwindowfocus -f #{args}"
    if $DEBUG
      puts "Running: #{@xdotool} #{args}"
    end
    return runcmd("#{@xdotool} #{args}")
  end # def xdotool
end # class XdotoolWindowChain1Tests

class XdotoolWindowChainAllTests < XdotoolWindowChainTests
  def xdotool(args)
    args.gsub!(@wid.to_s, "%@");
    args = "getwindowfocus -f #{args}"
    if $DEBUG
      puts "Running: #{@xdotool} #{args}"
    end
    return runcmd("#{@xdotool} #{args}")
  end # def xdotool
end # class XdotoolWindowChainAllTests
