#!/usr/bin/env ruby
#

require "test/unit"
require "xdo_test_helper"

class XdotoolSearchTests < Test::Unit::TestCase
  include XdoTestHelper

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

  def test_search_maxdeth_0_has_no_results
    status, lines = _xdotool "search --maxdepth 0 ."
    assert_equal(1, status, "Exit status should be nonzero (no results expected)")
    assert_equal(0, lines.length, "Search with --maxdepth 0 should return no results");
  end
end # XdotoolSearchTests
