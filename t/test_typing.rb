#!/usr/bin/env ruby
#

require "minitest"
require "tempfile"
require "./xdo_test_helper"

class XdotoolTypingTests < MiniTest::Test
  include XdoTestHelper
  SYMBOLS = "`12345678990-=~ !@\#$%^&*()_+[]\{}|;':\",./<>?"
  LETTERS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"

  # override XdoTestHelper#setup
  def setup
    @xdotool = "../xdotool"
    @title = "#{self.class.name}_#{rand}"

    setup_ensure_x_is_healthy
    @file = Tempfile.new("xdotool-test")

    setup_launch_xterm("cat >> #{@file.path}")

    ready = false
    while !ready
      #puts "Waiting for focus"
      xdotool "windowfocus #{@wid}"
      status, lines = xdotool "getwindowfocus -f"
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
    input.gsub!(/'/, "\\'")
    status, lines = xdotool "type --clearmodifiers '#{input}'"
    xdotool "key ctrl+d ctrl+d"
    Process.wait(@launchpid) rescue nil
    return readfile
  end

  def _test_typing(input, knownbroken=false)
    data = type(input)
    if (knownbroken and ENV['SKIP_KNOWN_BROKEN_TESTS'])
      puts "Skipping known-broken test"
    else
      assert_equal(input, data)
    end
  end

  def test_us_simple_typing
    system("setxkbmap us")
    _test_typing(LETTERS)
  end

  def test_us_symbol_typing
    system("setxkbmap us")
    _test_typing(SYMBOLS)
  end

  def test_us_se_simple_typing
    system("setxkbmap -option grp:switch,grp:shifts_toggle us,se")
    _test_typing(LETTERS)
  end

  def test_us_se_symbol_typing
    system("setxkbmap -option grp:switch,grp:shifts_toggle us,se")
    _test_typing(SYMBOLS, true)
  end

  def test_se_us_simple_typing
    system("setxkbmap -option grp:switch,grp:shifts_toggle se,us")
    _test_typing(LETTERS)
  end

  def test_se_us_symbol_typing
    system("setxkbmap -option grp:switch,grp:shifts_toggle se,us")
    _test_typing(SYMBOLS, true)
  end

  def test_us_dvorak_simple_typing
    system("setxkbmap us dvorak")
    _test_typing(LETTERS)
  end

  def test_us_dvorak_symbol_typing
    system("setxkbmap us dvorak")
    _test_typing(SYMBOLS)
  end

  def test_se_simple_typing
    system("setxkbmap se")
    _test_typing(LETTERS)
  end

  def test_se_symbol_typing
    system("setxkbmap se")
    _test_typing(SYMBOLS, true)
  end

  def test_de_simple_typing
    system("setxkbmap de")
    _test_typing(LETTERS)
  end

  def test_de_symbol_typing
    system("setxkbmap de")
    _test_typing(SYMBOLS, true)
  end

  def test_terminator
    input = %w{hello world}
    status, lines = xdotool "type --clearmodifiers --terminator FIZZLE #{input.join(" ")} FIZZLE"
    xdotool "key ctrl+d ctrl+d"
    Process.wait(@launchpid) rescue nil
    data = readfile
    assert_equal(input.join(""), data)
  end # def test_terminator

  def test_arity
    input = %w{hello world}
    status, lines = xdotool "type --clearmodifiers --args 2 #{input.join(" ")}"
    xdotool "key ctrl+d ctrl+d"
    Process.wait(@launchpid) rescue nil
    data = readfile
    assert_equal(input.join(""), data)
  end # def test_terminator

end # class XdotoolTypingTests

