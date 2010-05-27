#!/usr/bin/env ruby

require "test/unit"
require "xdo_test_helper"

class XdotoolMouseMoveTests < Test::Unit::TestCase
  include XdoTestHelper

  def test_mousemove_should_have_no_output
    status, lines = xdotool_ok "mousemove 0 0"
    assert_equal(0, lines.length, "mousemove should have no output")
  end # def test_mousemove_should_have_no_output

  def test_mousemove_flags
    xdotool_ok "mousemove -c 0 0"
    xdotool_ok "mousemove --clearmodifiers 0 0"
    xdotool_ok "mousemove -p 0 0"
    xdotool_ok "mousemove --polar 0 0"
    xdotool_ok "mousemove --screen 0 0 0"
    xdotool_ok "mousemove --sync 0 0"
    xdotool_ok "mousemove -w #{@wid} 0 0"
    xdotool_ok "mousemove --window #{@wid} 0 0"
  end # def test_mousemove_flags

  def test_mousemove
    x_list = [0, 1, 100, 200, 400]
    y_list = [0, 1, 100, 200, 400]

    x_list.each do |x|
      y_list.each do |y|
        status, lines = xdotool_ok "mousemove #{x} #{y}"
        try do
          assert_mouse_position(x, y)
        end
      end # x_list.each
    end # y_list.each 
  end # def test_mousemove

  def test_mousemove_with_sync
    x_list = [0, 200, 400]
    y_list = [0, 200, 400]

    x_list.each do |x|
      y_list.each do |y|
        status, lines = xdotool_ok "mousemove --sync #{x} #{y}"
        assert_mouse_position(x, y)
      end # x_list.each
    end # y_list.each 
  end # def test_mousemove
end # XdotoolMouseMoveTests
