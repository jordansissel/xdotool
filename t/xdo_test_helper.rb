module XdoTestHelper
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
    return runcmd("#{@xdotool} #{args}")
  end

  def _xdotool_ok(args)
    status, lines = _xdotool(args)
    assert_equal(0, status, "Exit code expected to be 0 for #{args}")
    return [status, lines]
  end

  def runcmd(command)
    io = IO.popen(command)
    output = io.readlines.collect { |i| i.chomp }
    io.close
    return [$?.exitstatus, output]
  end

  def assert_status_ok(status)
    assert_equal(0, status, "Exit status should have been 0, was #{status}")
  end

end
