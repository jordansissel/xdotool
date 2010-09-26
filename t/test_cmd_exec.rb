
#!/usr/bin/env ruby
#

require "test/unit"
require "xdo_test_helper"

class XdotoolCommandExecTests < Test::Unit::TestCase
  include XdoTestHelper

  def test_expected_failures
    xdotool_fail "exec" # no arguments == failure
    xdotool_fail "exec --sync"
    xdotool_fail "exec --sync someinvalidcommandthatshouldnotexist"
    xdotool_fail "exec --sync /bin/false"
    xdotool_fail "exec --sync sh -c 'exit 1'"
  end # def test_expected_failures

  def test_expected_successes
    xdotool_ok "exec true"
    xdotool_ok "exec false" # no --sync
    xdotool_ok "exec --sync true"
    xdotool_ok "exec --sync sh -c 'exit 0'"
  end

  def test_chaining
    xdotool_fail "exec --sync false search --name ."
  end # def test_chaining
end # class XdotoolCommandWindowFocusTests

