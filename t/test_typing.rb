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
    @title = "#{self.class.name}_#{rand}"

    setup_ensure_x_is_healthy
    @file = Tempfile.new("xdotool-test")

    setup_launch("xterm -u8 -T '#{@title}' -e 'cat >> #{@file.path}'")

    found = false
    while !found
      #status, lines = xdotool "search --onlyvisible --pid #{@launchpid}"
      status, lines = xdotool "search --onlyvisible --name '#{@title}'"
      found = (status == 0)
      assert(lines.length < 2, "Should only be at most 1 window matching #{@launchpid}")
      @wid = lines.first.to_i
      sleep 0.2
    end

    ready = false
    while !ready
      xdotool "windowfocus #{@wid}"
      status, lines = xdotool "getwindowfocus"
      if status == 0 and lines.first.to_i == @wid
        ready = true
      end
      sleep 0.2
    end
    #puts "Window #{@wid} has focus"
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
    #status, lines = xdotool "type --window #{@wid} --clearmodifiers '#{input}'"
    #_xdotool "key ctrl+s ctrl+q"
    status, lines = xdotool "type --clearmodifiers '#{input}'"
    xdotool "key ctrl+d ctrl+d"
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
    # SE keymap has no '^' or '$'
    _test_typing("!#%&*()")
  end

end # class XdotoolTypingTests

