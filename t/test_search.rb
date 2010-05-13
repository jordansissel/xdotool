#!/usr/bin/env ruby
#

require "test/unit"
require "xdo_test_helper"

class XdotoolSearchTests < Test::Unit::TestCase
  include XdoTestHelper

  def test_search_pid
    try do
      status, lines = _xdotool "search --pid #{@windowpid}"
      assert_equal(0, status, "Exit status should have been 0")
      assert_equal(1, lines.size, "Expect only one match to our search (only one window running that should match)")
      assert_equal(@wid, lines[0].to_i, "Expected the correct windowid when searching for its pid")
    end
  end

  def test_search_title
    status, lines = _xdotool "search --title #{@title}"
    try do
      assert_equal(0, status, "Exit status should have been 0")
      assert_equal(1, lines.size,
                   "Expect only one match to our search (only one window " \
                   "running that should match)")
      assert_equal(@wid, lines[0].to_i,
                   "Expected the correct windowid when searching for its pid")
    end
  end

  def test_search_onlyvisible_with_pid
    try do
      status, lines = _xdotool "search --onlyvisible --pid #{@windowpid}"
      assert_equal(0, status, "Exit status should have been 0")
      assert_equal(1, lines.size, "Expect only one match to our search (only one window running that should match)")
      assert_equal(@wid, lines[0].to_i, "Expected the correct windowid when searching for its pid")
    end

    try do
      # Hide the window and try searching for it. We shouldn't find it.
      status, lines = _xdotool "windowunmap #{@wid}"
      assert_equal(0, status, "Exit status should have been 0")
    end

    try do
      status, lines = _xdotool "search --onlyvisible --pid #{@windowpid}"
      # search will exit 1 when no matches are found
      assert_equal(1, status, "Exit status should have been 1")
      assert_equal(0, lines.size, "Expect no matches with onlyvisible and the only match window is hidden")
    end

    try do
      # Bring up the window again and try searching for it.
      status, lines = _xdotool "windowmap #{@wid}"
      assert_equal(0, status, "Exit status should have been 0")

      status, lines = _xdotool "search --onlyvisible --pid #{@windowpid}"
      assert_equal(0, status, "Exit status should have been 0")
      assert_equal(1, lines.size,
                   "Expect only one match to our search (only one window" \
                   "running that should match)")
      assert_equal(@wid, lines[0].to_i,
                   "Expected the correct windowid when searching for its pid")
    end
  end

  def test_search_maxdeth_0_has_no_results
    status, lines = _xdotool "search --maxdepth 0 ."
    assert_equal(1, status, "Exit status should be nonzero (no results expected)")
    assert_equal(0, lines.length, "Search with --maxdepth 0 should return no results");
  end

  def test_search_by_name
    name = "name#{rand}"
    status, lines = _xdotool "search --name '#{name}'"
    assert_equal(1, status, 
                 "Search for window with name '#{name}' exit nonzero when" \
                 + " no window with that name exists")
    assert_equal(0, lines.length, 
                 "Search for window with name '#{name}' should have no results when" \
                 + " no window with that name exists")
                  

    _xdotool "set_window --name '#{name}' #{@wid}"
    try do
      status, lines = _xdotool "search --name '#{name}'"
      assert_equal(0, status, "Search for name '#{name}' should exit zero")
      assert_equal(1, lines.size, "Search for name '#{name}' have one result")
      assert(lines.include?(@wid.to_s),
             "Searched results should include our expected window")
    end
  end

  def test_search_by_class
    name = "class#{rand}"
    status, lines = _xdotool "search --class '#{name}'"
    assert_equal(1, status, 
                 "Search for window with class '#{name}' exit nonzero when" \
                 + " no window with that class exists")
    assert_equal(0, lines.length, 
                 "Search for window with class '#{name}' should have no results when" \
                 + " no window with that class exists")
                  

    _xdotool "set_window --class '#{name}' #{@wid}"

    try do
      status, lines = _xdotool "search --class '#{name}'"
      assert_equal(0, status, "Search for class '#{name}' should exit zero")
      assert_equal(1, lines.size, "Search for class '#{name}' have one result")
      assert(lines.include?(@wid.to_s),
             "Searched results should include our expected window")
    end
  end

  def test_search_by_classname
    name = "classname#{rand}"
    status, lines = _xdotool "search --classname '#{name}'"
    assert_equal(1, status, 
                 "Search for window with classname '#{name}' exit nonzero when" \
                 + " no window with that classname exists")
    assert_equal(0, lines.length, 
                 "Search for window with classname '#{name}' should have no results when" \
                 + " no window with that classname exists")
                  

    _xdotool "set_window --classname '#{name}' #{@wid}"

    try do
      status, lines = _xdotool "search --classname '#{name}'"
      assert_equal(0, status, "Search for classname '#{name}' should exit zero")
      assert_equal(1, lines.size, "Search for classname '#{name}' have one result")
      assert(lines.include?(@wid.to_s),
             "Searched results should include our expected window")
    end
  end
end # XdotoolSearchTests
