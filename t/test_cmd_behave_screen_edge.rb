#!/usr/bin/env ruby
#

require "minitest"
require "./xdo_test_helper"

class XdotoolCommandBehaveScreenEdgeTests < MiniTest::Test
  include XdoTestHelper

  def test_expected_failures
    xdotool_fail "behave_screen_edge" # no arguments == failure
    xdotool_fail "behave_screen_edge top"
  end # def test_expected_failures
end # class XdotoolCommandBehaveScreenEdgeTests

