require "test/unit"

module XdoTestHelper
  def setup
    @xdotool = "../xdotool"
    @title = "#{self.class.name}_#{rand}"
    setup_ensure_x_is_healthy
    setup_launch_xterm
  end # def setup

  def setup_launch(*cmd)
    @launchpid = fork do
      exec(*cmd)
    end
  end

  def setup_launch_xterm
    # Clever pipe trick to get xterm to tell us its window id
    reader, writer = IO.pipe
    @windowpid = fork do
      reader.close
      exec("exec xterm -T '#{@title}' -e 'echo $WINDOWID >& #{writer.fileno}; echo $$ >& #{writer.fileno}; exec sleep 300'")
    end # xterm fork
    writer.close
    @wid = reader.readline.to_i
    @shellpid = reader.readline.to_i

    healthy = false
    while !healthy
      status, lines = runcmd("xwininfo -id #{@wid}")
      healthy = (status == 0)
      sleep 0.2
    end

  end # def setup_launch_xterm
  
  def setup_ensure_x_is_healthy
    healthy = false
    (1 .. 10).each do
      system("xdpyinfo > /dev/null 2>&1")
      healthy = ($?.exitstatus == 0)
      break if healthy
      puts "Waiting for xserver on #{ENV["DISPLAY"]} to be healthy"
      sleep 0.3
    end

    # Give up if we can't talk to the X server.
    if !healthy
      fail("X server on #{ENV["DISPLAY"]} was not responding. Aborting")
    end
  end

  def teardown
    if @shellpid
      Process.kill("TERM", @shellpid)
      Process.wait(@shellpid) rescue nil
    end

    if @windowpid
      Process.wait(@windowpid) rescue nil
    end

    if @launchpid
      Process.kill("TERM", @launchpid) rescue nil
      Process.wait(@launchpid) rescue nil
    end
  end # def teardown

  def _xdotool(args)
    #puts "Running: #{@xdotool} #{args}"
    return runcmd("#{@xdotool} #{args}")
  end # def _xdotool

  def _xdotool_ok(args)
    status, lines = _xdotool(args)
    assert_equal(0, status, "Exit code expected to be 0 for #{args}")
    return [status, lines]
  end # def _xdotool_ok

  def runcmd(command)
    io = IO.popen(command)
    output = io.readlines.collect { |i| i.chomp }
    io.close
    return [$?.exitstatus, output]
  end # def runcmd

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
    delay = options[:delay] || 0.3

    last_exception = nil
    (1 .. times).each do
      begin
        yield
      rescue Test::Unit::AssertionFailedError => e
        $stderr.puts "Retrying..."
        last_exception = e
        sleep(delay)
        next
      end # begin

      # If we get here, there was no assertions. Test pass.
      return
    end # loop 1 .. times

    raise last_exception
  end

end
