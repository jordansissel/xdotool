#!/usr/bin/env ruby
#

require 'test/unit'

class XdotoolSearchTests < Test::Unit::TestCase
  def setup
    @xdotool = "../xdotool"
    @title = "#{self.class.name}_#{rand}"

    # Clever pipe trick to get xterm to tell us its window id
    reader, writer = IO.pipe
    @windowpid = fork { 
      exec("exec xterm -T '#{@title}' -e 'echo $WINDOWID >& #{writer.fileno}; echo $$ >& #{writer.fileno}; exec sleep 300'") 
    };
    @wid = reader.readline.to_i
    @shellpid = reader.readline.to_i
  end

  def teardown
    Process.kill("TERM", @shellpid)
  end

  def _xdotool(args)
    io = IO.popen("#{@xdotool} #{args}")
    output = io.readlines.collect { |i| i.chomp }
    io.close
    return [$?.exitstatus, output]
  end

  def test_search_pid
    status, lines = _xdotool "search --pid #{@windowpid}"
    assert_equal(0, status, "Exit status should have been 0")
    assert_equal(1, lines.size, "Expect only one match to our search (only one window running that should match)")
    assert_equal(@wid, lines[0].to_i, "Expected the correct windowid when searching for its pid")
  end

  def test_search_title
    status, lines = _xdotool "search --title #{@title}"
    assert_equal(0, status, "Exit status should have been 0")
    assert_equal(1, lines.size, "Expect only one match to our search (only one window running that should match)")
    assert_equal(@wid, lines[0].to_i, "Expected the correct windowid when searching for its pid")
  end

  def test_search_onlyvisible_with_pid
    status, lines = _xdotool "search --onlyvisible --pid #{@windowpid}"
    assert_equal(0, status, "Exit status should have been 0")
    assert_equal(1, lines.size, "Expect only one match to our search (only one window running that should match)")
    assert_equal(@wid, lines[0].to_i, "Expected the correct windowid when searching for its pid")

    # Hide the window and try searching for it. We shouldn't find it.
    status, lines = _xdotool "windowunmap #{@wid}"
    assert_equal(0, status, "Exit status should have been 0")
    status, lines = _xdotool "search --onlyvisible --pid #{@windowpid}"
    # search will exit 1 when no matches are found
    assert_equal(1, status, "Exit status should have been 1")
    assert_equal(0, lines.size, "Expect no matches with onlyvisible and the only match window is hidden")

    # Bring up the window again and try searching for it.
    status, lines = _xdotool "windowmap #{@wid}"
    assert_equal(0, status, "Exit status should have been 0")
    status, lines = _xdotool "search --onlyvisible --pid #{@windowpid}"
    assert_equal(0, status, "Exit status should have been 0")
    assert_equal(1, lines.size, "Expect only one match to our search (only one window running that should match)")
    assert_equal(@wid, lines[0].to_i, "Expected the correct windowid when searching for its pid")
  end
end # XdotoolSearchTests
