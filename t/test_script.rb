#!/usr/bin/env ruby
#

require "minitest"
require "./xdo_test_helper"
require "tempfile"

class XdotoolScriptTests < MiniTest::Test
  include XdoTestHelper

  def xdotool_script_ok(commands)
    scriptfile = Tempfile.new("xdotool-script-test")
    scriptfile.puts(commands.join("\n"))
    scriptfile.flush
    return xdotool_ok scriptfile.path
  end # def xdotool_script

  def xdotool_script_fail(commands)
    scriptfile = Tempfile.new("xdotool-script-test")
    scriptfile.puts(commands.join("\n"))
    scriptfile.flush
    return xdotool_fail scriptfile.path
  end # def xdotool_script

  def test_simple
    xdotool_script_fail [ "Hello world", "one two three" ]
    xdotool_script_ok [ "# comment", "mousemove 0 0" ]
    xdotool_script_ok [ "mousemove 0 0", "mousemove 0 0" ]
    xdotool_script_ok [ "mousemove 0 0", "", "mousemove 0 0" ]

  end # def test_expected_failures

  def test_long
    # Create a really long chained command that is longer than a single 4096byte read
    # xdotool's script_main reads the input 4096 bytes at a time.
    xdotool_script_ok [ "mousemove 0 0 " * 1000 ]
  end
end # class XdotoolCommandWindowFocusTests

