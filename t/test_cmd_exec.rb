
#!/usr/bin/env ruby
#

require "minitest"
require "./xdo_test_helper"

class XdotoolCommandExecTests < MiniTest::Test
  include XdoTestHelper

  def test_expected_failures
    xdotool_fail "exec" # no arguments == failure
    xdotool_fail "exec --sync"
    xdotool_fail "exec --sync someinvalidcommandthatshouldnotexist"
    xdotool_fail "exec --sync /bin/false"
    xdotool_fail "exec --sync sh -c 'exit 1'"
    xdotool_fail "exec --sync --args 3 echo ok"
    xdotool_fail "exec --sync --args 3 ok"
  end # def test_expected_failures

  def test_expected_successes
    xdotool_ok "exec true"
    xdotool_ok "exec false" # no --sync
    xdotool_ok "exec --sync true"
    xdotool_ok "exec --sync sh -c 'exit 0'"
    xdotool_ok "exec --sync --args 3 sh -c 'exit 0'"
    xdotool_ok "exec --sync --terminator END sh -c 'exit 0' END"
  end

  def test_chaining
    xdotool_fail "exec --sync false search --name ."
  end # def test_chaining
end # class XdotoolCommandWindowFocusTests

