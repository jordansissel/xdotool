#!/usr/bin/env ruby
#

require "minitest"
require "./xdo_test_helper"

class XdotoolCommandKeyTests < MiniTest::Test
  include XdoTestHelper

  def test_flags
    %w{key keyup keydown}.each do |command|
      xdotool_fail "#{command}"
      xdotool_ok "#{command} Return"

      xdotool_ok "#{command} --clearmodifiers Return"
      xdotool_fail "#{command} --clearmodifiers"

      xdotool_ok "#{command} --delay 30 Return"
      xdotool_fail "#{command} --delay Return"
      xdotool_fail "#{command} --delay 100"
      xdotool_fail "#{command} --delay"

      xdotool_ok "#{command} --help"

      xdotool_ok "#{command} --window #{@wid} Return"
      xdotool_fail "#{command} --window"
      xdotool_fail "#{command} --window Return"

      # window -1 should be invalid, right?
      xdotool_fail "#{command} --window -1 Return"

      xdotool_ok "#{command} --delay 10 --clearmodifiers Return Return shift+Return"
    end # %w{ ... }.each
  end # def test_flags

  def test_another_command_ends_command
    %w{key keyup keydown}.each do |command|
      xdotool_ok "#{command} a b c getmouselocation"
    end
  end

  def test_chaining
    xdotool_ok "windowfocus --sync #{@wid}"
    xdotool_ok "getwindowfocus -f key a b c d e"
    xdotool_ok "getwindowfocus -f key --window %1 a b c d e"
    xdotool_ok "getwindowfocus -f key --window %@ a b c d e"
  end
end # class XdotoolCommandKeyTests 

