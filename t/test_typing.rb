#!/usr/bin/env ruby
#

require "test/unit"
require "tempfile"
require "xdo_test_helper"

class XdotoolTypingTests < Test::Unit::TestCase
  include XdoTestHelper

  # override XdoTestHelper#setup
  def setup
    @xdotool = "../xdotool"
    setup_ensure_x_is_healthy
    @file = Tempfile.new("xdotool-test")

    # Launch an editor so we can test typing.
    setup_launch("gedit", @file.path)

    found = false
    while !found
      status, lines = _xdotool "search --onlyvisible --class gedit"
      found = (status == 0)
      @wid = lines.first.to_i
      sleep 0.2
    end

    ready = false
    while !ready
      _xdotool "windowfocus #{@wid}"
      status, lines = _xdotool "getwindowfocus"
      if status == 0 and lines.first.to_i == @wid
        ready = true
      end
      sleep 0.2
    end

  end # def setup

  def readfile
    # gedit and other editors don't truncate the file, they simply
    # create a new file and overwrite it. This is observable by
    # noting the inode number changing on the file. At any rate, this
    # requires us to open the file by name instead of using the existing
    # @file.
    file = File.open(@file.path, "r")
    return file.read.chomp
  end

  def type(input)
    status, lines = _xdotool "type --clearmodifiers '#{input}'"
    _xdotool "key ctrl+s ctrl+q"
    Process.wait(@launchpid) rescue nil
    return readfile
  end

  def _test_typing(input)
    data = type(input)
    assert_equal(input, data)
  end

  def test_us_simple_typing
    system("setxkbmap us")
    _test_typing("Hello world")
  end

  def test_us_symbol_typing
    system("setxkbmap us")
    _test_typing("!@\#$%^&*()")
  end

  def test_us_dvorak_simple_typing
    system("setxkbmap us dvorak")
    _test_typing("Hello world")
  end

  def test_us_dvorak_symbol_typing
    system("setxkbmap us dvorak")
    _test_typing("!@\#$%^&*()")
  end

  def test_se_simple_typing
    system("setxkbmap se")
    _test_typing("Hello world")
  end

  def test_se_symbol_typing
    system("setxkbmap se")
    # SE keymap has no '^' (circum)
    _test_typing("!@\#$%&*()")
  end

end # class XdotoolTypingTests

