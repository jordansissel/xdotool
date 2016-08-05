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
end # class XdotoolCommandWindowFocusTests

