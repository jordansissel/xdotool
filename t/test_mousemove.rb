#!/usr/bin/env ruby

require "minitest"
require "./xdo_test_helper"

class XdotoolMouseMoveTests < MiniTest::Test
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

  def test_mousemove_polar
    dimensions = %x{xdpyinfo}.split("\n").grep(/dimensions:/).first.split[1]
    w, h = dimensions.split("x").collect { |v| v.to_i }

    center_x = (w/2).to_i
    center_y = (h/2).to_i

    xdotool_ok "mousemove --sync --polar 0 100"
    assert_mouse_position_near(center_x, center_y - 100);

    xdotool_ok "mousemove --sync --polar 90 100"
    assert_mouse_position_near(center_x + 100, center_y);

    xdotool_ok "mousemove --sync --polar 180 100"
    assert_mouse_position_near(center_x, center_y + 100);

    xdotool_ok "mousemove --sync --polar 270 100"
    assert_mouse_position_near(center_x - 100, center_y);
  end

  def test_mousemove_relative
    start_x = 300
    start_y = 200

    xdotool_ok "mousemove --sync #{start_x} #{start_y}"

    x_list = [-100, -50, -10, -5, -1, 0,1,20,50,100]
    y_list = [-100, -50, -10, -5, -1, 0,1,20,50,100]

    x_list.each do |x|
      y_list.each do |y|
        xdotool_ok "mousemove --sync #{start_x} #{start_y}"
        status, lines = xdotool_ok "mousemove_relative --sync -- #{x} #{y}"
        assert_mouse_position_near(start_x + x, start_y + y)
      end # x_list.each
    end # y_list.each 
  end

  def test_mousemove_relative_polar
    start_x = 300
    start_y = 200;

    xdotool_ok "mousemove --sync #{start_x} #{start_y}"
    xdotool_ok "mousemove_relative --sync --polar 0 100"
    assert_mouse_position_near(start_x, start_y - 100);

    xdotool_ok "mousemove --sync #{start_x} #{start_y}"
    xdotool_ok "mousemove_relative --sync --polar 90 100"
    assert_mouse_position_near(start_x + 100, start_y);

    xdotool_ok "mousemove --sync #{start_x} #{start_y}"
    xdotool_ok "mousemove_relative --sync --polar 180 100"
    assert_mouse_position_near(start_x, start_y + 100);

    xdotool_ok "mousemove --sync #{start_x} #{start_y}"
    xdotool_ok "mousemove_relative --sync --polar 270 100"
    assert_mouse_position_near(start_x - 100, start_y);
  end

  # https://github.com/jordansissel/xdotool/issues/64
  def test_mousemove_same_position_of_the_other_window
    move_x = 100
    move_y = 200

    # It seems window wid located at (0, 0) at first.
    xdotool_ok "windowmove --sync #{@wid} 400 400"

    status, xwininfo_output = runcmd "xwininfo -id #{@wid}"
    the_other_window_x = xwininfo_output.grep(/ Absolute upper-left X:/).first[/[0-9]+/].to_i
    the_other_window_y = xwininfo_output.grep(/ Absolute upper-left Y:/).first[/[0-9]+/].to_i

    xdotool_ok "mousemove --sync #{move_x} #{move_y}"
    xdotool_ok "mousemove --sync --window #{@wid} #{move_x} #{move_y}"

    assert_mouse_position_near(the_other_window_x + move_x,
                               the_other_window_y + move_y)
  end
end # XdotoolMouseMoveTests
