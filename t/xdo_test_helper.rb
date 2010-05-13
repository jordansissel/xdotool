require "test/unit"

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
    #puts "Running: #{@xdotool} #{args}"
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

  def assert_status_ok(status, msg="")
    assert_equal(0, status, "Exit status should have been 0, was #{status}. #{msg}")
  end

  def assert_status_fail(status, msg="")
    assert_not_equal(0, status, "Exit status should not have been 0, was #{status}. #{msg}")
  end

  def detect_window_manager
    status, lines = runcmd("xprop -root")
    
    # ion
    if lines.grep(/^_ION_WORKSPACE/).length > 0
      return :ion
    end

    return :unknown
  end

  def wm_supports?(feature)
    status, lines = runcmd("xprop -root")

    supported = lines.grep(/^_NET_SUPPORTED/)
    return false if supported.length == 0

    features = supported.first.split(" = ")[-1].split(", ")
    return features.include?(feature)
  end

  def try(options = {})
    times = options[:times] || 5
    delay = options[:delay] || 0.1

    last_exception = nil
    (1 .. times).each do
      begin
        yield
      rescue Test::Unit::AssertionFailedError => e
        $stderr.puts "Retrying..."
        last_exception = e
        next
      end # begin

      # If we get here, there was no assertions. Test pass.
      return
    end # loop 1 .. times

    raise last_exception
  end

end
